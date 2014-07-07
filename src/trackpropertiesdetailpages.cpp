
#include "trackpropertiesdetailpages.h"

#include <qformlayout.h>

#include <kdebug.h>
#include <kdialog.h>
#include <klocale.h>
#include <kglobal.h>

#include "trackdata.h"
#include "trackdatalabel.h"
#include "variableunitdisplay.h"
#include "dataindexer.h"






TrackItemDetailPage::TrackItemDetailPage(const QList<TrackDataItem *> items, QWidget *pnt)
    : TrackPropertiesPage(items, pnt)
{
    kDebug();
    setObjectName("TrackItemDetailPage");

    addSeparatorField();
}





void TrackItemDetailPage::addTimeDistanceSpeedFields(const QList<TrackDataItem *> &items, bool bothTimes)
{
    TimeRange tsp = TrackData::unifyTimeSpans(items);
    TrackDataLabel *l = new TrackDataLabel(tsp.start(), this);
    mFormLayout->addRow(i18nc("@label:textbox", "Time start:"), l);
    disableIfEmpty(l);

    l = new TrackDataLabel(tsp.finish(), this);
    mFormLayout->addRow(i18nc("@label:textbox", "Time end:"), l);
    disableIfEmpty(l);

    unsigned tt = tsp.timeSpan();
    l = new TrackDataLabel(TrackData::formattedDuration(tt, isEmpty()), this);
    mFormLayout->addRow(i18nc("@label:textbox", "Time span:"), l);
    disableIfEmpty(l);

    if (bothTimes)
    {
        tt = TrackData::sumTotalTravelTime(items);
        l = new TrackDataLabel(TrackData::formattedDuration(tt, isEmpty()), this);
        mFormLayout->addRow(i18nc("@label:textbox", "Travel time:"), l);
        disableIfEmpty(l);
    }

    addSeparatorField();

    double dist = TrackData::sumTotalTravelDistance(items);
    VariableUnitDisplay *vl = new VariableUnitDisplay(VariableUnitDisplay::Distance, this);
    vl->setSaveId("totaltraveldistance");
    vl->setValue(dist);
    mFormLayout->addRow(i18nc("@label:textbox", "Travel distance:"), vl);
    disableIfEmpty(vl);

    double averageSpeed = dist/(tt/3600.0);
    vl = new VariableUnitDisplay(VariableUnitDisplay::Speed, this);
    vl->setSaveId("averagespeed");
    vl->setValue(averageSpeed);
    mFormLayout->addRow(i18nc("@label:textbox", "Average speed:"), vl);
    disableIfEmpty(vl);
}



void TrackItemDetailPage::addBoundingAreaField(const QList<TrackDataItem *> &items)
{
    BoundingArea bb = TrackData::unifyBoundingAreas(items);
    TrackDataLabel *l = new TrackDataLabel(bb.north(), bb.west(), true, this);
    mFormLayout->addRow(i18nc("@label:textbox", "Bounding area:"), l);
    disableIfEmpty(l);
    l = new TrackDataLabel(bb.south(), bb.east(), true, this);
    mFormLayout->addRow(QString::null, l);
    disableIfEmpty(l);
}



void TrackItemDetailPage::addChildCountField(const QList<TrackDataItem *> &items, const QString &labelText)
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




TrackFileDetailPage::TrackFileDetailPage(const QList<TrackDataItem *> items, QWidget *pnt)
    : TrackItemDetailPage(items, pnt)
{
    kDebug();
    setObjectName("TrackFileDetailPage");

    addChildCountField(items, i18nc("@label:textbox", "Tracks:"));
    addBoundingAreaField(items);
    addTimeDistanceSpeedFields(items);

    if (items.count()==1)				// should always be so
    {
        const TrackDataFile *tdf = dynamic_cast<const TrackDataFile *>(items.first());
        Q_ASSERT(tdf!=NULL);

        addSeparatorField(i18nc("@title:group", "File"));
        addMetadataField(tdf, "creator", i18nc("@label:textbox", "Creator:"));
        addMetadataField(tdf, "time", i18nc("@label:textbox", "Save time:"));
    }
}







TrackTrackDetailPage::TrackTrackDetailPage(const QList<TrackDataItem *> items, QWidget *pnt)
    : TrackItemDetailPage(items, pnt)
{
    kDebug();
    setObjectName("TrackTrackDetailPage");

    addChildCountField(items, i18nc("@label:textbox", "Segments:"));
    addBoundingAreaField(items);
    addTimeDistanceSpeedFields(items);

    if (items.count()==1)				// should always be so
    {
        const TrackDataTrack *tdt = dynamic_cast<const TrackDataTrack *>(items.first());
        Q_ASSERT(tdt!=NULL);

        addSeparatorField(i18nc("@title:group", "Source"));
        addMetadataField(tdt, "creator", i18nc("@label:textbox", "Creator:"));
        addMetadataField(tdt, "time", i18nc("@label:textbox", "Time:"));
    }
}






