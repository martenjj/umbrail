
#include "trackpropertiesdetailpages.h"

#include <qformlayout.h>
#include <qdebug.h>

#include <klocalizedstring.h>

#include "trackdata.h"
#include "trackdatalabel.h"
#include "variableunitdisplay.h"
#include "dataindexer.h"

//////////////////////////////////////////////////////////////////////////
//									//
//  Utility functions							//
//									//
//////////////////////////////////////////////////////////////////////////

double sumTotalTravelDistance2(const TrackDataItem *item, bool tracksOnly)
{
    const int num = item->childCount();
    if (num==0) return (0.0);				// no children

    // Ignore folders.  Waypoints are not considred to be in either time or
    // file order, so the concept of travel distance for them is meaningless.
    const TrackDataFolder *tdf = dynamic_cast<const TrackDataFolder *>(item);
    if (tdf!=nullptr) return (0.0);			// don't want folders here

    // See whether the item is a ordered container (segment or route).
    // If it is a route, but a higher level has decided that routes are
    // not wanted, then finish here.
    const TrackDataSegment *tds = dynamic_cast<const TrackDataSegment *>(item);
    const TrackDataRoute *tdr = dynamic_cast<const TrackDataRoute *>(item);
    if (tdr!=nullptr && tracksOnly) return (0.0);	// don't want routes here

    double dist = 0.0;					// running total

    if (tds!=nullptr || tdr!=nullptr)			// this is a point container
    {
        // For any ordered container (segment or route), sum all of its
        // contained points.
        const TrackDataAbstractPoint *prev = nullptr;
        for (int i = 0; i<num; ++i)
        {
            const TrackDataAbstractPoint *tdp = dynamic_cast<TrackDataAbstractPoint *>(item->childAt(i));
            Q_ASSERT(tdp!=nullptr);
            if (prev!=nullptr) dist += prev->distanceTo(tdp);
            prev = tdp;
        }
    }
    else						// any other container
    {
        // For any other sort ofcontainer (file or track), recurse into its children.
        for (int i = 0; i<num; ++i)
        {
            dist += sumTotalTravelDistance2(item->childAt(i), tracksOnly);
        }
    }

    return (dist);
}


double sumTotalTravelDistance(const QList<TrackDataItem *> *items)
{
    // Sum the total travel distance over all selected items, recursively.

    // At the top level, look to see what sort of item(s) is/are being
    // considered.  If a file, only look recursively into tracks and segments.
    // If a route, look at its route points.
    //
    // The intention of this is to give sensible results for the "total
    // "travel distance" of a file.  If it contains tracks then it is
    // probably a track recording file and the figure of interest is that
    // for the recordings.  If it does not contain any tracks then it is
    // probably a route planning file and the figure of interest is
    // that for the routes.

    if (items==nullptr) return (0.0);
    int num = items->count();
    if (num==0) return (0.0);

    const TrackDataItem *item1 = items->first();	// first item
    double dist = 0.0;					// running total

    const TrackDataAbstractPoint *tdp1 = dynamic_cast<const TrackDataAbstractPoint *>(item1);

    // If two points are selected in an ordered container (segment or
    // route), look at all of the points between them.  This works
    // for waypoints in folders also, but the result is meaningless.

    if (num==2 && tdp1!=nullptr)
    {
        const TrackDataItem *pnt = tdp1->parent();
        Q_ASSERT(pnt!=nullptr);
        const int idx1 = pnt->childIndex(tdp1);

        const TrackDataAbstractPoint *tdp2 = dynamic_cast<TrackDataAbstractPoint *>(items->last());
        Q_ASSERT(tdp2!=nullptr);

        const TrackDataAbstractPoint *prev = tdp1;
        const int idx2 = pnt->childIndex(tdp2);
        for (int i = idx1+1; i<=idx2; ++i)
        {
            const TrackDataAbstractPoint *tdp = dynamic_cast<TrackDataAbstractPoint *>(pnt->childAt(i));
            Q_ASSERT(tdp!=nullptr);
            dist += prev->distanceTo(tdp);
            prev = tdp;
        }

        return (dist);
    }

    // If a number of points in an ordered container are selected,
    // sum the distance between each one and the next.  If the selection
    // is not contiguous then the result is not really meaningful.

    if (num>2 && tdp1!=nullptr)
    {
        const TrackDataAbstractPoint *prev = nullptr;
        foreach (const TrackDataItem *item, *items)
        {
            const TrackDataAbstractPoint *tdp = dynamic_cast<const TrackDataAbstractPoint *>(item);
            Q_ASSERT(tdp!=nullptr);
            if (prev!=nullptr) dist += prev->distanceTo(tdp);
            prev = tdp;
        }

        return (dist);
    }

    // Here at the top recursion level, check whether this is a file.
    // In which case, only tracks are to be considered.  If the file
    // contains no tracks but may still contain routes, then the routes
    // will be considered.

    bool tracksOnly = false;				// assume so to start
    if (dynamic_cast<const TrackDataFile *>(item1)!=nullptr)
    {							// file at top level
        foreach (const TrackDataItem *item, *items)
        {
            for (int i = 0; i<item->childCount(); ++i)
            {
                const TrackDataItem *childItem = item->childAt(i);
                const TrackDataTrack *tdt = dynamic_cast<const TrackDataTrack *>(childItem);
                if (tdt!=nullptr)			// have found a track
                {
                    tracksOnly = true;
                    break;
                }
            }
            if (tracksOnly) break;			// have found a track,
        }						// no need to look further
    }

    // Sum segments and/or routes, and recurse into other containers.

    foreach (const TrackDataItem *item, *items)
    {
        dist += sumTotalTravelDistance2(item, tracksOnly);
    }

    return (dist);
}


