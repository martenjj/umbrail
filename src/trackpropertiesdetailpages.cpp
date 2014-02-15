
#include "trackpropertiesdetailpages.h"

#include <qformlayout.h>

#include <kdebug.h>
#include <kdialog.h>
#include <klocale.h>
#include <kglobal.h>

#include "trackdata.h"
#include "trackdatalabel.h"
#include "variableunitdisplay.h"






TrackItemDetailPage::TrackItemDetailPage(const QList<TrackDataItem *> items, QWidget *pnt)
    : TrackPropertiesPage(items, pnt)
{
    kDebug();
    setObjectName("TrackItemDetailPage");

    addSpacerField();
}





void TrackItemDetailPage::addTimeDistanceSpeedFields(const QList<TrackDataItem *> &items, bool bothTimes)
{
    TimeRange tsp = TrackData::unifyTimeSpans(items);
    TrackDataLabel *l = new TrackDataLabel(tsp.start(), this);
    mFormLayout->addRow(i18nc("@label:textbox", "Time start:"), l);

    l = new TrackDataLabel(tsp.finish(), this);
    mFormLayout->addRow(i18nc("@label:textbox", "Time end:"), l);

    unsigned tt = tsp.timeSpan();
    l = new TrackDataLabel(TrackData::formattedDuration(tt), this);
    mFormLayout->addRow(i18nc("@label:textbox", "Time span:"), l);

    if (bothTimes)
    {
        tt = TrackData::sumTotalTravelTime(items);
        l = new TrackDataLabel(TrackData::formattedDuration(tt), this);
        mFormLayout->addRow(i18nc("@label:textbox", "Travel time:"), l);
    }

    double dist = TrackData::sumTotalTravelDistance(items);
    VariableUnitDisplay *vl = new VariableUnitDisplay(VariableUnitDisplay::Distance, this);
    vl->setSaveId("totaltraveldistance");
    vl->setValue(dist);
    mFormLayout->addRow(i18nc("@label:textbox", "Travel distance:"), vl);

    double averageSpeed = dist/(tt/3600.0);
    vl = new VariableUnitDisplay(VariableUnitDisplay::Speed, this);
    vl->setSaveId("averagespeed");
    vl->setValue(averageSpeed);
    mFormLayout->addRow(i18nc("@label:textbox", "Average speed:"), vl);
}



void TrackItemDetailPage::addBoundingAreaField(const QList<TrackDataItem *> &items)
{
    BoundingArea bb = TrackData::unifyBoundingAreas(items);
    TrackDataLabel *l = new TrackDataLabel(bb.north(), bb.west(), this);
    mFormLayout->addRow(i18nc("@label:textbox", "Bounding area:"), l);
    l = new TrackDataLabel(bb.south(), bb.east(), this);
    mFormLayout->addRow(QString::null, l);
}



void TrackItemDetailPage::addChildCountField(const QList<TrackDataItem *> &items, const QString &labelText)
{
    if (items.count()!=1) return;			// only for a single item

    TrackDataLabel *l = new TrackDataLabel(items.first()->childCount(), this);
    mFormLayout->addRow(labelText, l);
}






void TrackFileDetailPage::addMetadataField(const TrackDataMeta *meta, const QString &key, const QString &label)
{
    QString m = meta->data(key);
    if (!m.isEmpty())
    {
        TrackDataLabel *l;
        if (key=="time")				// special conversion for this
        {
            l = new TrackDataLabel(QDateTime::fromString(m, Qt::ISODate), this);
        }
        else						// just string value
        {
            l = new TrackDataLabel(m, this);
        }
        mFormLayout->addRow(label, l);
    }
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
        const TrackDataFile *f = dynamic_cast<const TrackDataFile *>(items.first());
        Q_ASSERT(f!=NULL);

        const TrackDataMeta *meta = f->metadata();
        if (meta!=NULL)
        {
            kDebug() << "metadata" << meta->toString();
            addMetadataField(meta, "creator", i18nc("@label:textbox", "Creator:"));
            addMetadataField(meta, "version", i18nc("@label:textbox", "GPX version:"));
            addMetadataField(meta, "time", i18nc("@label:textbox", "Save time:"));
        }
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
}






