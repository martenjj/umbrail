
#include "trackslayer.h"

#include <qelapsedtimer.h>
#include <qapplication.h>
#include <qevent.h>

#include <kdebug.h>
#include <klocale.h>
#include <kcolorscheme.h>

#include <marble/GeoPainter.h>
#include <marble/GeoDataPlacemark.h>

#include "filesmodel.h"
#include "filesview.h"
#include "mainwindow.h"
#include "style.h"
#include "settings.h"
#include "mapview.h"


#undef DEBUG_PAINTING
#undef DEBUG_DRAGGING
#undef DEBUG_SELECTING


static const double RADIANS_TO_DEGREES = 360/(2*M_PI);	// multiplier

static const int POINTS_PER_BLOCK = 50;			// points drawn per polyline
static const int POINT_CIRCLE_SIZE = 10;		// size of point circle
static const int POINT_SELECTED_WIDTH = 3;		// line width for selected points

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

class SelectionRun
{
public:
    SelectionRun()				{}
    ~SelectionRun()				{}

    void addPoint(const GeoDataCoordinates &coord)	{ mThesePoints.append(coord); }
    void setPrevPoint(const GeoDataCoordinates &coord)	{ mPrevPoint = coord; }
    void setNextPoint(const GeoDataCoordinates &coord)	{ mNextPoint = coord; }

    void clear();
    bool isEmpty() const				{ return (mThesePoints.isEmpty()); }

    const GeoDataCoordinates *prevPoint() const		{ return (&mPrevPoint); }
    const GeoDataCoordinates *nextPoint() const		{ return (&mNextPoint); }
    const GeoDataLineString *thesePoints() const	{ return (&mThesePoints); }

private:
    GeoDataCoordinates mPrevPoint;
    GeoDataLineString mThesePoints;
    GeoDataCoordinates mNextPoint;
};


void SelectionRun::clear()
{
    mPrevPoint = mNextPoint = GeoDataCoordinates();
    mThesePoints.clear();
}

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////






TracksLayer::TracksLayer(QWidget *pnt)
    : QObject(pnt)
{
    kDebug();

    mMapView = qobject_cast<MapView *>(pnt);

    mClickedPoint = NULL;
    mDraggingPoints = NULL;
    mClickTimer = new QElapsedTimer;
    mMovePointsMode = false;

    mapView()->installEventFilter(this);
}


TracksLayer::~TracksLayer()
{
    delete mDraggingPoints;
    kDebug() << "done";
}


QStringList TracksLayer::renderPosition() const
{
    return (QStringList("HOVERS_ABOVE_SURFACE"));
}


qreal TracksLayer::zValue() const
{
    return (1.0);
}


static inline GeoDataCoordinates applyOffset(const GeoDataCoordinates &coords, qreal lonOff, qreal latOff)
{
    return (GeoDataCoordinates(coords.longitude(GeoDataCoordinates::Degree)+lonOff,
                               coords.latitude(GeoDataCoordinates::Degree)+latOff,
                               0, GeoDataCoordinates::Degree));
}


bool TracksLayer::render(GeoPainter *painter, ViewportParams *viewport,
                         const QString &renderPos, GeoSceneLayer *layer)
{
    const FilesModel *filesModel = mapView()->filesModel();
    if (filesModel==NULL) return (false);		// no data to use!

    const FilesView *filesView = mapView()->filesView();
    mSelectionId = (filesView!=NULL ? filesView->selectionId() : 0);

    // Paint the data in two passes.  The first does all non-selected tracks,
    // the second selected ones.  This is so that selected tracks show up
    // on top of all non-selected ones.  In the absence of any selection,
    // all tracks will be painted in file and then time order (i.e. later
    // track segments on top of earlier ones).
    paintDataTree(filesModel->rootFileItem(), painter, false, false);
    paintDataTree(filesModel->rootFileItem(), painter, true, false);

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

    return (true);
}