static void getTwoPoints(const QList<TrackDataItem *> *items,
                         const TrackDataItem *parentContainer,
                         TrackDataAbstractPoint **ppt1,
                         TrackDataAbstractPoint **ppt2)
{
    TrackDataItem *item1 = items->first();
    TrackDataItem *item2 = items->last();

    Q_ASSERT(parentContainer!=nullptr);
    int idx1 = parentContainer->childIndex(item1);
    int idx2 = parentContainer->childIndex(item2);
    if (idx1>idx2) qSwap(idx1, idx2);			// order by index = time

    TrackDataAbstractPoint *pnt1 = dynamic_cast<TrackDataAbstractPoint *>(parentContainer->childAt(idx1));
    TrackDataAbstractPoint *pnt2 = dynamic_cast<TrackDataAbstractPoint *>(parentContainer->childAt(idx2));
    Q_ASSERT(pnt1!=nullptr && pnt2!=nullptr);

    *ppt1 = pnt1;
    *ppt2 = pnt2;
}

//////////////////////////////////////////////////////////////////////////
//									//
//  TrackItemDetailPage							//
//									//
//////////////////////////////////////////////////////////////////////////

TrackItemDetailPage::TrackItemDetailPage(const QList<TrackDataItem *> *items, QWidget *pnt)
    : TrackPropertiesPage(items, pnt)
{
    qDebug();
    setObjectName("TrackItemDetailPage");

    addSeparatorField();
}


