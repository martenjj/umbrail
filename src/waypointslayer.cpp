
#include "waypointslayer.h"

#include <qelapsedtimer.h>
#include <qapplication.h>
#include <qevent.h>

#include <kdebug.h>
#include <klocale.h>
#include <kcolorscheme.h>
#include <kiconloader.h>

#include <marble/GeoPainter.h>
#include <marble/GeoDataPlacemark.h>

#include "filesmodel.h"
#include "filesview.h"
#include "mainwindow.h"
#include "settings.h"
#include "mapview.h"


#undef DEBUG_PAINTING
#undef DEBUG_DRAGGING
#undef DEBUG_SELECTING


static const double RADIANS_TO_DEGREES = 360/(2*M_PI);	// multiplier

static const int POINT_CIRCLE_SIZE = 20;		// size of selected circle
static const int POINT_SELECTED_WIDTH = 3;		// line width for selected points


WaypointsLayer::WaypointsLayer(QWidget *pnt)
    : QObject(pnt)
{
    kDebug();

    mMapView = qobject_cast<MapView *>(pnt);

    mClickedPoint = NULL;
//     mDraggingPoints = NULL;
    mClickTimer = new QElapsedTimer;
    mMovePointsMode = false;

    mapView()->installEventFilter(this);
}


WaypointsLayer::~WaypointsLayer()
{
//     delete mDraggingPoints;
    kDebug() << "done";
}


QStringList WaypointsLayer::renderPosition() const
{
    return (QStringList("HOVERS_ABOVE_SURFACE"));
}


qreal WaypointsLayer::zValue() const
{
    return (2.0);
}


static inline GeoDataCoordinates applyOffset(const GeoDataCoordinates &coords, qreal lonOff, qreal latOff)
{
    return (GeoDataCoordinates(coords.longitude(GeoDataCoordinates::Degree)+lonOff,
                               coords.latitude(GeoDataCoordinates::Degree)+latOff,
                               0, GeoDataCoordinates::Degree));
}


bool WaypointsLayer::render(GeoPainter *painter, ViewportParams *viewport,
                            const QString &renderPos, GeoSceneLayer *layer)
{
    const FilesModel *filesModel = mapView()->filesModel();
    if (filesModel==NULL) return (false);		// no data to use!

    const FilesView *filesView = mapView()->filesView();
    mSelectionId = (filesView!=NULL ? filesView->selectionId() : 0);

    // Paint the data in two passes.  The first does all non-selected waypoints,
    // the second selected ones.  This is so that selected waypoints show up
    // on top of all non-selected ones.  In the absence of any selection,
    // all waypoints will be painted in file and then time order (i.e. later
    // track segments on top of earlier ones).
    paintDataTree(filesModel->rootFileItem(), painter, false, false);
    paintDataTree(filesModel->rootFileItem(), painter, true, false);

#if 0
    if (mDraggingPoints!=NULL)
    {
#ifdef DEBUG_DRAGGING
        kDebug() << "paint for drag";
#endif
        for (QList<SelectionRun>::const_iterator it = mDraggingPoints->constBegin();
             it!=mDraggingPoints->constEnd(); ++it)
        {
            const SelectionRun &run = (*it);
            const GeoDataCoordinates *prevPoint = run.prevPoint();
            const GeoDataLineString *thesePoints = run.thesePoints();
            const GeoDataCoordinates *nextPoint = run.nextPoint();

            GeoDataLineString line;
            if (prevPoint->isValid())
            {
                line.append(*prevPoint);
                line.append(applyOffset(thesePoints->first(), mLonOff, mLatOff));

                painter->setPen(QPen(Qt::black, 2, Qt::DotLine));
                painter->drawPolyline(line);
            }

            line.clear();
            for (int i = 0; i<thesePoints->size(); ++i)
            {
                line.append(applyOffset(thesePoints->at(i), mLonOff, mLatOff));
            }

            painter->setPen(QPen(Qt::black, 2, Qt::SolidLine));
            painter->drawPolyline(line);

            if (nextPoint->isValid())
            {
                line.clear();
                line.append(applyOffset(thesePoints->last(), mLonOff, mLatOff));
                line.append(*nextPoint);

                painter->setPen(QPen(Qt::black, 2, Qt::DotLine));
                painter->drawPolyline(line);
            }
        }
    }
#endif

    return (true);
}


