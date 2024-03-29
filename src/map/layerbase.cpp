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

#include "layerbase.h"

#include <qelapsedtimer.h>
#include <qapplication.h>
#include <qevent.h>
#include <qtimer.h>
#include <qdebug.h>

#include <klocalizedstring.h>
#include <kcolorscheme.h>

#include <marble/GeoPainter.h>
#include <marble/GeoDataPlacemark.h>

#include "filesmodel.h"
#include "filesview.h"
#include "mapcontroller.h"
#include "mapview.h"
#include "settings.h"

//////////////////////////////////////////////////////////////////////////
//									//
//  Debugging switches							//
//									//
//////////////////////////////////////////////////////////////////////////

#undef DEBUG_PAINTING
#undef DEBUG_DRAGGING
#undef DEBUG_SELECTING

//////////////////////////////////////////////////////////////////////////
//									//
// Painting parameters							//
//									//
//////////////////////////////////////////////////////////////////////////

static const int POINT_SELECTED_WIDTH = 3;		// line width for selected points

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

void SelectionRun::clear()
{
    mPrevPoint = mNextPoint = GeoDataCoordinates();
    mThesePoints.clear();
}

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

LayerBase::LayerBase(QWidget *pnt)
    : QObject(pnt),
      ApplicationDataInterface(pnt)
{
    qDebug();

    mVisible = true;
    mClickedPoint = nullptr;
    mDraggingPoints = nullptr;
    mClickTimer = new QElapsedTimer;
    mMovePointsMode = false;
    mViewport = nullptr;

    QTimer::singleShot(0, this, &LayerBase::slotInstallEventFilter);
}


LayerBase::~LayerBase()
{
    delete mDraggingPoints;
    qDebug() << "done";
}


void LayerBase::slotInstallEventFilter()
{
    mapController()->view()->installEventFilter(this);
}


QStringList LayerBase::renderPosition() const
{
    return (QStringList("HOVERS_ABOVE_SURFACE"));
}

//////////////////////////////////////////////////////////////////////////
//									//
//  className -- This class is the abstract base class of drawing	//
//  layers.  For qDebug() messages to be useful, they need to identify	//
//  the real class name.						//
//									//
//////////////////////////////////////////////////////////////////////////

static QByteArray className(const QObject *obj)
{
    return (QString("[%1]").arg(obj->metaObject()->className()).toLatin1());
}



GeoDataCoordinates LayerBase::applyOffset(const GeoDataCoordinates &coords) const
{
    return (GeoDataCoordinates(coords.longitude(GeoDataCoordinates::Degree)+mLonOff,
                               coords.latitude(GeoDataCoordinates::Degree)+mLatOff,
                               0, GeoDataCoordinates::Degree));
}



bool LayerBase::render(GeoPainter *painter, ViewportParams *viewport,
                       const QString &renderPos, GeoSceneLayer *layer)
{
    if (!isVisible()) return (true);			// no painting if not visible
    mViewport = viewport;				// save for access by layers

    const FilesModel *filesModel = qobject_cast<FilesModel *>(filesView()->model());
    if (filesModel==nullptr) return (false);		// no data to use!

    mSelectionId = filesView()->selectionId();

    // Paint the data in two passes.  The first does all non-selected items,
    // the second selected ones.  This is so that selected items show up
    // on top of all non-selected ones.  In the absence of any selection,
    // everything will be painted in file and then time order (i.e. later
    // items on top of earlier ones).
    paintDataTree(filesModel->rootFileItem(), painter, false, false);
    paintDataTree(filesModel->rootFileItem(), painter, true, false);

    if (mDraggingPoints!=nullptr)
    {
#ifdef DEBUG_DRAGGING
        qDebug() << className(this).constData() << "paint for drag";
#endif
        for (const SelectionRun &run : qAsConst(*mDraggingPoints)) this->doPaintDrag(&run, painter);
    }

    return (true);
}



