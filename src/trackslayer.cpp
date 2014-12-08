
#include "trackslayer.h"

#include <kdebug.h>
#include <klocale.h>
#include <kcolorscheme.h>

#include <marble/GeoPainter.h>

#include "settings.h"
#include "mapview.h"
#include "trackdata.h"


#undef DEBUG_PAINTING


static const int POINTS_PER_BLOCK = 50;			// points drawn per polyline
static const int POINT_CIRCLE_SIZE = 10;		// size of point circle
static const int POINT_SELECTED_WIDTH = 3;		// line width for selected points



TracksLayer::TracksLayer(QWidget *pnt)
    : LayerBase(pnt)
{
    kDebug();
}



TracksLayer::~TracksLayer()
{
    kDebug() << "done";
}



bool TracksLayer::isApplicableItem(const TrackDataItem *item) const
{
    // We are only interested in trackpoints
    return (dynamic_cast<const TrackDataTrackpoint *>(item)!=NULL);
}



bool TracksLayer::isDirectContainer(const TrackDataItem *item) const
{
    // Only segments contain trackpoints to be drawn
    return (dynamic_cast<const TrackDataSegment *>(item)!=NULL);
}



bool TracksLayer::isIndirectContainer(const TrackDataItem *item) const
{
    // Files, tracks or segments can include trackpoints
    return (dynamic_cast<const TrackDataFile *>(item)!=NULL ||
            dynamic_cast<const TrackDataTrack *>(item)!=NULL ||
            dynamic_cast<const TrackDataSegment *>(item)!=NULL);
}



void TracksLayer::doPaintItem(const TrackDataItem *item, GeoPainter *painter, bool isSelected) const
{
    const int cnt = item->childCount();
#ifdef DEBUG_PAINTING
    kDebug() << "trackpoints for" << item->name() << "count" << cnt;
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
    painter->setPen(QPen(col, 4));

    GeoDataLineString lines;				// generated coordinate list
    int start = 0;					// start point in list
    while (start<(cnt-1))				// while more blocks to do
    {
#ifdef DEBUG_PAINTING
        kDebug() << "starting at" << start;
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
        kDebug() << "block of" << sofar << "points";
#endif
        if (lines.size()<2) break;			// nothing more to draw
        painter->drawPolyline(lines);			// draw track in its colour

        // Step back one point from the last one drawn, so that
        // the line connecting the last point of the previous
        // block to the first of the next is drawn.
        start += sofar-1;
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


        // TODO: colour selection to base class

        // Finally, draw the selected points along the line, if there are any.
        // Do this last so that the selection markers always show up on top.

        if (Settings::selectedUseSystemColours())
        {						// use system selection colours
            KColorScheme sch(QPalette::Active, KColorScheme::Selection);
            painter->setPen(QPen(sch.background().color(), POINT_SELECTED_WIDTH));
            painter->setBrush(sch.foreground());
        }
        else						// our own custom colours
        {
            painter->setPen(QPen(Settings::selectedMarkOuter(), POINT_SELECTED_WIDTH));
            painter->setBrush(Settings::selectedMarkInner());
        }

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
