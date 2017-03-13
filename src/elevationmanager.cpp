//////////////////////////////////////////////////////////////////////////
//									//
//  Project:	NavTracks						//
//  Edit:	13-Mar-17						//
//									//
//////////////////////////////////////////////////////////////////////////
//									//
//  Copyright (c) 2012-2014 Jonathan Marten <jjm@keelhaul.me.uk>	//
//  Home and download page:  http://www.keelhaul.me.uk/TBD/		//
//									//
//  This program is free software; you can redistribute it and/or	//
//  modify it under the terms of the GNU General Public License as	//
//  published by the Free Software Foundation; either version 2 of	//
//  the License, or (at your option) any later version.			//
//									//
//  It is distributed in the hope that it will be useful, but		//
//  WITHOUT ANY WARRANTY; without even the implied warranty of		//
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the	//
//  GNU General Public License for more details.			//
//									//
//  You should have received a copy of the GNU General Public		//
//  License along with this program; see the file COPYING for further	//
//  details.  If not, write to the Free Software Foundation, Inc.,	//
//  59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.		//
//									//
//////////////////////////////////////////////////////////////////////////

#include "elevationmanager.h"

#include <qdebug.h>
#include <qtimer.h>
#include <qstandardpaths.h>
#include <qurl.h>
#include <qdir.h>

#include <kmessagebox.h>
#include <klocalizedstring.h>

#include <kio/filecopyjob.h>

#include "elevationtile.h"


static const int MAX_DOWNLOADS = 2;			// limit on running downloads
static const int DOWNLOAD_INTERVAL = 1000;		// interval between queue runs


ElevationManager::ElevationManager(QObject *pnt)
    : QObject(pnt)
{
    qDebug();

    mRunningJobs = 0;

    mQueueTimer = new QTimer(this);
    mQueueTimer->setInterval(DOWNLOAD_INTERVAL);
    mQueueTimer->setSingleShot(false);
    connect(mQueueTimer, &QTimer::timeout, this, &ElevationManager::slotNextFromQueue);

    //  This internal queued signal connection is so that the external
    //  tileReady() signal will be emitted on the next event loop.
    //  If the signal were emitted directly, then a code sequence
    //  such as:
    //
    //    connect(ElevationManager::self(), &ElevationManager::tileReady,
    //            TileUser, &TileUser::slotUseTile);
    //    requestedTile = ElevationManager::self()->requestTile(lat, lon);
    //
    //    Tile User::slotUseTile(const ElevationTile *tile)
    //    {
    //      if (tile==requestedTile) ...
    //
    //  would not work because, if the tile were already in memory, the
    //  signal would call slotUseTile() before the requestedTile was set.
    //
    //  Emitting the signal from requestTile(), in the case where the tile
    //  is already in memory, saves the DOWNLOAD_INTERVAL delay that would
    //  happen if the cache check were done and the signal emitted in
    //  slotNextFromQueue().
    //
    //  TODO: can use QMetaMethod::invoke() to emit signal queued?
    //
    connect(this, &ElevationManager::tileReadyInternal,
            this, &ElevationManager::slotTileReady,
            Qt::QueuedConnection);
}


ElevationManager::~ElevationManager()
{
    qDebug() << "requested" << mTiles.count() << "tiles";
    qDeleteAll(mTiles);
}


/* static */ ElevationManager *ElevationManager::self()
{
    static ElevationManager *instance = new ElevationManager;
    return (instance);
}


const ElevationTile *ElevationManager::requestTile(double lat, double lon)
{
    qDebug() << "for lat" << lat << "lon" << lon;

    ElevationTile *tile = NULL;

    ElevationTile::TileId id = ElevationTile::makeTileId(lat, lon);
    if (mTiles.contains(id))				// do we have tile alredy?
    {
        tile = mTiles.value(id);			// get from our tile map
        qDebug() << "  have tile" << tile->id();
        emit tileReadyInternal(tile);
    }
    else						// need a new tile
    {
        tile = new ElevationTile(id);			// create one for that location
        qDebug() << "  new tile" << tile->id();
        mTiles[id] = tile;				// save in our tile map
        startDownload(tile);				// start to fetch tile
    }

    return (tile);
}


