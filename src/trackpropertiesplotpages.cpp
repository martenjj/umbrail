
#include "trackpropertiesplotpages.h"

#include <qformlayout.h>
#include <qdebug.h>

#include <klocalizedstring.h>

#include "ploteditwidget.h"
#include "trackdata.h"
#include "metadatamodel.h"
#include "dataindexer.h"

//////////////////////////////////////////////////////////////////////////
//									//
//  TrackItemPlotPage							//
//									//
//////////////////////////////////////////////////////////////////////////

TrackItemPlotPage::TrackItemPlotPage(const QList<TrackDataItem *> *items, QWidget *pnt)
    : TrackPropertiesPage(items, pnt)
{
    qDebug();
    setObjectName("TrackItemPlotPage");
}

//////////////////////////////////////////////////////////////////////////
//									//
//  TrackWaypointPlotPage						//
//									//
//////////////////////////////////////////////////////////////////////////

TrackWaypointPlotPage::TrackWaypointPlotPage(const QList<TrackDataItem *> *items, QWidget *pnt)
    : TrackItemPlotPage(items, pnt)
{
    qDebug();
    setObjectName("TrackWaypointPlotPage");

    // TODO: maybe for routepoints too

    mBearingEdit = new PlotEditWidget(PlotEditWidget::Bearing, this);
    connect(mBearingEdit, &PlotEditWidget::dataChanged, this, &TrackWaypointPlotPage::slotPlotDataChanged);
    mFormLayout->addRow(QString(), mBearingEdit);

    mRangeEdit = new PlotEditWidget(PlotEditWidget::Range, this);
    connect(mRangeEdit, &PlotEditWidget::dataChanged, this, &TrackWaypointPlotPage::slotPlotDataChanged);
    mFormLayout->addRow(QString(), mRangeEdit);

}


void TrackWaypointPlotPage::slotPlotDataChanged()
{
    PlotEditWidget *plot = qobject_cast<PlotEditWidget *>(sender());
    Q_ASSERT(plot!=nullptr);

    const QString pd = plot->plotData();
    if (plot==mBearingEdit) dataModel()->setData(DataIndexer::self()->index("bearingline"), pd);
    else if (plot==mRangeEdit) dataModel()->setData(DataIndexer::self()->index("rangering"), pd);
    else Q_ASSERT(false);
}


void TrackWaypointPlotPage::refreshData()
{
    qDebug();
    mBearingEdit->setPlotData(dataModel()->data("bearingline").toString());
    mRangeEdit->setPlotData(dataModel()->data("rangering").toString());
}

//////////////////////////////////////////////////////////////////////////
//									//
//  Page creation interface						//
//									//
//////////////////////////////////////////////////////////////////////////


CREATE_PROPERTIES_PAGE(Waypoint, Plot)

NULL_PROPERTIES_PAGE(File, Plot)
NULL_PROPERTIES_PAGE(Track, Plot)
NULL_PROPERTIES_PAGE(Segment, Plot)
NULL_PROPERTIES_PAGE(Trackpoint, Plot)
NULL_PROPERTIES_PAGE(Folder, Plot)
NULL_PROPERTIES_PAGE(Route, Plot)
NULL_PROPERTIES_PAGE(Routepoint, Plot)
