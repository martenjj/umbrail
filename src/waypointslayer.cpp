
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
    return (dynamic_cast<const TrackDataWaypoint *>(item)!=NULL);
}



bool WaypointsLayer::isDirectContainer(const TrackDataItem *item) const
{
    // Only folders contain waypoints to be drawn
    return (dynamic_cast<const TrackDataFolder *>(item)!=NULL);
}



bool WaypointsLayer::isIndirectContainer(const TrackDataItem *item) const
{
    // Files or folders can include waypoints
    return (dynamic_cast<const TrackDataFile *>(item)!=NULL ||
            dynamic_cast<const TrackDataFolder *>(item)!=NULL);
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
        if (tdw==NULL) continue;

#ifdef DEBUG_PAINTING
        qDebug() << "draw waypoint" << tdw->name();
#endif
        GeoDataCoordinates coord(tdw->longitude(), tdw->latitude(),
                                 0, GeoDataCoordinates::Degree);

        // Set pen for bearing and range lines, if required
        painter->setPen(QPen(Qt::black, 2, Qt::DashLine));
        painter->setBrush(QBrush());

        // First of all the bearing lines, if there are any
        const QString brg = tdw->metadata("bearingline");
        if (!brg.isEmpty())
        {
            const QStringList brgs = brg.split(';', QString::SkipEmptyParts);
            foreach (const QString &brgVal, brgs)
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
        const QString rng = tdw->metadata("rangering");
        if (!rng.isEmpty())
        {
            const QStringList rngs = rng.split(';', QString::SkipEmptyParts);
            foreach (const QString &rngVal, rngs)
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