void TrackItemDetailPage::addDisplayFields(const QList<TrackDataItem *> *items,
                                           DisplayItems disp)
{
    const int num = items->count();			// number of items
    qDebug() << "display" << disp << "for" << num << "items";
    Q_ASSERT(num>0);					// must have something here
							// parent container, if any
    const TrackDataItem *parentContainer = items->first()->parent();

    // See if this selection is contiguous within its parent.
    // A single selected item or an item with no parent (i.e. the
    // top level file or root) is assumed to be contiguous.

    bool contiguousSelection = true;			// assume so at start
    if (num>1 && parentContainer!=nullptr)		// more than one item in container
    {							// its index of first item
        const int firstIdx = parentContainer->childIndex(items->first());
        for (int i = 1; i<num; ++i)			// look at following items
        {
            if (parentContainer->childAt(firstIdx+i)!=items->at(i))
            {						// mismatch children/selection
                contiguousSelection = false;
                break;
            }
        }
    }

    const TrackDataAbstractPoint *tdp = dynamic_cast<const TrackDataAbstractPoint *>(items->first());

    TrackDataLabel *l;
    VariableUnitDisplay *vl;

    double travelDistance = sumTotalTravelDistance(items);
    TimeRange tsp = TrackData::unifyTimeSpans(items);
    const bool blankTimes = (isEmpty() || !tsp.isValid());

    // Position or bounding area
    if (disp & DisplayPosition)
    {
        if (num==1 && tdp!=nullptr)			// a single point selected
        {
            l = new TrackDataLabel(tdp->formattedPosition(), this);
            mFormLayout->addRow(i18nc("@label:textbox", "Position:"), l);
            mPositionLabel = l;
        }
        else						// multiple or container selected
        {
            const BoundingArea bb = TrackData::unifyBoundingAreas(items);
            l = new TrackDataLabel(bb.north(), bb.west(), true, this);
            mFormLayout->addRow(i18nc("@label:textbox", "Bounding area:"), l);
            disableIfEmpty(l);
            l = new TrackDataLabel(bb.south(), bb.east(), true, this);
            mFormLayout->addRow("", l);
            disableIfEmpty(l);
        }
    }

    // Time or time span
    if (disp & DisplayTime)
    {
        if (num==1 && tdp!=nullptr)			// a single point selected
        {
            l = new TrackDataLabel(tdp->time(), this);
            mFormLayout->addRow(i18nc("@label:textbox", "Time:"), l);
        }
        else						// multiple or container selected
        {
            l = new TrackDataLabel(tsp.start(), this);
            mFormLayout->addRow(i18nc("@label:textbox", "Time start:"), l);
            disableIfEmpty(l);

            l = new TrackDataLabel(tsp.finish(), this);
            mFormLayout->addRow(i18nc("@label:textbox", "Time end:"), l);
            disableIfEmpty(l);

            unsigned tt = tsp.timeSpan();
            l = new TrackDataLabel(TrackData::formattedDuration(tt, blankTimes), this);
            mFormLayout->addRow(i18nc("@label:textbox", "Time span:"), l);
            disableIfEmpty(l);
        }
    }

    // Travel time
    if (disp & DisplayTravelTime)
    {
        unsigned tt = TrackData::sumTotalTravelTime(items);
        l = new TrackDataLabel(TrackData::formattedDuration(tt, blankTimes), this);
        mFormLayout->addRow(i18nc("@label:textbox", "Travel time:"), l);
        disableIfEmpty(l);
    }

    // Elevation or elevation difference
    if (disp & DisplayElevation)
    {
        if (disp & (DisplayPosition|DisplayTime|DisplayTravelTime))
        {						// some fields added already
            addSeparatorField();
        }

        if (tdp!=nullptr && num==1)			// a single point selected
        {						// show the point elevation
            const double ele = tdp->elevation();
            if (!ISNAN(ele))
            {
                vl = new VariableUnitDisplay(VariableUnitCombo::Elevation, this);
                vl->setSaveId("elevation");
                vl->setValue(ele);
                mFormLayout->addRow(i18nc("@label:textbox", "Elevation:"), vl);
            }
        }
        else						// multiple or container selected
        {
            if (tdp!=nullptr && num==2)			// exactly two points selected
            {						// (nocontiguous doesn't matter)
                TrackDataAbstractPoint *pnt1;
                TrackDataAbstractPoint *pnt2;
                getTwoPoints(items, parentContainer, &pnt1, &pnt2);

                double ele1 = pnt1->elevation();
                double ele2 = pnt2->elevation();
                if (!ISNAN(ele1) && !ISNAN(ele2))
                {
                    vl = new VariableUnitDisplay(VariableUnitCombo::Elevation, this);
                    vl->setSaveId("elediff");
                    vl->setValue(ele2-ele1);
                    mFormLayout->addRow(i18nc("@label:textbox", "Elevation difference:"), vl);
                }
            }
        }
    }

    // Total travel distance.  Not for a contiguous selection of only
    // two points, that case will show as "Straight line distance" below.
    if (disp & DisplayTravelDistance)
    {
        bool showIt = (tdp==nullptr);				// show for a container
        if (!showIt)						// not for a container
        {
            if (num==2 && !contiguousSelection) showIt = true;	// show for 2 separated points
            if (num>2 && contiguousSelection) showIt = true;	// or for many contiguous
        }

        if (showIt)
        {
            vl = new VariableUnitDisplay(VariableUnitCombo::Distance, this);
            vl->setSaveId("totaltraveldistance");
            vl->setValue(travelDistance);
            mFormLayout->addRow(i18nc("@label:textbox", "Travel distance:"), vl);
            disableIfEmpty(vl);
        }
    }

    // Average speed.  For any selection of two points,
    // or a contiguous selection.
    if (disp & DisplayAverageSpeed)
    {
        if (num==2 || contiguousSelection)
        {						// if not calculated already
            QWidget *w;
            if (!ISNAN(travelDistance) && tsp.isValid())
            {
                const unsigned tt = tsp.timeSpan();
                // First calculate the average speed in metres/second.  It doesn't matter
                // which unit is used here and below, as long as they are both the same!
                double averageSpeed = Units::internalToLength(travelDistance, Units::LengthMetres)/tt;
                // Then convert the speed back into internal units.  It will then be
                // converted back into the selected user unit for display.
                averageSpeed = Units::speedToInternal(averageSpeed, Units::SpeedMetresSecond);

                vl = new VariableUnitDisplay(VariableUnitCombo::Speed, this);
                vl->setSaveId("averagespeed");
                vl->setValue(averageSpeed);
                w = vl;
            }
            else w = new QLabel(i18nc("Not available", "N/A"), this);
            mFormLayout->addRow(i18nc("@label:textbox", "Average speed:"), w);
            disableIfEmpty(w);
        }
    }

    // Total route length
    if (disp & DisplayRouteLength)
    {
        double routeLength = sumTotalTravelDistance(items);

        vl = new VariableUnitDisplay(VariableUnitCombo::Distance, this);
        vl->setSaveId("routelength");
        vl->setValue(routeLength);
        mFormLayout->addRow(i18nc("@label:textbox", "Length:"), vl);
        disableIfEmpty(vl);
    }

    // Relative bearing, for exactly two points
    if (disp & DisplayRelativeBearing)
    {
        if (tdp!=nullptr && num==2)			// exactly two points selected
        {						// (nocontiguous doesn't matter)
            TrackDataAbstractPoint *pnt1;
            TrackDataAbstractPoint *pnt2;
            getTwoPoints(items, parentContainer, &pnt1, &pnt2);

            vl = new VariableUnitDisplay(VariableUnitCombo::Bearing, this);
            vl->setSaveId("bearing");
            vl->setValue(pnt1->bearingTo(pnt2));
            mFormLayout->addRow(i18nc("@label:textbox", "Relative bearing:"), vl);
        }
    }

    // Straight line distance, for exactly two points
    if (disp & DisplayStraightLine)
    {
        if (tdp!=nullptr && num==2)			// exactly two points selected
        {						// (nocontiguous doesn't matter)
            TrackDataAbstractPoint *pnt1;
            TrackDataAbstractPoint *pnt2;
            getTwoPoints(items, parentContainer, &pnt1, &pnt2);

            vl = new VariableUnitDisplay(VariableUnitCombo::Distance, this);
            vl->setSaveId("crowflies");
            vl->setValue(pnt1->distanceTo(pnt2, true));
            mFormLayout->addRow(i18nc("@label:textbox", "Straight line distance:"), vl);
        }
    }
}


