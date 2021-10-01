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

#include "trackslayer.h"

#include <qdebug.h>

#include <klocalizedstring.h>
#include <kcolorscheme.h>

#include <marble/GeoPainter.h>

#include "settings.h"
#include "mapcontroller.h"
#include "mapview.h"
#include "trackdata.h"

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

static const int POINTS_PER_BLOCK = 50;			// points drawn per polyline
static const int POINT_CIRCLE_SIZE = 10;		// size of point circle

static const int ARROW_PER_SEGMENTS = 5;		// arrow every this many segments
static const int ARROW_MIN_LENGTH = 20;			// minimum segment length for arrow
static const double ARROW_TRI_WIDTH = 10.0/2.0;		// half of direction arrow width
static const double ARROW_TRI_HEIGHT = 12.0/3.0;	// third of direction arrow height

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

TracksLayer::TracksLayer(QWidget *pnt)
    : LayerBase(pnt)
{
    qDebug();
}



TracksLayer::~TracksLayer()
{
    qDebug() << "done";
}



bool TracksLayer::isApplicableItem(const TrackDataItem *item) const
{
    // We are only interested in trackpoints
    return (dynamic_cast<const TrackDataTrackpoint *>(item)!=nullptr);
}



bool TracksLayer::isDirectContainer(const TrackDataItem *item) const
{
    // Only segments contain trackpoints to be drawn
    return (dynamic_cast<const TrackDataSegment *>(item)!=nullptr);
}



bool TracksLayer::isIndirectContainer(const TrackDataItem *item) const
{
    // Files, tracks or segments can include trackpoints
    return (dynamic_cast<const TrackDataFile *>(item)!=nullptr ||
            dynamic_cast<const TrackDataTrack *>(item)!=nullptr ||
            dynamic_cast<const TrackDataSegment *>(item)!=nullptr);
}


