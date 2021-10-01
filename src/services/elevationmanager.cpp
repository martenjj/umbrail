//////////////////////////////////////////////////////////////////////////
//									//
//  Project:	Umbrail - GPX track viewer and editor			//
//									//
//////////////////////////////////////////////////////////////////////////
//									//
//  Copyright (c) 2014-2021 Jonathan Marten <jjm@keelhaul.me.uk>	//
//  Home and download page: <http://github.com/martenjj/umbrail>	//
//									//
//  This program is free software; you can redistribute it and/or	//
//  modify it under the terms of the GNU General Public License as	//
//  published by the Free Software Foundation, either version 3 of	//
//  the License or (at your option) any later version.			//
//									//
//  It is distributed in the hope that it will be useful, but		//
//  WITHOUT ANY WARRANTY;  without even the implied warranty of		//
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the	//
//  GNU General Public License for more details.			//
//									//
//  You should have received a copy of the GNU General Public License	//
//  along with this program;  see the file COPYING for further		//
//  details.  If not, see <http://gnu.org/licenses/gpl>.      		//
//									//
//////////////////////////////////////////////////////////////////////////

#include "elevationmanager.h"

#include <qdebug.h>
#include <qtimer.h>
#include <qstandardpaths.h>
#include <qurl.h>
#include <qdir.h>
#include <qguiapplication.h>

#include <kmessagebox.h>
#include <klocalizedstring.h>

#include <kio/filecopyjob.h>

#include "elevationtile.h"


#undef DEBUG_REQUESTS


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


const ElevationTile *ElevationManager::requestTile(double lat, double lon, bool wantImmediateSignal)
{
    ElevationTile *tile = nullptr;

    ElevationTile::TileId id = ElevationTile::makeTileId(lat, lon);
    if (mTiles.contains(id))				// do we have tile alredy?
    {
        tile = mTiles.value(id);			// get from our tile map
#ifdef DEBUG_REQUESTS
        qDebug() << "for lat" << lat << "lon" << lon << "have tile" << tile->id();
#endif
        //if (wantImmediateSignal) emit tileReadyInternal(tile);
        if (tile->state()==ElevationTile::Loaded && wantImmediateSignal) emit tileReadyInternal(tile);
    }
    else						// need a new tile
    {
        tile = new ElevationTile(id);			// create one for that location
        qDebug() << "for lat" << lat << "lon" << lon << "new tile" << tile->id();
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
        loadTile(tile, cache);				// do in loader thread
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
    Q_ASSERT(copyJob!=nullptr);
    --mRunningJobs;

    // get tile pointer from job
    ElevationTile *tile = static_cast<ElevationTile *>(copyJob->property("tile").value<void *>());
    Q_ASSERT(tile!=nullptr);

    if (!job->error())					// check download success
    {
        QString cache = copyJob->destUrl().toLocalFile();
        loadTile(tile, cache);				// do in loader thread
    }
    else						// error downloading
    {
        // report error
        KMessageBox::error(nullptr,
                           xi18nc("@info", "Error downloading tile <link>%1</link><nl/>to cache file <link>%2</link><nl/>%3",
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
    qDebug() << "running" << mRunningJobs << "waiting" << mDownloadQueue.count();

    if (mDownloadQueue.isEmpty())			// any more downloads waiting?
    {							// queue empty, nothing to do
        mQueueTimer->stop();
        return;
    }

    if (mRunningJobs>=MAX_DOWNLOADS)			// enough running already?
    {							// try again next timer tick
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
    job->setProperty("tile", QVariant::fromValue(static_cast<void *>(tile)));
    // start job
    job->start();
    ++mRunningJobs;
}


void ElevationManager::slotTileReady(ElevationTile *tile)
{
    emit tileReady(tile);
}


void ElevationManager::loadTile(ElevationTile *tile, const QString &file)
{
    QGuiApplication::setOverrideCursor(Qt::BusyCursor);

    LoaderThread *thr = new LoaderThread(tile, file, this);
    connect(thr, &QThread::finished, this, &ElevationManager::slotLoaderThreadFinished);
    thr->start(QThread::LowPriority);
}


void ElevationManager::slotLoaderThreadFinished()
{
    qDebug();

    LoaderThread *thr = qobject_cast<LoaderThread *>(sender());
    Q_ASSERT(thr!=nullptr);
    ElevationTile *tile = thr->tile();
    Q_ASSERT(tile!=nullptr);

    if (tile->state()==ElevationTile::Loaded) emit tileReadyInternal(tile);
    thr->deleteLater();

    QGuiApplication::restoreOverrideCursor();
}


LoaderThread::LoaderThread(ElevationTile *tileToLoad, const QString &cacheFile, QObject *pnt)
    : QThread(pnt)
{
    qDebug() << "for" << tileToLoad->id();
    mTile = tileToLoad;
    mFile = cacheFile;
}


LoaderThread::~LoaderThread()
{
    qDebug() << mTile->id();
}


void LoaderThread::run()
{
    qDebug() << mTile->id();
    QThread::msleep(100);
    mTile->load(mFile);					// load cache file into tile
}
