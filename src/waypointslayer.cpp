
#include "waypointslayer.h"

#include <kdebug.h>
#include <klocale.h>
#include <kcolorscheme.h>
#include <kiconloader.h>

#include <marble/GeoPainter.h>

#include "settings.h"
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

static const int POINT_CIRCLE_SIZE = 20;		// size of selected circle

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

WaypointsLayer::WaypointsLayer(QWidget *pnt)
    : LayerBase(pnt)
{
    kDebug();
}



WaypointsLayer::~WaypointsLayer()
{
    kDebug() << "done";
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
    kDebug() << "waypoints for" << item->name() << "count" << cnt;
#endif

    for (int i = 0; i<cnt; ++i)
    {
        const TrackDataWaypoint *tdw = dynamic_cast<const TrackDataWaypoint *>(item->childAt(i));
        if (tdw==NULL) continue;

#ifdef DEBUG_PAINTING
        kDebug() << "draw waypoint" << tdw->name();
#endif
        GeoDataCoordinates coord(tdw->longitude(), tdw->latitude(),
                                 0, GeoDataCoordinates::Degree);

        // First the selection marker
        // Not "isSelected" - only for the selected waypoint, not the container
        if (tdw->selectionId()==mSelectionId)
        {
            setSelectionColours(painter, false);	// pen only, not brush
            painter->drawEllipse(coord, POINT_CIRCLE_SIZE, POINT_CIRCLE_SIZE);
        }

        // Then the waypoint icon image
        const QPixmap img = KIconLoader::global()->loadIcon(tdw->iconName(), KIconLoader::NoGroup, KIconLoader::SizeSmall,
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