void TrackItemDetailPage::addChildCountField(const QList<TrackDataItem *> *items, const QString &labelText)
{
    int num = TrackData::sumTotalChildCount(items);
    TrackDataLabel *l = new TrackDataLabel(num, this);
    mFormLayout->addRow(labelText, l);
}


void TrackItemDetailPage::addMetadataField(const TrackDataItem *tdi, const QString &key, const QString &label)
{
    QString s = tdi->metadata(DataIndexer::self()->index(key));
    if (s.isEmpty()) return;				// nothing to display

    TrackDataLabel *l;
    if (key=="time")					// special conversion for this
    {
        l = new TrackDataLabel(QDateTime::fromString(s, Qt::ISODate), this);
    }
    else if (key=="speed")				// special conversion for this
    {
        double speed = s.toDouble();
        l = new TrackDataLabel(QString::number(speed, 'f', 3), this);
    }
    else						// just string value
    {
        l = new TrackDataLabel(s, this);
    }

    mFormLayout->addRow(label, l);
}

//////////////////////////////////////////////////////////////////////////
//									//
//  TrackFileDetailPage							//
//									//
//////////////////////////////////////////////////////////////////////////

TrackFileDetailPage::TrackFileDetailPage(const QList<TrackDataItem *> *items, QWidget *pnt)
    : TrackItemDetailPage(items, pnt)
{
    qDebug();
    setObjectName("TrackFileDetailPage");

    int nTracks = 0;
    int nFolders = 0;
    int nRoutes = 0;
    for (int i = 0; i<items->count(); ++i)
    {
        const TrackDataItem *item = items->at(i);
        for (int j = 0; j<item->childCount(); ++j)
        {
            const TrackDataItem *childItem = item->childAt(j);
            if (dynamic_cast<const TrackDataTrack *>(childItem)!=NULL) ++nTracks;
            else if (dynamic_cast<const TrackDataFolder *>(childItem)!=NULL) ++nFolders;
            else if (dynamic_cast<const TrackDataRoute *>(childItem)!=NULL) ++nRoutes;
        }
    }

    TrackDataLabel *l = new TrackDataLabel(nTracks, this);
    mFormLayout->addRow(i18nc("@label:textbox", "Tracks:"), l);
    l = new TrackDataLabel(nFolders, this);
    mFormLayout->addRow(i18nc("@label:textbox", "Folders:"), l);
    l = new TrackDataLabel(nRoutes, this);
    mFormLayout->addRow(i18nc("@label:textbox", "Routes:"), l);
    addSeparatorField();

    addDisplayFields(items, DisplayPosition|DisplayTime|DisplayTravelTime|DisplayTravelDistance);

    if (items->count()==1)				// show metadata if only one
    {
        const TrackDataFile *tdf = dynamic_cast<const TrackDataFile *>(items->first());
        Q_ASSERT(tdf!=nullptr);

        addSeparatorField();
        addMetadataField(tdf, "creator", i18nc("@label:textbox", "Creator:"));
        addMetadataField(tdf, "time", i18nc("@label:textbox", "Save time:"));
    }
}

