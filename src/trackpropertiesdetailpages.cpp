
#include "trackpropertiesdetailpages.h"

#include <qformlayout.h>
#include <qdebug.h>

#include <klocalizedstring.h>

#include "trackdata.h"
#include "trackdatalabel.h"
#include "variableunitdisplay.h"
#include "dataindexer.h"



TrackItemDetailPage::TrackItemDetailPage(const QList<TrackDataItem *> *items, QWidget *pnt)
    : TrackPropertiesPage(items, pnt)
{
    qDebug();
    setObjectName("TrackItemDetailPage");

    addSeparatorField();
}



void TrackItemDetailPage::addTimeDistanceSpeedFields(const QList<TrackDataItem *> *items, bool bothTimes, bool tracksOnly)
{
    TimeRange tsp = TrackData::unifyTimeSpans(items);
    const bool blankIfZero = (isEmpty() || !tsp.isValid());

    TrackDataLabel *l = new TrackDataLabel(tsp.start(), this);
    mFormLayout->addRow(i18nc("@label:textbox", "Time start:"), l);
    disableIfEmpty(l);

    l = new TrackDataLabel(tsp.finish(), this);
    mFormLayout->addRow(i18nc("@label:textbox", "Time end:"), l);
    disableIfEmpty(l);

    unsigned tt = tsp.timeSpan();
    l = new TrackDataLabel(TrackData::formattedDuration(tt, blankIfZero), this);
    mFormLayout->addRow(i18nc("@label:textbox", "Time span:"), l);
    disableIfEmpty(l);

    if (bothTimes && dynamic_cast<TrackDataFile *>(items->first())==NULL)
    {
        tt = TrackData::sumTotalTravelTime(items);
        l = new TrackDataLabel(TrackData::formattedDuration(tt, blankIfZero), this);
        mFormLayout->addRow(i18nc("@label:textbox", "Travel time:"), l);
        disableIfEmpty(l);
    }

    addSeparatorField();

    double dist = TrackData::sumTotalTravelDistance(items, tracksOnly);
    VariableUnitDisplay *vl = new VariableUnitDisplay(VariableUnitCombo::Distance, this);
    vl->setSaveId("totaltraveldistance");
    vl->setValue(dist);
    mFormLayout->addRow(i18nc("@label:textbox", "Travel distance:"), vl);
    disableIfEmpty(vl);

    QWidget *w;
    if (tt>0)
    {
        double averageSpeed = dist/(tt/3600.0);
        vl = new VariableUnitDisplay(VariableUnitCombo::Speed, this);
        vl->setSaveId("averagespeed");
        vl->setValue(averageSpeed);
        w = vl;
    }
    else w = new QLabel(i18nc("Not available", "N/A"), this);
    mFormLayout->addRow(i18nc("@label:textbox", "Average speed:"), w);
    disableIfEmpty(w);
}



