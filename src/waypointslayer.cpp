
#include "waypointslayer.h"

#include <qdebug.h>
#include <qicon.h>

#include <klocalizedstring.h>
#include <kcolorscheme.h>
#include <kiconloader.h>

#include <marble/GeoPainter.h>

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

        // First of all the bearing lines, if there are any
        const QString brg = tdw->metadata("bearingline");
        if (!brg.isEmpty())
        {
            const QStringList brgs = brg.split(';', QString::SkipEmptyParts);
            foreach (const QString &brg, brgs)
            {
                GeoDataCoordinates coord2 = coord.moveByBearing(DEGREES_TO_RADIANS(brg.toDouble()),
                                                                BEARING_LINE_LENGTH);

                // Using Marble::Tessellate gives a great circle line.
                // Not a great difference at small scales, but
                // better to have the best accuracy.
                GeoDataLineString lineString(Marble::Tessellate);
                lineString << coord << coord2;

                painter->setPen(QPen(Qt::black, 2, Qt::DashLine));
                painter->drawPolyline(lineString);
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