//////////////////////////////////////////////////////////////////////////
//									//
//  TrackTrackDetailPage						//
//									//
//////////////////////////////////////////////////////////////////////////

TrackTrackDetailPage::TrackTrackDetailPage(const QList<TrackDataItem *> *items, QWidget *pnt)
    : TrackItemDetailPage(items, pnt)
{
    qDebug();
    setObjectName("TrackTrackDetailPage");

    addChildCountField(items, i18nc("@label:textbox", "Segments:"));
    addSeparatorField();

    addDisplayFields(items, DisplayPosition|DisplayTime|DisplayTravelTime|DisplayTravelDistance);

    if (items->count()==1)				// show metadata if only one
    {
        const TrackDataTrack *tdt = dynamic_cast<const TrackDataTrack *>(items->first());
        Q_ASSERT(tdt!=nullptr);

        addSeparatorField();
        addMetadataField(tdt, "creator", i18nc("@label:textbox", "Creator:"));
        addMetadataField(tdt, "time", i18nc("@label:textbox", "Time:"));
    }
}

//////////////////////////////////////////////////////////////////////////
//									//
//  TrackRouteDetailPage						//
//									//
//////////////////////////////////////////////////////////////////////////

TrackRouteDetailPage::TrackRouteDetailPage(const QList<TrackDataItem *> *items, QWidget *pnt)
    : TrackItemDetailPage(items, pnt)
{
    qDebug();
    setObjectName("TrackRouteDetailPage");

    addChildCountField(items, i18nc("@label:textbox", "Points:"));
    addDisplayFields(items, DisplayRouteLength);
    addSeparatorField();
    addDisplayFields(items, DisplayPosition);

    if (items->count()==1)				// show metadata if only one
    {
        const TrackDataRoute *tdr = dynamic_cast<const TrackDataRoute *>(items->first());
        Q_ASSERT(tdr!=nullptr);

        addSeparatorField();
        addMetadataField(tdr, "creator", i18nc("@label:textbox", "Creator:"));
        addMetadataField(tdr, "time", i18nc("@label:textbox", "Time:"));
    }
}