void ElevationManager::startDownload(ElevationTile *tile)
{
    qDebug() << "for" << tile->id();

    switch (tile->state())
    {
case ElevationTile::Pending:				// already active or waiting
        qDebug() << "  already pending";
        return;

case ElevationTile::Loaded:				// loaded, should not happen
        qWarning() << "  already loaded";
        return;

case ElevationTile::Empty:				// nothing loaded yet
case ElevationTile::Error:				// error last time, can retry
        break;

default:						// other state, should not happen
        qWarning() << "  unknown tile state" << tile->state();
        return;
    }

    QString cache = cacheFile(tile);			// make cache filename
    qDebug() << "cache file" << cache;
    if (QFile::exists(cache))				// in cache already?
    {
    // TODO: this is slow, do in a thread
        const bool ok = tile->load(cache);		// load cache file into tile
        if (ok) emit tileReadyInternal(tile);
        return;
    }

    // set tile state = Pending
    tile->setState(ElevationTile::Pending);

    mDownloadQueue.enqueue(tile);			// add to download queue
    if (!mQueueTimer->isActive()) mQueueTimer->start();	// start to process queue
}


QString ElevationManager::cacheFile(const ElevationTile *tile)
{
    const QString cacheDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    QDir d(cacheDir);
    if (!d.exists())
    {
        QString cacheName = d.dirName();
        d.cdUp();
        if (d.mkdir(cacheName))
        {
            qDebug() << "Created cache directory" << d.absoluteFilePath(cacheName);
        }
        else
        {
            qWarning() << "Cannot create cache directory" << d.absoluteFilePath(cacheName);
        }
    }

    return (cacheDir+'/'+tile->cacheFile());		// make cache file name
}


void ElevationManager::slotDownloadResult(KJob *job)
{
    KIO::FileCopyJob *copyJob = qobject_cast<KIO::FileCopyJob *>(job);
    Q_ASSERT(copyJob!=NULL);
    --mRunningJobs;

    // get tile pointer from job
    ElevationTile *tile = static_cast<ElevationTile *>(copyJob->property("tile").value<void *>());
    Q_ASSERT(tile!=NULL);

    if (!job->error())					// check download success
    {
        QString cache = copyJob->destUrl().toLocalFile();
        const bool ok = tile->load(cache);		// load cache file into tile
        if (ok) emit tileReadyInternal(tile);		// tile now ready to use
    }
    else						// error downloading
    {
        // report error
        KMessageBox::error(NULL,
                           xi18nc("@info", "Error downloading tile <link>%1</link><nl/>to cache file <link>%2</link>%3",
                                  copyJob->srcUrl().toDisplayString(),
                                  copyJob->destUrl().toDisplayString(),
                                  copyJob->errorString()),
                           i18nc("@title:window", "Tile Download Failed"));
        // set tile status = Error
        tile->setState(ElevationTile::Error);
    }
}


void ElevationManager::slotNextFromQueue()
{
    if (mDownloadQueue.isEmpty())			// any more downloads waiting?
    {							// queue empty, nothing to do
        qDebug() << "queue empty, idle";
        mQueueTimer->stop();
        return;
    }

    if (mRunningJobs>=MAX_DOWNLOADS)			// enough running already?
    {							// try again next timer tick
        qDebug() << "too many running, wait";
        return;
    }

    ElevationTile *tile = mDownloadQueue.dequeue();	// get next waiting from queue
    qDebug() << "next from queue" << tile->id();

    // get source URL
    QUrl sourceUrl = tile->sourceUrl();
    qDebug() << "source" << sourceUrl;

    // create a KIO job to download the file
    QString cache = cacheFile(tile);			// make cache filename
    QUrl destUrl = QUrl::fromLocalFile(cache);
    qDebug() << "dest" << destUrl;

    KIO::FileCopyJob *job = KIO::file_copy(sourceUrl, destUrl, -1, KIO::Overwrite);
    connect(job, &KJob::result, this, &ElevationManager::slotDownloadResult);

    // Save the tile pointer as a property of the job (which is a QVariant),
    // see http://blog.bigpixel.ro/2010/04/storing-pointer-in-qvariant
    job->setProperty("tile", qVariantFromValue(static_cast<void *>(tile)));
    // start job
    job->start();
    ++mRunningJobs;
}


void ElevationManager::slotTileReady(ElevationTile *tile)
{
    emit tileReady(tile);
}