void LayerBase::paintDataTree(const TrackDataItem *item, GeoPainter *painter, 
                              bool doSelected, bool parentSelected)
{
    if (item==nullptr) return;				// nothing to paint

    bool isSelected = parentSelected || (item->selectionId()==mSelectionId);
#ifdef DEBUG_PAINTING
    qDebug() << className(this).constData() << item->name() << "isselected" << isSelected << "doselected" << doSelected;
#endif

    // In order to be able to draw tracks, which are drawn on the map as a
    // polyline representing a sequence of points, we do not want to recurse
    // all the way down the data tree to the individual points but to stop
    // at their containing segment.  For consistency, waypoints (which are
    // always contained within a folder) are treated the same way.
    if (this->isDirectContainer(item))			// paint this container?
    {
        if (!(isSelected ^ doSelected))			// check selected or not
        {
            doPaintItem(item, painter, isSelected);
        }
    }

    const int cnt = item->childCount();
    for (int i = 0; i<cnt; ++i)				// recurse to paint children
    {
        const TrackDataItem *childItem = item->childAt(i);
        if (childItem->childCount()==0) continue;	// no point if no children
        if (this->isIndirectContainer(childItem))	// can contain applicable items?
        {
            paintDataTree(childItem, painter, doSelected, isSelected);
        }
    }
}



const TrackDataAbstractPoint *LayerBase::findClickedPoint(const TrackDataItem *item)
{
    if (item==nullptr) return (nullptr);		// nothing to do

    if (this->isApplicableItem(item))			// consider this item itself?
    {
        const TrackDataAbstractPoint *tdp = dynamic_cast<const TrackDataAbstractPoint *>(item);
        if (tdp!=nullptr)				// applicable type of point
        {
            const double lat = tdp->latitude();
            const double lon = tdp->longitude();
            if (lat>=mLatMin && lat<=mLatMax &&
                lon>=mLonMin && lon<=mLonMax)
            {						// check within tolerance
#ifdef DEBUG_SELECTING
                qDebug() << className(this).constData() << "found point" << tdp->name();
#endif
                return (tdp);				// clicked point found
            }
        }

        return (nullptr);				// applicable but not clicked
    }

    if (this->isIndirectContainer(item))		// can contain applicable items?
    {
        for (int i = 0; i<item->childCount(); ++i)	// recurse to search children
        {
            const TrackDataItem *childItem = item->childAt(i);
            const TrackDataAbstractPoint *childPoint = findClickedPoint(childItem);
            if (childPoint!=nullptr) return (childPoint);
        }						// this point found
     }

    return (nullptr);					// nothing found
}



bool LayerBase::testClickTolerance(const QMouseEvent *mev) const
{
    const int dragStart = qApp->startDragDistance();
    return (qAbs(mev->pos().x()-mClickX)<dragStart &&
            qAbs(mev->pos().y()-mClickY)<dragStart &&
            mClickTimer->elapsed()<qApp->startDragTime());
}