void TracksLayer::doPaintItem(const TrackDataItem *item, GeoPainter *painter, bool isSelected) const
{
    const int cnt = item->childCount();
#ifdef DEBUG_PAINTING
    qDebug() << "trackpoints for" << item->name() << "count" << cnt;
#endif

    // Scan along the segment, assembling the coordinates into a list,
    // and draw them as a polyline.
    //
    // Some polyline segments appear to be not drawn if there are too many
    // of them at high magnifications - is this a limitation in Marble?
    // Split the drawing up if there are too many - this may make clipping
    // more efficient too.

    QColor col = MapView::resolveLineColour(item);
    painter->setBrush(Qt::NoBrush);
    painter->setPen(QPen(col, 3));			// odd width gives symmetrical arrows

    GeoDataLineString lines;				// generated coordinate list
    int start = 0;					// start point in list
    while (start<(cnt-1))				// while more blocks to do
    {
#ifdef DEBUG_PAINTING
        qDebug() << "starting at" << start;
#endif
        lines.clear();					// empty the list
        int sofar = 0;					// points so far this block
        for (int i = start; i<cnt; ++i)			// up to end of list
        {
            const TrackDataTrackpoint *tdp = static_cast<const TrackDataTrackpoint *>(item->childAt(i));
            GeoDataCoordinates coord(tdp->longitude(), tdp->latitude(),
                                     0, GeoDataCoordinates::Degree);
            lines.append(coord);			// add point to list
            ++sofar;					// count how many this block
            if (sofar>=POINTS_PER_BLOCK) break;		// too many, draw and start again
        }

#ifdef DEBUG_PAINTING
        qDebug() << "block of" << sofar << "points";
#endif
        if (lines.size()<2) break;			// nothing more to draw
        painter->drawPolyline(lines);			// draw track in its colour

        // Step back one point from the last one drawn, so that
        // the line connecting the last point of the previous
        // block to the first of the next is drawn.
        start += sofar-1;
    }

    if (Settings::showTrackArrows())
    {
        // Then overlay the track with the direction arrows.
        //
        // So as not to clutter the view too much, an arrow will only be drawn on
        // a minimum of every ARROW_PER_SEGMENTS segments.  In order to get a
        // reasonable angle resolution, the manhattan length of the segment must
        // be at least ARROW_MIN_LENGTH pixels.
        //
        // In order that at least some arrows are drawn if these criteria are too
        // strict (i.e. in the case of dense tracks or at low zoom factors), if
        // there has been no arrow drawn for the last 3*ARROW_PER_SEGMENTS then
        // the minimum length is reduced to ARROW_MIN_LENGTH/4.

        int sincelast = ARROW_PER_SEGMENTS-2;		// start considering immediately
        for (int i = 0; i<(cnt-2); ++i)			// scan along each line segment
        {
            ++sincelast;				// how many since last drawn
            if (sincelast<ARROW_PER_SEGMENTS) continue;	// not enough since last time

            const TrackDataTrackpoint *p1 = static_cast<const TrackDataTrackpoint *>(item->childAt(i));
            const TrackDataTrackpoint *p2 = static_cast<const TrackDataTrackpoint *>(item->childAt(i+1));

            qreal x1, y1;				// coordinates of this point
            qreal x2, y2;				// coordinates of next point
            bool onScreen = mapController()->view()->screenCoordinates(p1->longitude(), p1->latitude(), x1, y1) &&
                            mapController()->view()->screenCoordinates(p2->longitude(), p2->latitude(), x2, y2);
            if (!onScreen) continue;			// map to screen coordinates

            int len = qRound((qAbs(x1-x2)+qAbs(y1-y2))/2);
							// length of this segment
            if (len<ARROW_MIN_LENGTH)			// is the segment long enough?
            {						// no, but look again
                if (sincelast<3*ARROW_PER_SEGMENTS) continue;
							// not too many since last one
                if (len<(ARROW_MIN_LENGTH/4)) continue;	// if too many, try lower limit
            }

#ifdef DEBUG_PAINTING
            qDebug() << "arrow at" << i << "length" << len;
#endif

            // Draw the arrow at the midpoint of this line segment,
            // in the line colour with no outline.

            painter->setPen(Qt::NoPen);
            painter->setBrush(col);

            const double theta = atan2(y2-y1, x2-x1);	// angle of line
            const double tc = cos(theta);
            const double ts = sin(theta);

            const double xc = (x1+x2)/2;		// centre point of line
            const double yc = (y1+y2)/2;

            const double xb = xc - tc*ARROW_TRI_HEIGHT;	// base point of triangle
            const double yb = yc - ts*ARROW_TRI_HEIGHT;
							// apex of triangle
            const int xa = qRound(xc + tc*ARROW_TRI_HEIGHT*2.0);
            const int ya = qRound(yc + ts*ARROW_TRI_HEIGHT*2.0);
							// base vertex 1
            const int xd = qRound(xb - ts*ARROW_TRI_WIDTH);
            const int yd = qRound(yb + tc*ARROW_TRI_WIDTH);
							// base vertex 2
            const int xe = qRound(xb + ts*ARROW_TRI_WIDTH);
            const int ye = qRound(yb - tc*ARROW_TRI_WIDTH);

            const QPoint tri[] =			// list of polygon points
            {
                QPoint(xa, ya),
                QPoint(xd, yd),
                QPoint(xe, ye)
            };

            painter->drawConvexPolygon(tri, 3);		// draw the arrow
            sincelast = 0;				// reset for counting onwards
        }
    }

    if (isSelected)
    {
        // Next, if the line is selected, draw the point markers.  In the
        // same track colour, circled with a black cosmetic line.

        painter->setPen(QPen(Qt::black, 0));
        painter->setBrush(col);

        for (int i = 0; i<cnt; ++i)
        {
            const TrackDataTrackpoint *tdp = static_cast<const TrackDataTrackpoint *>(item->childAt(i));
            GeoDataCoordinates coord(tdp->longitude(), tdp->latitude(),
                                     0, GeoDataCoordinates::Degree);
            painter->drawEllipse(coord, POINT_CIRCLE_SIZE, POINT_CIRCLE_SIZE);
        }

        // Finally, draw the selected points along the line, if there are any.
        // Do this last so that the selection markers always show up on top.

        setSelectionColours(painter);
        for (int i = 0; i<cnt; ++i)
        {
            const TrackDataTrackpoint *tdp = static_cast<const TrackDataTrackpoint *>(item->childAt(i));
            if (tdp->selectionId()==mSelectionId)
            {
                GeoDataCoordinates coord(tdp->longitude(), tdp->latitude(),
                                         0, GeoDataCoordinates::Degree);
                painter->drawEllipse(coord, POINT_CIRCLE_SIZE, POINT_CIRCLE_SIZE);
            }
        }
    }
}


void TracksLayer::doPaintDrag(const SelectionRun *run, GeoPainter *painter) const
{
    const GeoDataCoordinates *prevPoint = run->prevPoint();
    const GeoDataLineString *thesePoints = run->thesePoints();
    const GeoDataCoordinates *nextPoint = run->nextPoint();

    GeoDataLineString line;
    if (prevPoint->isValid())
    {
        line.append(*prevPoint);
        line.append(applyOffset(thesePoints->first()));

        painter->setPen(QPen(Qt::black, 2, Qt::DotLine));
        painter->drawPolyline(line);
    }

    line.clear();
    for (int i = 0; i<thesePoints->size(); ++i)
    {
        line.append(applyOffset(thesePoints->at(i)));
    }

    painter->setPen(QPen(Qt::black, 2, Qt::SolidLine));
    painter->drawPolyline(line);

    if (nextPoint->isValid())
    {
        line.clear();
        line.append(applyOffset(thesePoints->last()));
        line.append(*nextPoint);

        painter->setPen(QPen(Qt::black, 2, Qt::DotLine));
        painter->drawPolyline(line);
    }
}
