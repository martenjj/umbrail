//////////////////////////////////////////////////////////////////////////
//									//
//  Project:	Umbrail - GPX track viewer and editor			//
//									//
//////////////////////////////////////////////////////////////////////////
//									//
//  Copyright (c) 2014-2022 Jonathan Marten <jjm@keelhaul.me.uk>	//
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

#include "waypointslayer.h"

#include <qdebug.h>
#include <qicon.h>

#include <klocalizedstring.h>
#include <kcolorscheme.h>
#include <kiconloader.h>

#include <marble/GeoPainter.h>
#include <marble/ViewportParams.h>

#include "settings.h"
#include "mapview.h"
#include "dataindexer.h"
#include "trackdata.h"
#include "units.h"


//////////////////////////////////////////////////////////////////////////
//									//
//  Debugging switches							//
//									//
//////////////////////////////////////////////////////////////////////////

#undef DEBUG_PAINTING

//////////////////////////////////////////////////////////////////////////
//									//
// Painting parameters							//
//									//
//////////////////////////////////////////////////////////////////////////

static const int POINT_CIRCLE_SIZE = 20;		// size of selected circle

// Earth circumference is 2*PI*radius.  This corresponds to an value
// of 2*PI for the 'distance' parameter to GeoDataCoordinates::moveByBearing().
//
// The 2*PI factor cancels out, so this value gives a bearing line
// of that length in kilometres.

static constexpr double BEARING_LINE_LENGTH_KM = 10;
static constexpr double BEARING_LINE_LENGTH = BEARING_LINE_LENGTH_KM/Units::EARTH_RADIUS_KM;

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

WaypointsLayer::WaypointsLayer(QWidget *pnt)
    : LayerBase(pnt)
{
    qDebug();
}



WaypointsLayer::~WaypointsLayer()
{
    qDebug() << "done";
}



bool WaypointsLayer::isApplicableItem(const TrackDataItem *item) const
{
    // We are only interested in waypoints
    return (dynamic_cast<const TrackDataWaypoint *>(item)!=nullptr);
}



bool WaypointsLayer::isDirectContainer(const TrackDataItem *item) const
{
    // Only folders contain waypoints to be drawn
    return (dynamic_cast<const TrackDataFolder *>(item)!=nullptr);
}



bool WaypointsLayer::isIndirectContainer(const TrackDataItem *item) const
{
    // Files or folders can include waypoints
    return (dynamic_cast<const TrackDataFile *>(item)!=nullptr ||
            dynamic_cast<const TrackDataFolder *>(item)!=nullptr);
}