void LayerBase::findSelectionInTree(const TrackDataItem *item)
{
    if (item==nullptr) return;				// nothing to search
    int cnt = item->childCount();

    if (this->isDirectContainer(item))			// look at contained items?
    {
#ifdef DEBUG_SELECTING
        qDebug() << className(this).constData() << "consider" << item->name();
#endif
        // If there are any selected points in this container, assemble
        // them into a SelectionRun and add it to the dragging list.

        SelectionRun run;
        for (int i = 0; i<cnt; ++i)
        {
            const TrackDataAbstractPoint *tdp = dynamic_cast<const TrackDataAbstractPoint *>(item->childAt(i));
            if (tdp==nullptr) continue;

#ifdef DEBUG_SELECTING
            qDebug() << "  " << i << tdp->name() << "selected?" << (tdp->selectionId()==mSelectionId);
#endif
            if (tdp->selectionId()==mSelectionId)	// this point is selected
            {
                if (run.isEmpty())			// start of a new run
                {
#ifdef DEBUG_SELECTING
                    qDebug() << "    starting new run";
#endif
                    if (i>0)				// not first point in container
                    {
                        const TrackDataAbstractPoint *prev = dynamic_cast<const TrackDataAbstractPoint *>(item->childAt(i-1));
                        if (prev!=nullptr)
                        {
#ifdef DEBUG_SELECTING
                            qDebug() << "    setting prev point" << prev->name();
#endif
                            run.setPrevPoint(GeoDataCoordinates(prev->longitude(), prev->latitude(),
                                                                0, GeoDataCoordinates::Degree));
                        }
                    }
                }

                GeoDataCoordinates coord(tdp->longitude(), tdp->latitude(),
                                         0, GeoDataCoordinates::Degree);
#ifdef DEBUG_SELECTING
                qDebug() << "    add point" << tdp->name();
#endif
                run.addPoint(coord);
            }
            else					// this point not selected
            {
                if (!run.isEmpty())
                {
#ifdef DEBUG_SELECTING
                    qDebug() << "    setting next point" << tdp->name();
#endif
                    run.setNextPoint(GeoDataCoordinates(tdp->longitude(), tdp->latitude(),
                                                        0, GeoDataCoordinates::Degree));
                    mDraggingPoints->append(run);
#ifdef DEBUG_SELECTING
                    qDebug() << "    add run";
#endif
                    run.clear();			// clear for next time
                }
            }
        }

        if (!run.isEmpty())
        {
#ifdef DEBUG_SELECTING
            qDebug() << "add final run";
#endif
            mDraggingPoints->append(run);
        }

#ifdef DEBUG_SELECTING
        qDebug() << "done" << "with" << mDraggingPoints->count() << "runs";
#endif
    }

    if (this->isIndirectContainer(item))		// can contain applicable items?
    {
        for (int i = 0; i<cnt; ++i)			// just recurse to search children
        {
            const TrackDataItem *childItem = item->childAt(i);
            if (childItem->childCount()==0) continue;	// no point if no children
            this->findSelectionInTree(childItem);
        }
    }
}



void LayerBase::setMovePointsMode(bool on)
{
    mMovePointsMode = on;
}



