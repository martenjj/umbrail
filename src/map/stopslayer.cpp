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

#include "stopslayer.h"

#include <qdebug.h>
#include <qicon.h>

#include <klocalizedstring.h>
#include <kcolorscheme.h>
#include <kiconloader.h>

#include <marble/GeoDataCoordinates.h>
#include <marble/GeoPainter.h>

#include "trackdata.h"

//////////////////////////////////////////////////////////////////////////
//									//
//  Debugging switches							//
//									//
//////////////////////////////////////////////////////////////////////////

#undef DEBUG_PAINTING

//////////////////////////////////////////////////////////////////////////
//									//
//									//
//////////////////////////////////////////////////////////////////////////

StopsLayer::StopsLayer(QWidget *pnt)
{
    qDebug();
    mStopsData = nullptr;
}


StopsLayer::~StopsLayer()
{
    qDebug() << "done";
}


QStringList StopsLayer::renderPosition() const
{
    return (QStringList("HOVERS_ABOVE_SURFACE"));
}


bool StopsLayer::render(GeoPainter *painter, ViewportParams *viewport,
                        const QString &renderPos, GeoSceneLayer *layer)
{
    if (mStopsData==nullptr) return (true);		// nothing to draw

    for (int i = 0; i<mStopsData->count(); ++i)
    {
        const TrackDataWaypoint *tdw = mStopsData->at(i);

        // TODO: combine with same in WaypointsLayer
#ifdef DEBUG_PAINTING
        qDebug() << "draw stop" << i << tdw->name();
#endif
        GeoDataCoordinates coord(tdw->longitude(), tdw->latitude(),
                                 0, GeoDataCoordinates::Degree);

        // First the icon image
        const QPixmap img = tdw->icon().pixmap(KIconLoader::SizeSmall);
        if (!img.isNull())				// icon image available
        {
            painter->drawPixmap(coord, img);
        }
        else						// draw our own marker
        {
            painter->setPen(QPen(Qt::red, 2));
            painter->setBrush(Qt::yellow);
            painter->drawEllipse(coord, 12, 12);
        }

        // Finally the waypoint text
        painter->save();
        painter->translate(15, 3);			// offset text from point
        painter->setPen(Qt::gray);			// draw with drop shadow
        painter->drawText(coord, tdw->name());
        painter->translate(-1, -1);
        painter->setPen(Qt::black);
        painter->drawText(coord, tdw->name());
        painter->restore();
    }

    return (true);
}


void StopsLayer::setStopsData(const QList<const TrackDataWaypoint *> *data)
{
    mStopsData = data;
    if (mStopsData==nullptr) qDebug() << "data cleared";
    else qDebug() << "data set" << mStopsData->count() << "points";
}
