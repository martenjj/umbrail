
#include "trackpropertiesplotpages.h"

#include <qformlayout.h>
#include <qdebug.h>

#include <klocalizedstring.h>

#include "ploteditwidget.h"
#include "trackdata.h"


TrackItemPlotPage::TrackItemPlotPage(const QList<TrackDataItem *> *items, QWidget *pnt)
    : TrackPropertiesPage(items, pnt)
{
    qDebug();
    setObjectName("TrackItemPlotPage");

    const TrackDataItem *item = items->first();
    // TODO: maybe for routepoints too
    const TrackDataWaypoint *tdw = dynamic_cast<const TrackDataWaypoint *>(item);
    Q_ASSERT(tdw!=nullptr);				// only applicable to waypoints

    mBearingEdit = new PlotEditWidget(PlotEditWidget::Bearing, this);
    mBearingEdit->setPlotData(item->metadata("bearingline").toString());
    mFormLayout->addRow(QString(), mBearingEdit);

    mRangeEdit = new PlotEditWidget(PlotEditWidget::Range, this);
    mRangeEdit->setPlotData(item->metadata("rangering").toString());
    mFormLayout->addRow(QString(), mRangeEdit);
}


QString TrackItemPlotPage::newBearingData() const
{
    if (mBearingEdit==nullptr) return ("-");		// not for this data
    return (mBearingEdit->plotData());
}


QString TrackItemPlotPage::newRangeData() const
{
    if (mRangeEdit==nullptr) return ("-");		// not for this data
    return (mRangeEdit->plotData());
}


TrackWaypointPlotPage::TrackWaypointPlotPage(const QList<TrackDataItem *> *items, QWidget *pnt)
    : TrackItemPlotPage(items, pnt)
{
    qDebug();
    setObjectName("TrackWaypointPlotPage");
}


CREATE_PROPERTIES_PAGE(Waypoint, Plot)

NULL_PROPERTIES_PAGE(File, Plot)
NULL_PROPERTIES_PAGE(Track, Plot)
NULL_PROPERTIES_PAGE(Route, Plot)
NULL_PROPERTIES_PAGE(Segment, Plot)
NULL_PROPERTIES_PAGE(Trackpoint, Plot)
NULL_PROPERTIES_PAGE(Folder, Plot)
NULL_PROPERTIES_PAGE(Routepoint, Plot)