TrackSegmentDetailPage::TrackSegmentDetailPage(const QList<TrackDataItem *> items, QWidget *pnt)
    : TrackItemDetailPage(items, pnt)
{
    kDebug();
    setObjectName("TrackSegmentDetailPage");

    addChildCountField(items, i18nc("@label:textbox", "Points:"));
    addBoundingAreaField(items);
    addTimeDistanceSpeedFields(items, false);

    if (items.count()==1)				// should always be so
    {
        const TrackDataSegment *tds = dynamic_cast<const TrackDataSegment *>(items.first());
        Q_ASSERT(tds!=NULL);

        int cnt = tds->childCount();
        if (cnt>1)
        {
            const TrackDataPoint *first = dynamic_cast<const TrackDataPoint *>(tds->childAt(0));
            const TrackDataPoint *last = dynamic_cast<const TrackDataPoint *>(tds->childAt(cnt-1));
            Q_ASSERT(first!=NULL && last!=NULL);
            int tt = qRound(double(first->timeTo(last))/(cnt-1));
            QLabel *l = new TrackDataLabel(TrackData::formattedDuration(tt), this);
            mFormLayout->addRow(i18nc("@label:textbox", "Interval:"), l);
        }
    }
}






TrackPointDetailPage::TrackPointDetailPage(const QList<TrackDataItem *> items, QWidget *pnt)
    : TrackItemDetailPage(items, pnt)
{
    kDebug();
    setObjectName("TrackPointDetailPage");

    if (items.count()==1)				// single selection
    {
        const TrackDataPoint *tdp = dynamic_cast<const TrackDataPoint *>(items.first());
        Q_ASSERT(tdp!=NULL);

        TrackDataLabel *l = new TrackDataLabel(tdp->formattedPosition(), this);
        mFormLayout->addRow(i18nc("@label:textbox", "Position:"), l);

        l = new TrackDataLabel(tdp->time(), this);
        mFormLayout->addRow(i18nc("@label:textbox", "Time:"), l);

        double ele = tdp->elevation();
        if (!isnan(ele))
        {
            addSeparatorField();

            VariableUnitDisplay *vl = new VariableUnitDisplay(VariableUnitDisplay::Elevation, this);
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

        const TrackDataItem *seg = items.first()->parent();
        Q_ASSERT(seg!=NULL);				// find parent segment
        int firstIdx = seg->childIndex(items.first());	// its index of first item
        int num = items.count();

        bool contiguousSelection = true;		// assume so at start
        for (int i = 1; i<num; ++i)			// look at following items
        {
            if (seg->childAt(firstIdx+i)!=items.at(i))	// mismatch children/selection
            {
                contiguousSelection = false;
                break;
            }
        }

        if (contiguousSelection)			// selection is contiguous
        {
            addTimeDistanceSpeedFields(items, false);
        }

        if (items.count()==2)				// exactly two points selected
        {						// (nocontiguous doesn't matter)
            TrackDataItem *item1 = items.first();
            TrackDataItem *item2 = items.last();

            int idx1 = seg->childIndex(item1);
            int idx2 = seg->childIndex(item2);
            if (idx1>idx2) qSwap(idx1, idx2);		// order by index = time

            QList<TrackDataItem *> items2;
            for (int i = idx1; i<=idx2; ++i) items2.append(seg->childAt(i));

            if (!contiguousSelection)			// not already added above
            {
                addTimeDistanceSpeedFields(items2, false);
            }
            addSeparatorField();

            TrackDataPoint *pnt1 = dynamic_cast<TrackDataPoint *>(seg->childAt(idx1));
            TrackDataPoint *pnt2 = dynamic_cast<TrackDataPoint *>(seg->childAt(idx2));
            Q_ASSERT(pnt1!=NULL && pnt2!=NULL);

            VariableUnitDisplay *vl = new VariableUnitDisplay(VariableUnitDisplay::Bearing, this);
            vl->setSaveId("bearing");
            vl->setValue(pnt1->bearingTo(pnt2));
            mFormLayout->addRow(i18nc("@label:textbox", "Relative bearing:"), vl);

            if (!contiguousSelection)
            {
                vl = new VariableUnitDisplay(VariableUnitDisplay::Distance, this);
                vl->setSaveId("crowflies");
                vl->setValue(pnt1->distanceTo(pnt2, true));
                mFormLayout->addRow(i18nc("@label:textbox", "Straight line distance:"), vl);
            }

            double ele1 = pnt1->elevation();
            double ele2 = pnt2->elevation();
            if (!isnan(ele1) && !isnan(ele2))
            {
                vl = new VariableUnitDisplay(VariableUnitDisplay::Elevation, this);
                vl->setSaveId("elediff");
                vl->setValue(ele2-ele1);
                mFormLayout->addRow(i18nc("@label:textbox", "Elevation difference:"), vl);
            }
        }
    }
}


CREATE_PROPERTIES_PAGE(File, Detail);
CREATE_PROPERTIES_PAGE(Track, Detail);
CREATE_PROPERTIES_PAGE(Segment, Detail);
CREATE_PROPERTIES_PAGE(Point, Detail);