bool LayerBase::eventFilter(QObject *obj, QEvent *ev)
{
    if (!isVisible()) return (false);			// no interaction if not visible

    MapView *mapView = mapController()->view();

    FilesModel *filesModel = qobject_cast<FilesModel *>(filesView()->model());
    if (filesModel==nullptr) return (false);		// no data to use!

    if (ev->type()==QEvent::MouseButtonPress)
    {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(ev);
        if (mouseEvent->button()!=Qt::LeftButton) return (false);
#ifdef DEBUG_DRAGGING
        qDebug() << className(this).constData() << "press at" << mouseEvent->pos();
#endif
        mClickX = mouseEvent->pos().x();		// record click position
        mClickY = mouseEvent->pos().y();
        mClickTimer->start();				// start elapsed timer

        // See whether there is a track data point under the click.
        qreal lat,lon;
        bool onEarth = mapView->geoCoordinates(mClickX, mClickY, lon, lat);
#ifdef DEBUG_DRAGGING
        qDebug() << "  onearth" << onEarth << "lat" << lat << "lon" << lon;
#endif
        if (!onEarth) return (false);			// click not on Earth

        qreal lat1,lat2;
        qreal lon1,lon2;
        const int dragStart = qApp->startDragDistance();
        mapView->geoCoordinates(mClickX-dragStart, mClickY-dragStart, lon1, lat1);
        mapView->geoCoordinates(mClickX+dragStart, mClickY+dragStart, lon2, lat2);
        mLatMin = qMin(lat1, lat2);
        mLatMax = qMax(lat1, lat2);
        mLonMin = qMin(lon1, lon2);
        mLonMax = qMax(lon1, lon2);
#ifdef DEBUG_DRAGGING
        qDebug() << "  tolerance box" << mLatMin << mLonMin << "-" << mLatMax << mLonMax;
#endif
        const TrackDataAbstractPoint *tdp = findClickedPoint(filesModel->rootFileItem());
        if (tdp!=nullptr)				// a point was found
        {
            mClickedPoint = tdp;			// record for release event

            // If the click was over a point, then do not pass the event
            // on to other layers or to Marble.  This stops it turning
            // into a move-the-map drag.
            return (true);
        }
    }
    else if (ev->type()==QEvent::MouseButtonRelease)
    {
        if (mClickedPoint==nullptr) return (false);	// no point clicked to start

        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(ev);
#ifdef DEBUG_DRAGGING
        qDebug() << className(this).constData() << "release at" << mouseEvent->pos();
#endif
        const TrackDataAbstractPoint *clickedPoint = mClickedPoint;
        mClickedPoint = nullptr;

        if (mDraggingPoints!=nullptr)
        {
#ifdef DEBUG_DRAGGING
            qDebug() << "  end drag, lat/lon off" << mLatOff << mLonOff;
#endif
            emit draggedPoints(mLatOff, mLonOff);
            delete mDraggingPoints; mDraggingPoints = nullptr;
            mapView->update();
            return (true);
        }

        // See whether this release is in the same position and the same
        // time (allowing for a tolerance in both).  If so, accept the click
        // and action it for the point detected earlier.
        if (testClickTolerance(mouseEvent))
        {
#ifdef DEBUG_DRAGGING
            qDebug() << "  valid click detected";
#endif
            filesModel->clickedPoint(clickedPoint, mouseEvent->modifiers());
            return (true);				// event consumed
        }
    }
    else if (ev->type()==QEvent::MouseMove)
    {
        if (!mMovePointsMode) return (false);

        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(ev);
        if (mouseEvent->buttons()!=Qt::LeftButton) return (false);
        if (mClickedPoint==nullptr) return (false);	// no point clicked to start

        if (mDraggingPoints==nullptr)			// no drag in progress yet
        {
            if (testClickTolerance(mouseEvent))		// check whether in same place
            {
#ifdef DEBUG_DRAGGING
                qDebug() << className(this).constData() << "start drag";
#endif
                mClickTimer->invalidate();

                const TrackDataAbstractPoint *tdp = findClickedPoint(filesModel->rootFileItem());
                if (tdp!=nullptr && tdp->selectionId()!=mSelectionId)
                {
#ifdef DEBUG_DRAGGING
                    qDebug() << "  but not over a selected point";
#endif
                    return (true);
                }

                mDraggingPoints = new QList<SelectionRun>;
                this->findSelectionInTree(filesModel->rootFileItem());
            }
            else return (false);			// outside click tolerance
        }

        qreal lat, lon;
        if (mapView->geoCoordinates(mouseEvent->pos().x(), mouseEvent->pos().y(), lon, lat))
        {
            mLatOff = lat-mClickedPoint->latitude();
            mLonOff = lon-mClickedPoint->longitude();
#ifdef DEBUG_DRAGGING
            qDebug() << "  lat/lon off" << mLatOff << mLonOff;
#endif
            mapView->update();
        }

        return (true);					// event consumed
    }

    return (false);					// pass event on
}



void LayerBase::setSelectionColours(QPainter *painter, bool setBrush) const
{
    if (Settings::selectedUseSystemColours())		// use system selection colours
    {
        KColorScheme sch(QPalette::Active, KColorScheme::Selection);
        painter->setPen(QPen(sch.background().color(), POINT_SELECTED_WIDTH));
        if (setBrush) painter->setBrush(sch.foreground());
    }
    else						// our own custom colours
    {
        painter->setPen(QPen(Settings::selectedMarkOuter(), POINT_SELECTED_WIDTH));
        if (setBrush) painter->setBrush(Settings::selectedMarkInner());
    }
}


void LayerBase::cancelDrag()
{
#ifdef DEBUG_DRAGGING
        qDebug() << className(this).constData();
#endif
        if (mDraggingPoints!=nullptr)
        {
            delete mDraggingPoints; mDraggingPoints = nullptr;
            mapController()->view()->update();
        }

        mClickedPoint = nullptr;
}
