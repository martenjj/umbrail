//////////////////////////////////////////////////////////////////////////
//									//
//  Project:	Umbrail - GPX track viewer and editor			//
//									//
//////////////////////////////////////////////////////////////////////////
//									//
//  Copyright (c) 2014-2021 Jonathan Marten <jjm@keelhaul.me.uk>	//
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
    mBearingEdit->setEnabled(!isReadOnly());
    connect(mBearingEdit, &PlotEditWidget::dataChanged, this, &TrackWaypointPlotPage::slotPlotDataChanged);
    mFormLayout->addRow(QString(), mBearingEdit);

    mRangeEdit = new PlotEditWidget(PlotEditWidget::Range, this);
    mRangeEdit->setEnabled(!isReadOnly());
    connect(mRangeEdit, &PlotEditWidget::dataChanged, this, &TrackWaypointPlotPage::slotPlotDataChanged);
    mFormLayout->addRow(QString(), mRangeEdit);
}


void TrackWaypointPlotPage::slotPlotDataChanged()
{
    PlotEditWidget *plot = qobject_cast<PlotEditWidget *>(sender());
    Q_ASSERT(plot!=nullptr);

    const QString pd = plot->plotData();
    if (plot==mBearingEdit) dataModel()->setData(DataIndexer::index("bearingline"), pd);
    else if (plot==mRangeEdit) dataModel()->setData(DataIndexer::index("rangering"), pd);
    else Q_ASSERT(false);
}


void TrackWaypointPlotPage::refreshData()
{
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