//////////////////////////////////////////////////////////////////////////
//									//
//  TrackSegmentDetailPage						//
//									//
//////////////////////////////////////////////////////////////////////////

TrackSegmentDetailPage::TrackSegmentDetailPage(const QList<TrackDataItem *> *items, QWidget *pnt)
    : TrackItemDetailPage(items, pnt)
{
    qDebug();
    setObjectName("TrackSegmentDetailPage");

    addChildCountField(items, i18nc("@label:textbox", "Points:"));
    addSeparatorField();

    DisplayItems disp = DisplayPosition|DisplayTime|DisplayTravelDistance|DisplayAverageSpeed;
    if (items->count()>1) disp |= DisplayTravelTime;
    addDisplayFields(items, disp);

    if (items->count()==1)				// show interval if only one
    {
        const TrackDataSegment *tds = dynamic_cast<const TrackDataSegment *>(items->first());
        Q_ASSERT(tds!=nullptr);

        int cnt = tds->childCount();
        if (cnt>1)					// if more than one point
        {
            const TrackDataTrackpoint *first = dynamic_cast<const TrackDataTrackpoint *>(tds->childAt(0));
            const TrackDataTrackpoint *last = dynamic_cast<const TrackDataTrackpoint *>(tds->childAt(cnt-1));
            Q_ASSERT(first!=nullptr && last!=nullptr);
            int tt = qRound(double(first->timeTo(last))/(cnt-1));
            QLabel *l = new TrackDataLabel(TrackData::formattedDuration(tt), this);
            mFormLayout->addRow(i18nc("@label:textbox", "Interval:"), l);
        }
    }
}

//////////////////////////////////////////////////////////////////////////
//									//
//  TrackTrackpointDetailPage						//
//									//
//////////////////////////////////////////////////////////////////////////

TrackTrackpointDetailPage::TrackTrackpointDetailPage(const QList<TrackDataItem *> *items, QWidget *pnt)
    : TrackItemDetailPage(items, pnt)
{
    qDebug();
    setObjectName("TrackPointDetailPage");

    if (items->count()==1)				// single selection
    {
        const TrackDataTrackpoint *tdp = dynamic_cast<const TrackDataTrackpoint *>(items->first());
        Q_ASSERT(tdp!=nullptr);

        addDisplayFields(items, DisplayPosition|DisplayTime|DisplayElevation);
        addSeparatorField();
        addMetadataField(tdp, "hdop", i18nc("@label:textbox", "GPS HDOP:"));
        addMetadataField(tdp, "speed", i18nc("@label:textbox", "GPS speed:"));
    }
    else						// multiple selection
    {
        addDisplayFields(items, DisplayPosition|DisplayTime|DisplayElevation|DisplayTravelDistance|DisplayAverageSpeed|DisplayStraightLine|DisplayRelativeBearing);
    }
}

//////////////////////////////////////////////////////////////////////////
//									//
//  TrackFolderDetailPage						//
//									//
//////////////////////////////////////////////////////////////////////////

