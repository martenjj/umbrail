//////////////////////////////////////////////////////////////////////////
//									//
//  Project:	NavTracks						//
//  Edit:	06-Aug-21						//
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

#ifndef MAPBROWSER_H
#define MAPBROWSER_H


class QWidget;
class QRectF;
class TrackDataAbstractPoint;


namespace MapBrowser
{
    enum MapProvider
    {
        OSM,						// OpenStreetMap
        Google,						// Google Maps
        Bing						// Bing Maps
    };

    void openBrowser(MapBrowser::MapProvider map, const QRectF &displayedArea,
                     const TrackDataAbstractPoint *selectedPoint, QWidget *pnt = nullptr);
};

#endif							// MAPBROWSER_H