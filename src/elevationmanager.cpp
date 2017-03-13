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

#include "elevationdownloader.h"


ElevationManager::ElevationManager(QObject *pnt)
    : QObject(pnt)
{
    qDebug();
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


ElevationTile *ElevationManager::tile(double lat, double lon)
{
    qDebug() << "for lat" << lat << "lon" << lon;

    ElevationTile *t = NULL;

    ElevationTile::TileId id = ElevationTile::makeTileId(lat, lon);
    if (mTiles.contains(id))				// do we have tile alredy?
    {
        t = mTiles.value(id);				// get from our tile map
        qDebug() << "  have tile" << t->id();
    }
    else						// need a new tile
    {
        t = new ElevationTile(id);			// create one for that location
        qDebug() << "  new tile" << t->id();
        mTiles[id] = t;					// save in our tile map
        ElevationDownloader::self()->startDownload(t);	// start to fetch tile
    }

    return (t);
}