void TrackItemDetailPage::addBoundingAreaField(const QList<TrackDataItem *> *items)
{
    BoundingArea bb = TrackData::unifyBoundingAreas(items);
    TrackDataLabel *l = new TrackDataLabel(bb.north(), bb.west(), true, this);
    mFormLayout->addRow(i18nc("@label:textbox", "Bounding area:"), l);
    disableIfEmpty(l);
    l = new TrackDataLabel(bb.south(), bb.east(), true, this);
    mFormLayout->addRow("", l);
    disableIfEmpty(l);
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



TrackFileDetailPage::TrackFileDetailPage(const QList<TrackDataItem *> *items, QWidget *pnt)
    : TrackItemDetailPage(items, pnt)
{
    qDebug();
    setObjectName("TrackFileDetailPage");

    int nTracks = 0;
    int nFolders = 0;
    for (int i = 0; i<items->count(); ++i)
    {
        const TrackDataItem *item = items->at(i);
        for (int j = 0; j<item->childCount(); ++j)
        {
            const TrackDataItem *childItem = item->childAt(j);
            if (dynamic_cast<const TrackDataTrack *>(childItem)!=NULL) ++nTracks;
            else if (dynamic_cast<const TrackDataFolder *>(childItem)!=NULL) ++nFolders;
        }
    }

    TrackDataLabel *l = new TrackDataLabel(nTracks, this);
    mFormLayout->addRow(i18nc("@label:textbox", "Tracks:"), l);
    l = new TrackDataLabel(nFolders, this);
    mFormLayout->addRow(i18nc("@label:textbox", "Folders:"), l);

    addBoundingAreaField(items);
    addTimeDistanceSpeedFields(items);

    if (items->count()==1)				// should always be so
    {
        const TrackDataFile *tdf = dynamic_cast<const TrackDataFile *>(items->first());
        Q_ASSERT(tdf!=NULL);

        addSeparatorField(i18nc("@title:group", "File"));
        addMetadataField(tdf, "creator", i18nc("@label:textbox", "Creator:"));
        addMetadataField(tdf, "time", i18nc("@label:textbox", "Save time:"));
    }
}



TrackTrackDetailPage::TrackTrackDetailPage(const QList<TrackDataItem *> *items, QWidget *pnt)
    : TrackItemDetailPage(items, pnt)
{
    qDebug();
    setObjectName("TrackTrackDetailPage");

    addChildCountField(items, i18nc("@label:textbox", "Segments:"));
    addBoundingAreaField(items);
    addTimeDistanceSpeedFields(items);

    if (items->count()==1)				// should always be so
    {
        const TrackDataTrack *tdt = dynamic_cast<const TrackDataTrack *>(items->first());
        Q_ASSERT(tdt!=NULL);

        addSeparatorField(i18nc("@title:group", "Source"));
        addMetadataField(tdt, "creator", i18nc("@label:textbox", "Creator:"));
        addMetadataField(tdt, "time", i18nc("@label:textbox", "Time:"));
    }
}



TrackRouteDetailPage::TrackRouteDetailPage(const QList<TrackDataItem *> *items, QWidget *pnt)
    : TrackItemDetailPage(items, pnt)
{
    qDebug();
    setObjectName("TrackRouteDetailPage");

    addChildCountField(items, i18nc("@label:textbox", "Points:"));
    addBoundingAreaField(items);
    addSeparatorField();

    double dist = TrackData::sumTotalTravelDistance(items, false);
    VariableUnitDisplay *vl = new VariableUnitDisplay(VariableUnitCombo::Distance, this);
    vl->setSaveId("totaltraveldistance");
    vl->setValue(dist);
    mFormLayout->addRow(i18nc("@label:textbox", "Length:"), vl);
    disableIfEmpty(vl);

    if (items->count()==1)				// should always be so
    {
        const TrackDataRoute *tdr = dynamic_cast<const TrackDataRoute *>(items->first());
        Q_ASSERT(tdr!=NULL);

        addSeparatorField(i18nc("@title:group", "Source"));
        addMetadataField(tdr, "creator", i18nc("@label:textbox", "Creator:"));
        addMetadataField(tdr, "time", i18nc("@label:textbox", "Time:"));
    }
}



TrackSegmentDetailPage::TrackSegmentDetailPage(const QList<TrackDataItem *> *items, QWidget *pnt)
    : TrackItemDetailPage(items, pnt)
{
    qDebug();
    setObjectName("TrackSegmentDetailPage");

    addChildCountField(items, i18nc("@label:textbox", "Points:"));
    addBoundingAreaField(items);
    addTimeDistanceSpeedFields(items, false);

    if (items->count()==1)				// should always be so
    {
        const TrackDataSegment *tds = dynamic_cast<const TrackDataSegment *>(items->first());
        Q_ASSERT(tds!=NULL);

        int cnt = tds->childCount();
        if (cnt>1)
        {
            const TrackDataTrackpoint *first = dynamic_cast<const TrackDataTrackpoint *>(tds->childAt(0));
            const TrackDataTrackpoint *last = dynamic_cast<const TrackDataTrackpoint *>(tds->childAt(cnt-1));
            Q_ASSERT(first!=NULL && last!=NULL);
            int tt = qRound(double(first->timeTo(last))/(cnt-1));
            QLabel *l = new TrackDataLabel(TrackData::formattedDuration(tt), this);
            mFormLayout->addRow(i18nc("@label:textbox", "Interval:"), l);
        }
    }
}



TrackTrackpointDetailPage::TrackTrackpointDetailPage(const QList<TrackDataItem *> *items, QWidget *pnt)
    : TrackItemDetailPage(items, pnt)
{
    qDebug();
    setObjectName("TrackPointDetailPage");

    if (items->count()==1)				// single selection
    {
        const TrackDataTrackpoint *tdp = dynamic_cast<const TrackDataTrackpoint *>(items->first());
        Q_ASSERT(tdp!=NULL);

        TrackDataLabel *l = new TrackDataLabel(tdp->formattedPosition(), this);
        mFormLayout->addRow(i18nc("@label:textbox", "Position:"), l);
        mPositionLabel = l;

        l = new TrackDataLabel(tdp->time(), this);
        mFormLayout->addRow(i18nc("@label:textbox", "Time:"), l);

        double ele = tdp->elevation();
        if (!ISNAN(ele))
        {
            addSeparatorField();

            VariableUnitDisplay *vl = new VariableUnitDisplay(VariableUnitCombo::Elevation, this);
            vl->setSaveId("elevation");
            vl->setValue(ele);
            mFormLayout->addRow(i18nc("@label:textbox", "Elevation:"), vl);
        }

        addSeparatorField();

        addMetadataField(tdp, "hdop", i18nc("@label:textbox", "GPS HDOP:"));
        addMetadataField(tdp, "speed", i18nc("@label:textbox", "GPS speed:"));
    }
    else						// multiple selection
    {
        addBoundingAreaField(items);

        const TrackDataItem *seg = items->first()->parent();
        Q_ASSERT(seg!=NULL);				// find parent segment
        int firstIdx = seg->childIndex(items->first());	// its index of first item
        int num = items->count();

        bool contiguousSelection = true;		// assume so at start
        for (int i = 1; i<num; ++i)			// look at following items
        {
            if (seg->childAt(firstIdx+i)!=items->at(i))	// mismatch children/selection
            {
                contiguousSelection = false;
                break;
            }
        }

        if (contiguousSelection)			// selection is contiguous
        {
            addTimeDistanceSpeedFields(items, false);
        }

        if (items->count()==2)				// exactly two points selected
        {						// (nocontiguous doesn't matter)
            TrackDataItem *item1 = items->first();
            TrackDataItem *item2 = items->last();

            int idx1 = seg->childIndex(item1);
            int idx2 = seg->childIndex(item2);
            if (idx1>idx2) qSwap(idx1, idx2);		// order by index = time

            QList<TrackDataItem *> items2;
            for (int i = idx1; i<=idx2; ++i) items2.append(seg->childAt(i));

            if (!contiguousSelection)			// not already added above
            {
                addTimeDistanceSpeedFields(&items2, false);
            }
            addSeparatorField();

            TrackDataTrackpoint *pnt1 = dynamic_cast<TrackDataTrackpoint *>(seg->childAt(idx1));
            TrackDataTrackpoint *pnt2 = dynamic_cast<TrackDataTrackpoint *>(seg->childAt(idx2));
            Q_ASSERT(pnt1!=NULL && pnt2!=NULL);

            VariableUnitDisplay *vl = new VariableUnitDisplay(VariableUnitCombo::Bearing, this);
            vl->setSaveId("bearing");
            vl->setValue(pnt1->bearingTo(pnt2));
            mFormLayout->addRow(i18nc("@label:textbox", "Relative bearing:"), vl);

            if (!contiguousSelection)
            {
                vl = new VariableUnitDisplay(VariableUnitCombo::Distance, this);
                vl->setSaveId("crowflies");
                vl->setValue(pnt1->distanceTo(pnt2, true));
                mFormLayout->addRow(i18nc("@label:textbox", "Straight line distance:"), vl);
            }

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



TrackWaypointDetailPage::TrackWaypointDetailPage(const QList<TrackDataItem *> *items, QWidget *pnt)
    : TrackItemDetailPage(items, pnt)
{
    qDebug();
    setObjectName("TrackWaypointDetailPage");

    if (items->count()==1)				// single selection
    {
        const TrackDataWaypoint *tdp = dynamic_cast<const TrackDataWaypoint *>(items->first());
        Q_ASSERT(tdp!=NULL);

        TrackDataLabel *l = new TrackDataLabel(tdp->formattedPosition(), this);
        mFormLayout->addRow(i18nc("@label:textbox", "Position:"), l);
        mPositionLabel = l;

        l = new TrackDataLabel(tdp->time(), this);
        mFormLayout->addRow(i18nc("@label:textbox", "Time:"), l);

        double ele = tdp->elevation();
        if (!ISNAN(ele))
        {
            addSeparatorField();

            VariableUnitDisplay *vl = new VariableUnitDisplay(VariableUnitCombo::Elevation, this);
            vl->setSaveId("elevation");
            vl->setValue(ele);
            mFormLayout->addRow(i18nc("@label:textbox", "Elevation:"), vl);
        }

        addSeparatorField();

        QLabel *pathDisplay = new QLabel(this);
        pathDisplay->setTextInteractionFlags(Qt::TextSelectableByMouse|Qt::TextSelectableByKeyboard);
        mFormLayout->addRow(i18nc("@label:textbox", "Folder:"), pathDisplay);

        TrackDataFolder *folderItem = dynamic_cast<TrackDataFolder *>(tdp->parent());
        Q_ASSERT(folderItem!=NULL);
        pathDisplay->setText(folderItem->path());
    }
    else						// multiple selection
    {
        addBoundingAreaField(items);

        const TrackDataItem *seg = items->first()->parent();
        Q_ASSERT(seg!=NULL);				// find parent segment
        int firstIdx = seg->childIndex(items->first());	// its index of first item
        int num = items->count();

        bool contiguousSelection = true;		// assume so at start
        for (int i = 1; i<num; ++i)			// look at following items
        {
            if (seg->childAt(firstIdx+i)!=items->at(i))	// mismatch children/selection
            {
                contiguousSelection = false;
                break;
            }
        }

        if (contiguousSelection)			// selection is contiguous
        {
            addTimeDistanceSpeedFields(items, false, false);
        }

        if (items->count()==2)				// exactly two points selected
        {						// (nocontiguous doesn't matter)
            TrackDataItem *item1 = items->first();
            TrackDataItem *item2 = items->last();

            int idx1 = seg->childIndex(item1);
            int idx2 = seg->childIndex(item2);
            if (idx1>idx2) qSwap(idx1, idx2);		// order by index = time

            QList<TrackDataItem *> items2;
            for (int i = idx1; i<=idx2; ++i) items2.append(seg->childAt(i));

            if (!contiguousSelection)			// not already added above
            {
                addTimeDistanceSpeedFields(&items2, false, false);
            }
            addSeparatorField();

            TrackDataWaypoint *pnt1 = dynamic_cast<TrackDataWaypoint *>(seg->childAt(idx1));
            TrackDataWaypoint *pnt2 = dynamic_cast<TrackDataWaypoint *>(seg->childAt(idx2));
            Q_ASSERT(pnt1!=NULL && pnt2!=NULL);

            VariableUnitDisplay *vl = new VariableUnitDisplay(VariableUnitCombo::Bearing, this);
            vl->setSaveId("bearing");
            vl->setValue(pnt1->bearingTo(pnt2));
            mFormLayout->addRow(i18nc("@label:textbox", "Relative bearing:"), vl);

            if (!contiguousSelection)
            {
                vl = new VariableUnitDisplay(VariableUnitCombo::Distance, this);
                vl->setSaveId("crowflies");
                vl->setValue(pnt1->distanceTo(pnt2, true));
                mFormLayout->addRow(i18nc("@label:textbox", "Straight line distance:"), vl);
            }

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



TrackRoutepointDetailPage::TrackRoutepointDetailPage(const QList<TrackDataItem *> *items, QWidget *pnt)
    : TrackItemDetailPage(items, pnt)
{
    qDebug();
    setObjectName("TrackRoutepointDetailPage");

    if (items->count()==1)				// single selection
    {
        const TrackDataRoutepoint *tdp = dynamic_cast<const TrackDataRoutepoint *>(items->first());
        Q_ASSERT(tdp!=NULL);

        TrackDataLabel *l = new TrackDataLabel(tdp->formattedPosition(), this);
        mFormLayout->addRow(i18nc("@label:textbox", "Position:"), l);
        mPositionLabel = l;

//         l = new TrackDataLabel(tdp->time(), this);
//         mFormLayout->addRow(i18nc("@label:textbox", "Time:"), l);
// 
//         double ele = tdp->elevation();
//         if (!ISNAN(ele))
//         {
//             addSeparatorField();
// 
//             VariableUnitDisplay *vl = new VariableUnitDisplay(VariableUnitCombo::Elevation, this);
//             vl->setSaveId("elevation");
//             vl->setValue(ele);
//             mFormLayout->addRow(i18nc("@label:textbox", "Elevation:"), vl);
//         }

//         addSeparatorField();

//         QLabel *pathDisplay = new QLabel(this);
//         pathDisplay->setTextInteractionFlags(Qt::TextSelectableByMouse|Qt::TextSelectableByKeyboard);
//         mFormLayout->addRow(i18nc("@label:textbox", "Folder:"), pathDisplay);
// 
//         TrackDataFolder *folderItem = dynamic_cast<TrackDataFolder *>(tdp->parent());
//         Q_ASSERT(folderItem!=NULL);
//         pathDisplay->setText(folderItem->path());
    }
    else						// multiple selection
    {
        addBoundingAreaField(items);

        const TrackDataItem *seg = items->first()->parent();
        Q_ASSERT(seg!=NULL);				// find parent segment
        int firstIdx = seg->childIndex(items->first());	// its index of first item
        int num = items->count();

        bool contiguousSelection = true;		// assume so at start
        for (int i = 1; i<num; ++i)			// look at following items
        {
            if (seg->childAt(firstIdx+i)!=items->at(i))	// mismatch children/selection
            {
                contiguousSelection = false;
                break;
            }
        }

        if (contiguousSelection)			// selection is contiguous
        {
            addTimeDistanceSpeedFields(items, false, false);
        }

        if (items->count()==2)				// exactly two points selected
        {						// (nocontiguous doesn't matter)
            TrackDataItem *item1 = items->first();
            TrackDataItem *item2 = items->last();

            int idx1 = seg->childIndex(item1);
            int idx2 = seg->childIndex(item2);
            if (idx1>idx2) qSwap(idx1, idx2);		// order by index = time

//             QList<TrackDataItem *> items2;
//             for (int i = idx1; i<=idx2; ++i) items2.append(seg->childAt(i));
// 
//             if (!contiguousSelection)			// not already added above
//             {
//                 addTimeDistanceSpeedFields(&items2, false, false);
//             }
            addSeparatorField();

            TrackDataRoutepoint *pnt1 = dynamic_cast<TrackDataRoutepoint *>(seg->childAt(idx1));
            TrackDataRoutepoint *pnt2 = dynamic_cast<TrackDataRoutepoint *>(seg->childAt(idx2));
            Q_ASSERT(pnt1!=NULL && pnt2!=NULL);

            VariableUnitDisplay *vl = new VariableUnitDisplay(VariableUnitCombo::Bearing, this);
            vl->setSaveId("bearing");
            vl->setValue(pnt1->bearingTo(pnt2));
            mFormLayout->addRow(i18nc("@label:textbox", "Relative bearing:"), vl);

            if (!contiguousSelection)
            {
                vl = new VariableUnitDisplay(VariableUnitCombo::Distance, this);
                vl->setSaveId("crowflies");
                vl->setValue(pnt1->distanceTo(pnt2, true));
                mFormLayout->addRow(i18nc("@label:textbox", "Straight line distance:"), vl);
            }

//             double ele1 = pnt1->elevation();
//             double ele2 = pnt2->elevation();
//             if (!ISNAN(ele1) && !ISNAN(ele2))
//             {
//                 vl = new VariableUnitDisplay(VariableUnitCombo::Elevation, this);
//                 vl->setSaveId("elediff");
//                 vl->setValue(ele2-ele1);
//                 mFormLayout->addRow(i18nc("@label:textbox", "Elevation difference:"), vl);
//             }
        }
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