void WaypointsLayer::doPaintItem(const TrackDataItem *item, GeoPainter *painter, bool isSelected) const
{
    const int cnt = item->childCount();
#ifdef DEBUG_PAINTING
    qDebug() << "waypoints for" << item->name() << "count" << cnt;
#endif

    for (int i = 0; i<cnt; ++i)
    {
        const TrackDataWaypoint *tdw = dynamic_cast<const TrackDataWaypoint *>(item->childAt(i));
        if (tdw==nullptr) continue;

#ifdef DEBUG_PAINTING
        qDebug() << "draw waypoint" << tdw->name();
#endif
        GeoDataCoordinates coord(tdw->longitude(), tdw->latitude(),
                                 0, GeoDataCoordinates::Degree);

        // TEMPORARY (hopefully): Workaround for Qt 5.15.2/5.15.3
        // painting bug, drawing a dashed line for bearing or
        // range ring crashes:
        //
        // ASSERT: "dpos >= 0" in file src/gui/painting/qstroker.cpp, line 1192
        //
        // QDashStroker::processCurrentSubpath() at src/gui/painting/qstroker.cpp:1192
        // QStrokerOps::end() at src/gui/painting/qstroker.cpp:221
        // QDashStroker::end() at src/gui/painting/qstroker_p.h:397
        // QPaintEngineEx::stroke() at src/gui/painting/qpaintengineex.cpp:535
        // QPaintEngineEx::drawEllipse() at src/gui/painting/qpaintengineex.cpp:860
        // QPainter::drawEllipse() at src/gui/painting/qpainter.cpp:4294
        // QPainter::drawEllipse() at QtGui/qpainter.h:677
        // WaypointsLayer::doPaintItem() at src/map/waypointslayer.cpp:192
        // LayerBase::paintDataTree() at src/map/layerbase.cpp:191
        // LayerBase::paintDataTree() at src/map/layerbase.cpp:202
        // LayerBase::render() at src/core/filesmodel.h:60
        // LayerBase::render() at src/map/layerbase.cpp:135
        // Marble::LayerManager::renderLayers() at src/lib/marble/LayerManager.cpp:177
        // Marble::MarbleMap::paint() at src/lib/marble/MarbleMap.cpp:856
        // Marble::MarbleWidget::paintEvent() at src/lib/marble/MarbleWidget.cpp:720

        // Set pen for bearing and range lines, if required
        //painter->setPen(QPen(Qt::black, 2, Qt::DashLine));
        painter->setPen(QPen(Qt::darkGray, 2, Qt::SolidLine));
        painter->setBrush(QBrush());

        // First of all the bearing lines, if there are any
        const QString brg = tdw->metadata("bearingline").toString();
        if (!brg.isEmpty())
        {
            const QStringList brgs = brg.split(';', Qt::SkipEmptyParts);
            for (const QString &brgVal : brgs)
            {
                GeoDataCoordinates coord2 = coord.moveByBearing(DEGREES_TO_RADIANS(brgVal.toDouble()),
                                                                Units::lengthToInternal(BEARING_LINE_LENGTH_KM,
                                                                                        Units::LengthKilometres));

                // Using Marble::Tessellate gives a great circle line.
                // Not a great difference at small scales, but
                // better to have the best accuracy.
                GeoDataLineString lineString(Marble::Tessellate);
                lineString << coord << coord2;

                painter->drawPolyline(lineString);
            }
        }

        // The the range rings, if there are any
        const QString rng = tdw->metadata("rangering").toString();
        if (!rng.isEmpty())
        {
            const QStringList rngs = rng.split(';', Qt::SkipEmptyParts);
            for (const QString &rngVal : rngs)
            {
                // To draw the range ring we convert all of the coordinates to screen
                // pixels and use the underlying Marble::ClipPainter directly, bypassing
                // Marble's GeoPainter.
                //
                // This is for two reasons:  firstly, the optimisation that GeoPainter
                // applies is too aggressive, drawing nothing if the centre point is
                // not visible - parts of the ring may be visible even if the central
                // waypoint or any/all of its cardinal points are not.  Secondly,
                // using GeoPainter::drawEllipse() with projected coordinates
                // (as opposed to screen pixels) seems to be far too enthusiastic at
                // flattening the curve and draws it with obvious straight line segments
                // at the right and left.

                qreal xCent, yCent;			// screen position of centre
                viewport()->screenCoordinates(coord, xCent, yCent);
					                // offset to right/top of circle
                const double offset = Units::lengthToInternal(rngVal.toDouble(), Units::LengthMetres);

                const GeoDataCoordinates coordRight = coord.moveByBearing(DEGREES_TO_RADIANS(90), offset);
                qreal xRight, yRight;			// screen position of right
                viewport()->screenCoordinates(coordRight, xRight, yRight);

                const GeoDataCoordinates coordTop = coord.moveByBearing(DEGREES_TO_RADIANS(0), offset);
                qreal xTop, yTop;			// screen position of top
                viewport()->screenCoordinates(coordTop, xTop, yTop);
							// now can draw the circle
                ClipPainter *cp = static_cast<ClipPainter *>(painter);
                cp->drawEllipse(QPointF(xCent, yCent), fabs(xRight-xCent), fabs(yTop-yCent));
            }
        }

        // Then the selection marker
        // Not "isSelected" - only for the selected waypoint, not the container
        if (tdw->selectionId()==mSelectionId)
        {
            setSelectionColours(painter, false);	// pen only, not brush
            painter->drawEllipse(coord, POINT_CIRCLE_SIZE, POINT_CIRCLE_SIZE);
        }

        // Then the waypoint icon image
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
}



void WaypointsLayer::doPaintDrag(const SelectionRun *run, GeoPainter *painter) const
{
    painter->setPen(QPen(Qt::black, 2, Qt::SolidLine));

    const GeoDataLineString *thesePoints = run->thesePoints();
    for (int i = 0; i<thesePoints->size(); ++i)
    {
        GeoDataCoordinates coord(applyOffset(thesePoints->at(i)));
        painter->drawEllipse(coord, POINT_CIRCLE_SIZE, POINT_CIRCLE_SIZE);
    }
}
