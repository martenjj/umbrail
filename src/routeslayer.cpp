
#include "routeslayer.h"

#include <math.h>

#include <kdebug.h>
#include <klocale.h>
#include <kcolorscheme.h>
#include <kiconloader.h>

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

static const int POINT_CIRCLE_SIZE = 20;		// size of point circle

static const int ARROW_MIN_LENGTH = 20;			// minimum segment length for arrow
static const double ARROW_TRI_WIDTH = 10.0/2.0;		// half of direction arrow width
static const double ARROW_TRI_HEIGHT = 12.0/3.0;	// third of direction arrow height

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

// TODO: can be a subclass of TracksLayer?
RoutesLayer::RoutesLayer(QWidget *pnt)
    : LayerBase(pnt)
{
    kDebug();
}



RoutesLayer::~RoutesLayer()
{
    kDebug() << "done";
}



bool RoutesLayer::isApplicableItem(const TrackDataItem *item) const
{
    // We are only interested in routepoints
    return (dynamic_cast<const TrackDataRoutepoint *>(item)!=NULL);
}



bool RoutesLayer::isDirectContainer(const TrackDataItem *item) const
{
    // Only routes contain routepoints to be drawn
    return (dynamic_cast<const TrackDataRoute *>(item)!=NULL);
}



bool RoutesLayer::isIndirectContainer(const TrackDataItem *item) const
{
    // Files or routes can include routepoints
    return (dynamic_cast<const TrackDataFile *>(item)!=NULL ||
            dynamic_cast<const TrackDataRoute *>(item)!=NULL);
}


void RoutesLayer::doPaintItem(const TrackDataItem *item, GeoPainter *painter, bool isSelected) const
{
    const int cnt = item->childCount();
#ifdef DEBUG_PAINTING
    kDebug() << "routepoints for" << item->name() << "count" << cnt;
#endif

    // Scan along the segment, assembling the coordinates into a list,
    // and draw them as a polyline.  We assume that routes will not be
    // so extensive as tracks, so there is no need to split it up into
    // smaller pieces.

    QColor col = MapView::resolveLineColour(item);
    painter->setBrush(Qt::NoBrush);
    painter->setPen(QPen(col, 3));			// odd width gives symmetrical arrows

    GeoDataLineString lines;				// generated coordinate list
    for (int i = 0; i<cnt; ++i)
    {
        const TrackDataRoutepoint *tdp = static_cast<const TrackDataRoutepoint *>(item->childAt(i));
        GeoDataCoordinates coord(tdp->longitude(), tdp->latitude(),
                                     0, GeoDataCoordinates::Degree);
        lines.append(coord);				// add point to list
    }
    painter->drawPolyline(lines);			// draw route in its colour

    if (Settings::showTrackArrows())
    {
        // Then overlay the route with the direction arrows.
        //
        // Again, routes will not likely be so extensive as tracks, so an arrow
        // is drawn on every line segment unless it is less than ARROW_MIN_LENGTH
        // pixels long.

        for (int i = 0; i<(cnt-1); ++i)			// scan along each line segment
        {
            const TrackDataRoutepoint *p1 = static_cast<const TrackDataRoutepoint *>(item->childAt(i));
            const TrackDataRoutepoint *p2 = static_cast<const TrackDataRoutepoint *>(item->childAt(i+1));

            qreal x1, y1;				// coordinates of this point
            qreal x2, y2;				// coordinates of next point
//             // Route segments can be relatively long, so don't bother checking whether
//             // both end points are on screen.

            bool onScreen = mapController()->view()->screenCoordinates(p1->longitude(), p1->latitude(), x1, y1) &&
                mapController()->view()->screenCoordinates(p2->longitude(), p2->latitude(), x2, y2);
            if (!onScreen) continue;			// map to screen coordinates

            int len = qRound((qAbs(x1-x2)+qAbs(y1-y2))/2);
							// length of this segment
            if (len<ARROW_MIN_LENGTH) continue;		// is the segment long enough?

#ifdef DEBUG_PAINTING
            kDebug() << "arrow at" << i << "length" << len;
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
        }
    }

    for (int i = 0; i<cnt; ++i)
    {
        const TrackDataRoutepoint *tdp = static_cast<const TrackDataRoutepoint *>(item->childAt(i));
        GeoDataCoordinates coord(tdp->longitude(), tdp->latitude(),
                                 0, GeoDataCoordinates::Degree);

        // First the selection marker
        // Not "isSelected" - only for the selected waypoint, not the container
        if (tdp->selectionId()==mSelectionId)
        {
            setSelectionColours(painter, false);	// pen only, not brush
            painter->setBrush(Qt::NoBrush);
            painter->drawEllipse(coord, POINT_CIRCLE_SIZE, POINT_CIRCLE_SIZE);
        }

        // Then the routepoint icon image
        const QPixmap img = KIconLoader::global()->loadIcon(tdp->iconName(), KIconLoader::NoGroup, KIconLoader::SizeSmall,
                                                            KIconLoader::DefaultState, QStringList(), NULL, true);
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

        // Finally the routepoint text
        painter->save();
        painter->translate(15, 3);			// offset text from point
        painter->setPen(Qt::gray);			// draw with drop shadow
        painter->drawText(coord, tdp->name());
        painter->translate(-1, -1);
        painter->setPen(Qt::black);
        painter->drawText(coord, tdp->name());
        painter->restore();
    }
}


void RoutesLayer::doPaintDrag(const SelectionRun *run, GeoPainter *painter) const
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
