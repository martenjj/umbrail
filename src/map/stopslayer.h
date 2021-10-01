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

#ifndef STOPSLAYER_H
#define STOPSLAYER_H
 
#include <qstring.h>
#include <marble/LayerInterface.h>

using namespace Marble;

class QWidget;
class TrackDataWaypoint;


class StopsLayer : public Marble::LayerInterface
{
public:
    explicit StopsLayer(QWidget *pnt = nullptr);
    virtual ~StopsLayer();

    qreal zValue() const override		{ return (5.0); }
    QStringList renderPosition() const override;

    bool render(GeoPainter *painter, ViewportParams *viewport,
                const QString &renderPos = "NONE", GeoSceneLayer *layer = nullptr) override;

    void setStopsData(const QList<const TrackDataWaypoint *> *data);

private:
    const QList<const TrackDataWaypoint *> *mStopsData;
};

#endif							// WAYPOINTSLAYER_H