TrackFolderDetailPage::TrackFolderDetailPage(const QList<TrackDataItem *> *items, QWidget *pnt)
    : TrackItemDetailPage(items, pnt)
{
    qDebug();
    setObjectName("TrackFolderDetailPage");

    int nWaypoints = 0;
    int nFolders = 0;
    for (int i = 0; i<items->count(); ++i)
    {
        const TrackDataItem *item = items->at(i);
        for (int j = 0; j<item->childCount(); ++j)
        {
            const TrackDataItem *childItem = item->childAt(j);
            if (dynamic_cast<const TrackDataWaypoint *>(childItem)!=NULL) ++nWaypoints;
            else if (dynamic_cast<const TrackDataFolder *>(childItem)!=NULL) ++nFolders;
        }
    }

    TrackDataLabel *l = new TrackDataLabel(nWaypoints, this);
    mFormLayout->addRow(i18nc("@label:textbox", "Waypoints:"), l);
    l = new TrackDataLabel(nFolders, this);
    mFormLayout->addRow(i18nc("@label:textbox", "Subfolders:"), l);

    addSeparatorField();

    QLabel *pathDisplay = new QLabel(this);
    pathDisplay->setTextInteractionFlags(Qt::TextSelectableByMouse|Qt::TextSelectableByKeyboard);
    mFormLayout->addRow(i18nc("@label:textbox", "Path:"), pathDisplay);

    if (items->count()==1)				// a single item
    {
        TrackDataFolder *folderItem = dynamic_cast<TrackDataFolder *>(items->first());
        Q_ASSERT(folderItem!=NULL);
        pathDisplay->setText(folderItem->path());
    }
    else disableIfEmpty(pathDisplay, true);
}

//////////////////////////////////////////////////////////////////////////
//									//
//  TrackWaypointDetailPage						//
//									//
//////////////////////////////////////////////////////////////////////////

TrackWaypointDetailPage::TrackWaypointDetailPage(const QList<TrackDataItem *> *items, QWidget *pnt)
    : TrackItemDetailPage(items, pnt)
{
    qDebug();
    setObjectName("TrackWaypointDetailPage");

    if (items->count()==1)				// single selection
    {
        const TrackDataWaypoint *tdp = dynamic_cast<const TrackDataWaypoint *>(items->first());
        Q_ASSERT(tdp!=NULL);

        addDisplayFields(items, DisplayPosition|DisplayTime|DisplayElevation);
        addSeparatorField();

        QLabel *pathDisplay = new QLabel(this);
        pathDisplay->setTextInteractionFlags(Qt::TextSelectableByMouse|Qt::TextSelectableByKeyboard);
        mFormLayout->addRow(i18nc("@label:textbox", "Folder:"), pathDisplay);

        TrackDataFolder *folderItem = dynamic_cast<TrackDataFolder *>(tdp->parent());
        Q_ASSERT(folderItem!=nullptr);
        pathDisplay->setText(folderItem->path());
    }
    else						// multiple selection
    {
        addDisplayFields(items, DisplayPosition|DisplayElevation|DisplayStraightLine|DisplayRelativeBearing);
    }
}

//////////////////////////////////////////////////////////////////////////
//									//
//  TrackRoutepointDetailPage						//
//									//
//////////////////////////////////////////////////////////////////////////

TrackRoutepointDetailPage::TrackRoutepointDetailPage(const QList<TrackDataItem *> *items, QWidget *pnt)
    : TrackItemDetailPage(items, pnt)
{
    qDebug();
    setObjectName("TrackRoutepointDetailPage");

    if (items->count()==1)				// single selection
    {
        addDisplayFields(items, DisplayPosition|DisplayElevation);
    }
    else						// multiple selection
    {
        addDisplayFields(items, DisplayPosition|DisplayElevation|DisplayStraightLine|DisplayRelativeBearing|DisplayTravelDistance);
    }
}


CREATE_PROPERTIES_PAGE(File, Detail)
CREATE_PROPERTIES_PAGE(Track, Detail)
CREATE_PROPERTIES_PAGE(Route, Detail)
CREATE_PROPERTIES_PAGE(Segment, Detail)
CREATE_PROPERTIES_PAGE(Trackpoint, Detail)
CREATE_PROPERTIES_PAGE(Folder, Detail)
CREATE_PROPERTIES_PAGE(Waypoint, Detail)
CREATE_PROPERTIES_PAGE(Routepoint, Detail)