TrackSegmentDetailPage::TrackSegmentDetailPage(const QList<TrackDataItem *> items, QWidget *pnt)
    : TrackItemDetailPage(items, pnt)
{
    kDebug();
    setObjectName("TrackSegmentDetailPage");

    addChildCountField(items, i18nc("@label:textbox", "Points:"));
    addBoundingAreaField(items);
    addTimeDistanceSpeedFields(items, false);
// TODO: recording interval (time span divided by points count)

}






TrackPointDetailPage::TrackPointDetailPage(const QList<TrackDataItem *> items, QWidget *pnt)
    : TrackItemDetailPage(items, pnt)
{
    kDebug();
    setObjectName("TrackPointDetailPage");

    if (items.count()==1)				// single selection
    {
        const TrackDataPoint *p = dynamic_cast<const TrackDataPoint *>(items.first());
        Q_ASSERT(p!=NULL);

        TrackDataLabel *l = new TrackDataLabel(p->formattedPosition(), this);
        mFormLayout->addRow(i18nc("@label:textbox", "Position:"), l);

        l = new TrackDataLabel(p->time(), this);
        mFormLayout->addRow(i18nc("@label:textbox", "Time:"), l);

        double ele = p->elevation();
        if (!isnan(ele))
        {
            VariableUnitDisplay *vl = new VariableUnitDisplay(VariableUnitDisplay::Elevation, this);
            vl->setSaveId("elevation");
            vl->setValue(ele);
            mFormLayout->addRow(i18nc("@label:textbox", "Elevation:"), vl);
        }

        addSpacerField();

        QString s = p->hdop();
        if (!s.isEmpty())
        {
            l = new TrackDataLabel(s, this);
            mFormLayout->addRow(i18nc("@label:textbox", "GPS HDOP:"), l);
        }

        s = p->speed();
        if (!s.isEmpty())
        {
            double speed = s.toDouble();
            l = new TrackDataLabel(QString::number(speed, 'f', 3), this);
            mFormLayout->addRow(i18nc("@label:textbox", "GPS speed:"), l);
        }
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
        else						// selection is noncontiguous
        {
            if (items.count()==2)			// exactly two points selected
            {
                TrackDataItem *item1 = items.first();
                TrackDataItem *item2 = items.last();

                int idx1 = seg->childIndex(item1);
                int idx2 = seg->childIndex(item2);
                if (idx1>idx2) qSwap(idx1, idx2);	// order by index = time

                QList<TrackDataItem *> items2;
                for (int i = idx1; i<=idx2; ++i) items2.append(seg->childAt(i));

                addTimeDistanceSpeedFields(items2, false);
                addSpacerField();

                TrackDataPoint *pnt1 = dynamic_cast<TrackDataPoint *>(seg->childAt(idx1));
                TrackDataPoint *pnt2 = dynamic_cast<TrackDataPoint *>(seg->childAt(idx2));
                Q_ASSERT(pnt1!=NULL && pnt2!=NULL);

                VariableUnitDisplay *vl = new VariableUnitDisplay(VariableUnitDisplay::Distance, this);
                vl->setSaveId("crowflies");
                vl->setValue(pnt1->distanceTo(pnt2, true));
                mFormLayout->addRow(i18nc("@label:textbox", "Straight line distance:"), vl);

                vl = new VariableUnitDisplay(VariableUnitDisplay::Bearing, this);
                vl->setSaveId("bearing");
                vl->setValue(pnt1->bearingTo(pnt2));
                mFormLayout->addRow(i18nc("@label:textbox", "Relative bearing:"), vl);

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
}







TrackPropertiesPage *TrackDataFile::createPropertiesDetailPage(const QList<TrackDataItem *> items, QWidget *pnt)
{
    return (new TrackFileDetailPage(items, pnt));
}


TrackPropertiesPage *TrackDataTrack::createPropertiesDetailPage(const QList<TrackDataItem *> items, QWidget *pnt)
{
    return (new TrackTrackDetailPage(items, pnt));
}


TrackPropertiesPage *TrackDataSegment::createPropertiesDetailPage(const QList<TrackDataItem *> items, QWidget *pnt)
{
    return (new TrackSegmentDetailPage(items, pnt));
}


TrackPropertiesPage *TrackDataPoint::createPropertiesDetailPage(const QList<TrackDataItem *> items, QWidget *pnt)
{
    return (new TrackPointDetailPage(items, pnt));
}