void TracksLayer::paintDataTree(const TrackDataItem *tdi, GeoPainter *painter, 
                                bool doSelected, bool parentSelected)
{
    if (tdi==NULL) return;				// nothing to paint
    int cnt = tdi->childCount();
    if (cnt==0) return;					// quick escape if no children

    bool isSelected = parentSelected || (tdi->selectionId()==mSelectionId);
#ifdef DEBUG_PAINTING
    kDebug() << tdi->name() << "isselected" << isSelected << "doselected" << doSelected;
#endif

    // What is actually drawn on the map is a polyline representing a sequence
    // of points (with their parent container normally a track segment, but this
    // is not mandated).  So it is not necessary to recurse all the way down the
    // data tree to the individual points, we can stop at the level above if the
    // first child item (assumed to be representative of the others) is a point.

    const TrackDataItem *firstChild = tdi->childAt(0);
    if (dynamic_cast<const TrackDataPoint *>(firstChild)!=NULL)
    {							// is first child a point?
        if (!(isSelected ^ doSelected))			// check selected or not
        {
#ifdef DEBUG_PAINTING
            kDebug() << "points for" << tdi->name() << "count" << cnt;
#endif
            // Scan along the segment, assembling the coordinates into a list,
            // and draw them as a polyline.
            //
            // Some polyline segments appear to be not drawn if there are too many
            // of them at high magnifications - is this a limitation in Marble?
            // Split the drawing up if there are too many - this may make clipping
            // more efficient too.

            QColor col = MapView::resolveLineColour(tdi);
            painter->setBrush(Qt::NoBrush);
            painter->setPen(QPen(col, 4));

            GeoDataLineString lines;			// generated coordinate list
            int start = 0;				// start point in list
            while (start<(cnt-1))			// while more blocks to do
            {
#ifdef DEBUG_PAINTING
                kDebug() << "starting at" << start;
#endif
                lines.clear();				// empty the list
                int sofar = 0;				// points so far this block
                for (int i = start; i<cnt; ++i)		// up to end of list
                {
                    const TrackDataPoint *tdp = static_cast<const TrackDataPoint *>(tdi->childAt(i));
                    GeoDataCoordinates coord(tdp->longitude(), tdp->latitude(),
                                             0, GeoDataCoordinates::Degree);
                    lines.append(coord);		// add point to list
                    ++sofar;				// count how many this block
                    if (sofar>=POINTS_PER_BLOCK) break;	// too many, draw and start again
                }

#ifdef DEBUG_PAINTING
                kDebug() << "block of" << sofar << "points";
#endif
                if (lines.size()<2) break;		// nothing more to draw
                painter->drawPolyline(lines);		// draw track in its colour

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
                    const TrackDataPoint *tdp = static_cast<const TrackDataPoint *>(tdi->childAt(i));
                    GeoDataCoordinates coord(tdp->longitude(), tdp->latitude(),
                                             0, GeoDataCoordinates::Degree);
                    painter->drawEllipse(coord, POINT_CIRCLE_SIZE, POINT_CIRCLE_SIZE);
                }

                // Finally, draw the selected points along the line, if there are any.
                // Do this last so that the selection markers always show up on top.

                if (Settings::selectedUseSystemColours())
                {					// use system selection colours
                    KColorScheme sch(QPalette::Active, KColorScheme::Selection);
                    painter->setPen(QPen(sch.background().color(), POINT_SELECTED_WIDTH));
                    painter->setBrush(sch.foreground());
                }
                else					// our own custom colours
                {
                    painter->setPen(QPen(Settings::selectedMarkOuter(), POINT_SELECTED_WIDTH));
                    painter->setBrush(Settings::selectedMarkInner());
                }

                for (int i = 0; i<cnt; ++i)
                {
                    const TrackDataPoint *tdp = static_cast<const TrackDataPoint *>(tdi->childAt(i));
                    if (tdp->selectionId()==mSelectionId)
                    {
                        GeoDataCoordinates coord(tdp->longitude(), tdp->latitude(),
                                                 0, GeoDataCoordinates::Degree);
                        painter->drawEllipse(coord, POINT_CIRCLE_SIZE, POINT_CIRCLE_SIZE);
                    }
                }
            }
        }
    }
    else						// first child not a point,
    {							// so we are higher container
        for (int i = 0; i<cnt; ++i)			// just recurse to paint children
        {
            const TrackDataItem *childItem = tdi->childAt(i);
            paintDataTree(childItem, painter, doSelected, isSelected);
        }
    }
}


const TrackDataPoint *TracksLayer::findClickedPoint(const TrackDataItem *tdi)
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


bool TracksLayer::testClickTolerance(const QMouseEvent *mev) const
{
    const int dragStart = qApp->startDragDistance();
    return (qAbs(mev->pos().x()-mClickX)<dragStart &&
            qAbs(mev->pos().y()-mClickY)<dragStart &&
            mClickTimer->elapsed()<qApp->startDragTime());
}


void TracksLayer::findSelectionInTree(const TrackDataItem *tdi)
{
    if (tdi==NULL) return;				// nothing to paint
    int cnt = tdi->childCount();
    if (cnt==0) return;					// quick escape if no children

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
}


void TracksLayer::setMovePointsMode(bool on)
{
    mMovePointsMode = on;
}


bool TracksLayer::eventFilter(QObject *obj, QEvent *ev)
{
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

    return (false);					// pass event on
}
