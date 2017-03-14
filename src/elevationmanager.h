//////////////////////////////////////////////////////////////////////////
//									//
//  Project:	NavTracks						//
//  Edit:	14-Mar-17						//
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

#ifndef ELEVATIONMANAGER_H
#define ELEVATIONMANAGER_H


#include <qobject.h>
#include <qmap.h>
#include <qqueue.h>
#include <qthread.h>

#include "elevationtile.h"


class QTimer;
class KJob;
class ElevationTile;


class ElevationManager : public QObject
{
    Q_OBJECT

public:
    static ElevationManager *self();

    const ElevationTile *requestTile(double lat, double lon);

signals:
    void tileReady(const ElevationTile *tile);
    void tileReadyInternal(ElevationTile *tile);

private:
    explicit ElevationManager(QObject *pnt = NULL);
    virtual ~ElevationManager();

    void startDownload(ElevationTile *tile);
    QString cacheFile(const ElevationTile *tile);
    void loadTile(ElevationTile *tile, const QString &file);

private slots:
    void slotNextFromQueue();
    void slotDownloadResult(KJob *job);
    void slotTileReady(ElevationTile *tile);
    void slotLoaderThreadFinished();

private:
    QMap<ElevationTile::TileId,ElevationTile *> mTiles;
    QQueue<ElevationTile *> mDownloadQueue;
    int mRunningJobs;
    QTimer *mQueueTimer;
};


class LoaderThread : public QThread
{
    Q_OBJECT

public:
    LoaderThread(ElevationTile *tileToLoad, const QString &cacheFile, QObject *pnt = NULL);
    virtual ~LoaderThread();

    ElevationTile *tile() const				{ return (mTile); }

protected:
    virtual void run() override;

private:
    ElevationTile *mTile;
    QString mFile;
};

#endif							// ELEVATIONMANAGER_H