void WaypointsLayer::paintDataTree(const TrackDataItem *tdi, GeoPainter *painter, 
                                   bool doSelected, bool parentSelected)
{
    if (tdi==NULL) return;				// nothing to paint

    bool isSelected = parentSelected || (tdi->selectionId()==mSelectionId);
#ifdef DEBUG_PAINTING
    kDebug() << tdi->name() << "isselected" << isSelected << "doselected" << doSelected;
#endif

    const TrackDataWaypoint *tdw = dynamic_cast<const TrackDataWaypoint *>(tdi);
    if (tdw!=NULL)
    {
        if (!(isSelected ^ doSelected))			// check selected or not
        {

#ifdef DEBUG_PAINTING
            kDebug() << "draw waypoint" << tdw->name();
#endif
            GeoDataCoordinates coord(tdw->longitude(), tdw->latitude(),
                                     0, GeoDataCoordinates::Degree);

            // First the selection marker
            if (isSelected)
            {
                if (Settings::selectedUseSystemColours())
                {					// use system selection colours
                    KColorScheme sch(QPalette::Active, KColorScheme::Selection);
                    painter->setPen(QPen(sch.background().color(), POINT_SELECTED_WIDTH));
                    //painter->setBrush(sch.foreground());
                }
                else					// our own custom colours
                {
                    painter->setPen(QPen(Settings::selectedMarkOuter(), POINT_SELECTED_WIDTH));
                    //painter->setBrush(Settings::selectedMarkInner());
                }

                painter->drawEllipse(coord, POINT_CIRCLE_SIZE, POINT_CIRCLE_SIZE);
            }

            // Then the waypoint icon image
            const QPixmap img = KIconLoader::global()->loadIcon("favorites", KIconLoader::NoGroup, KIconLoader::SizeSmall,
                                                                KIconLoader::DefaultState, QStringList(), NULL, true);
            if (!img.isNull())				// icon image available
            {
                painter->drawPixmap(coord, img);
            }
            else					// draw our own marker
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
    else						// not a waypoint,
    {							// so we are a higher container
        const int cnt = tdi->childCount();
        for (int i = 0; i<cnt; ++i)			// just recurse to paint children
        {
            const TrackDataItem *childItem = tdi->childAt(i);
            if (dynamic_cast<const TrackDataTrack *>(childItem)!=NULL) continue;
							// no need to look into tracks
            paintDataTree(childItem, painter, doSelected, isSelected);
        }
    }
}


const TrackDataPoint *WaypointsLayer::findClickedPoint(const TrackDataItem *tdi)
{
    if (tdi==NULL) return (NULL);			// nothing to do

    const TrackDataPoint *tdp = dynamic_cast<const TrackDataPoint *>(tdi);
    if (tdp!=NULL)					// is this a point?
    {
        const double lat = tdp->latitude();
        const double lon = tdp->longitude();
        if (lat>=mLatMin && lat<=mLatMax &&
            lon>=mLonMin && lon<=mLonMax)
        {						// check within tolerance
#ifdef DEBUG_SELECTING
            kDebug() << "found point" << tdp->name();
#endif
            return (tdp);				// clicked point found
        }
    }
    else						// not a point, so a container
    {
        for (int i = 0; i<tdi->childCount(); ++i)	// recurse to search children
        {
            const TrackDataItem *childItem = tdi->childAt(i);
            const TrackDataPoint *childPoint = findClickedPoint(childItem);
            if (childPoint!=NULL) return (childPoint);
        }
    }

    return (NULL);					// nothing found
}


bool WaypointsLayer::testClickTolerance(const QMouseEvent *mev) const
{
    const int dragStart = qApp->startDragDistance();
    return (qAbs(mev->pos().x()-mClickX)<dragStart &&
            qAbs(mev->pos().y()-mClickY)<dragStart &&
            mClickTimer->elapsed()<qApp->startDragTime());
}


void WaypointsLayer::findSelectionInTree(const TrackDataItem *tdi)
{
    if (tdi==NULL) return;				// nothing to paint
    int cnt = tdi->childCount();
    if (cnt==0) return;					// quick escape if no children

#if 0
    const TrackDataItem *firstChild = tdi->childAt(0);
    if (dynamic_cast<const TrackDataPoint *>(firstChild)!=NULL)
    {							// is first child a point?
#ifdef DEBUG_SELECTING
        kDebug() << "###" << tdi->name();
#endif
        // If there are any selected points on this segment, assemble them
        // into a SelectionRun and add it to the dragging list.

        SelectionRun run;
        for (int i = 0; i<tdi->childCount(); ++i)
        {
            const TrackDataPoint *tdp = dynamic_cast<const TrackDataPoint *>(tdi->childAt(i));
#ifdef DEBUG_SELECTING
            kDebug() << "  " << i << tdp->name() << "selected?" << (tdp->selectionId()==mSelectionId);
#endif
            if (tdp->selectionId()==mSelectionId)	// this point is selected
            {
                if (run.isEmpty())			// start of a new run
                {
#ifdef DEBUG_SELECTING
                    kDebug() << "    starting new run";
#endif
                    if (i>0)				// not first point in segment
                    {
                        const TrackDataPoint *prev = dynamic_cast<const TrackDataPoint *>(tdi->childAt(i-1));
                        Q_ASSERT(prev!=NULL);
#ifdef DEBUG_SELECTING
                        kDebug() << "    setting prev point" << prev->name();
#endif
                        run.setPrevPoint(GeoDataCoordinates(prev->longitude(), prev->latitude(),
                                                            0, GeoDataCoordinates::Degree));
                    }
                }

                GeoDataCoordinates coord(tdp->longitude(), tdp->latitude(),
                                         0, GeoDataCoordinates::Degree);
#ifdef DEBUG_SELECTING
                kDebug() << "    add point" << tdp->name();
#endif
                run.addPoint(coord);
            }
            else					// this point not selected
            {
                if (!run.isEmpty())
                {
#ifdef DEBUG_SELECTING
                    kDebug() << "    setting next point" << tdp->name();
#endif
                    run.setNextPoint(GeoDataCoordinates(tdp->longitude(), tdp->latitude(),
                                                        0, GeoDataCoordinates::Degree));
                    mDraggingPoints->append(run);
#ifdef DEBUG_SELECTING
                    kDebug() << "    add run";
#endif
                    run.clear();			// clear for next time
                }
            }
        }

        if (!run.isEmpty())
        {
#ifdef DEBUG_SELECTING
            kDebug() << "add final run";
#endif
            mDraggingPoints->append(run);
        }

#ifdef DEBUG_SELECTING
        kDebug() << "### done" << "with" << mDraggingPoints->count() << "runs";
#endif
    }
    else						// first child not a point,
    {							// so we are higher container
        for (int i = 0; i<cnt; ++i)			// just recurse to search children
        {
            findSelectionInTree(tdi->childAt(i));
        }
    }
#endif
}


void WaypointsLayer::setMovePointsMode(bool on)
{
    mMovePointsMode = on;
}


bool WaypointsLayer::eventFilter(QObject *obj, QEvent *ev)
{
#if 0
    if (ev->type()==QEvent::MouseButtonPress)
    {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(ev);
        if (mouseEvent->button()!=Qt::LeftButton) return (false);
#ifdef DEBUG_DRAGGING
        kDebug() << "press at" << mouseEvent->pos();
#endif

        mClickX = mouseEvent->pos().x();		// record click position
        mClickY = mouseEvent->pos().y();
        mClickTimer->start();				// start elapsed timer

        // See whether there is a track data point under the click.
        qreal lat,lon;
        bool onEarth = mapView()->geoCoordinates(mClickX, mClickY, lon, lat);
#ifdef DEBUG_DRAGGING
        kDebug() << "onearth" << onEarth << "lat" << lat << "lon" << lon;
#endif
        if (!onEarth) return (false);			// click not on Earth

        qreal lat1,lat2;
        qreal lon1,lon2;
        const int dragStart = qApp->startDragDistance();
        mapView()->geoCoordinates(mClickX-dragStart, mClickY-dragStart, lon1, lat1);
        mapView()->geoCoordinates(mClickX+dragStart, mClickY+dragStart, lon2, lat2);
        mLatMin = qMin(lat1, lat2);
        mLatMax = qMax(lat1, lat2);
        mLonMin = qMin(lon1, lon2);
        mLonMax = qMax(lon1, lon2);
#ifdef DEBUG_DRAGGING
        kDebug() << "tolerance box" << mLatMin << mLonMin << "-" << mLatMax << mLonMax;
#endif
        const TrackDataPoint *tdp = findClickedPoint(mapView()->filesModel()->rootFileItem());
        if (tdp!=NULL)					// a point was found
        {
            mClickedPoint = tdp;			// record for release event

            // If the click was over a point, then do not pass the event
            // on to Marble.  This stops it turning into a move-the-map drag.
            return (true);
        }
    }
    else if (ev->type()==QEvent::MouseButtonRelease)
    {
        if (mClickedPoint==NULL) return (false);	// no point clicked to start

        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(ev);
#ifdef DEBUG_DRAGGING
        kDebug() << "release at" << mouseEvent->pos();
#endif
        const TrackDataPoint *clickedPoint = mClickedPoint;
        mClickedPoint = NULL;

        if (mDraggingPoints!=NULL)
        {
#ifdef DEBUG_DRAGGING
            kDebug() << "end drag, lat/lon off" << mLatOff << mLonOff;
#endif
            emit draggedPoints(mLatOff, mLonOff);
            delete mDraggingPoints; mDraggingPoints = NULL;
            mapView()->update();
            return (true);
        }

        // See whether this release is in the same position and the same
        // time (allowing for a tolerance in both).  If so, accept the click
        // and action it for the point detected earlier.
        if (testClickTolerance(mouseEvent))
        {
#ifdef DEBUG_DRAGGING
            kDebug() << "valid click detected";
#endif
            mapView()->filesModel()->clickedPoint(clickedPoint, mouseEvent->modifiers());
            return (true);				// event consumed
        }
    }
    else if (ev->type()==QEvent::MouseMove)
    {
        if (!mMovePointsMode) return (false);

        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(ev);
        if (mouseEvent->buttons()!=Qt::LeftButton) return (false);
        if (mClickedPoint==NULL) return (false);	// no point clicked to start

        if (mDraggingPoints==NULL)			// no drag in progress yet
        {
            if (testClickTolerance(mouseEvent))		// check whether in same place
            {
#ifdef DEBUG_DRAGGING
                kDebug() << "start drag";
#endif
                mClickTimer->invalidate();

                const TrackDataPoint *tdp = findClickedPoint(mapView()->filesModel()->rootFileItem());
                if (tdp!=NULL && tdp->selectionId()!=mSelectionId)
                {
#ifdef DEBUG_DRAGGING
                    kDebug() << "but not over a selected point";
#endif
                    return (true);
                }

                mDraggingPoints = new QList<SelectionRun>;
                findSelectionInTree(mapView()->filesModel()->rootFileItem());
            }
            else return (false);			// outside click tolerance
        }

        qreal lat, lon;
        if (mapView()->geoCoordinates(mouseEvent->pos().x(),
                                      mouseEvent->pos().y(),
                                      lon, lat))
        {
            mLatOff = lat-mClickedPoint->latitude();
            mLonOff = lon-mClickedPoint->longitude();
#ifdef DEBUG_DRAGGING
            kDebug() << "lat/lon off" << mLatOff << mLonOff;
#endif
            mapView()->update();
        }

        return (true);					// event consumed
    }

#endif

    return (false);					// pass event on
}
