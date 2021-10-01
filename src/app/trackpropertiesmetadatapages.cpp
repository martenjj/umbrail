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

#include "trackpropertiesmetadatapages.h"

#include <qtableview.h>
#include <qheaderview.h>
#include <qformlayout.h>
#include <qdebug.h>

#include <klocalizedstring.h>

#include "trackdata.h"
#include "metadatamodel.h"
#include "autotooltipdelegate.h"

//////////////////////////////////////////////////////////////////////////
//									//
//  TrackItemMetadataPage						//
//									//
//////////////////////////////////////////////////////////////////////////

TrackItemMetadataPage::TrackItemMetadataPage(const QList<TrackDataItem *> *items, QWidget *pnt)
    : TrackPropertiesPage(items, pnt)
{
    qDebug();
    setObjectName("TrackItemMetadataPage");

    mView = new QTableView(this);
    mView->setCornerButtonEnabled(false);
    mView->setWordWrap(false);
    mView->setDragEnabled(false);
    mView->setSelectionMode(QAbstractItemView::NoSelection);
    mView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    mView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    mView->setItemDelegate(new AutoToolTipDelegate(this));
    mView->horizontalHeader()->setStretchLastSection(true);

    // Don't want to use the existing form layout, because that presumably has
    // some sort of row height limit and it doesn't stretch the table view all
    // the way to the bottom.  There are no labels and nothing else to display
    // on this page, so the form layout is not needed.
    delete mFormLayout; mFormLayout = nullptr;
    QVBoxLayout *vboxLayout = new QVBoxLayout(this);
    vboxLayout->addWidget(mView);

    if (items->count()!=1) return;			// only populate for single item

    mView->resizeColumnsToContents();
    mView->resizeRowsToContents();
}


void TrackItemMetadataPage::refreshData()
{
    qDebug();
    if (mView->model()==nullptr)
    {
        qDebug() << "setting model";
        mView->setModel(dataModel());
    }
}

//////////////////////////////////////////////////////////////////////////
//									//
//  TrackFileMetadataPage						//
//									//
//////////////////////////////////////////////////////////////////////////

TrackFileMetadataPage::TrackFileMetadataPage(const QList<TrackDataItem *> *items, QWidget *pnt)
    : TrackItemMetadataPage(items, pnt)
{
    qDebug();
    setObjectName("TrackFileMetadataPage");
}

//////////////////////////////////////////////////////////////////////////
//									//
//  TrackTrackMetadataPage						//
//									//
//////////////////////////////////////////////////////////////////////////

TrackTrackMetadataPage::TrackTrackMetadataPage(const QList<TrackDataItem *> *items, QWidget *pnt)
    : TrackItemMetadataPage(items, pnt)
{
    qDebug();
    setObjectName("TrackTrackMetadataPage");
}

//////////////////////////////////////////////////////////////////////////
//									//
//  TrackSegmentMetadataPage						//
//									//
//////////////////////////////////////////////////////////////////////////

TrackSegmentMetadataPage::TrackSegmentMetadataPage(const QList<TrackDataItem *> *items, QWidget *pnt)
    : TrackItemMetadataPage(items, pnt)
{
    qDebug();
    setObjectName("TrackSegmentMetadataPage");
}

//////////////////////////////////////////////////////////////////////////
//									//
//  TrackTrackpointMetadataPage						//
//									//
//////////////////////////////////////////////////////////////////////////

TrackTrackpointMetadataPage::TrackTrackpointMetadataPage(const QList<TrackDataItem *> *items, QWidget *pnt)
    : TrackItemMetadataPage(items, pnt)
{
    qDebug();
    setObjectName("TrackPointMetadataPage");
}

//////////////////////////////////////////////////////////////////////////
//									//
//  TrackFolderMetadataPage						//
//									//
//////////////////////////////////////////////////////////////////////////

TrackFolderMetadataPage::TrackFolderMetadataPage(const QList<TrackDataItem *> *items, QWidget *pnt)
    : TrackItemMetadataPage(items, pnt)
{
    qDebug();
    setObjectName("TrackFolderMetadataPage");
}

//////////////////////////////////////////////////////////////////////////
//									//
//  TrackWaypointMetadataPage						//
//									//
//////////////////////////////////////////////////////////////////////////

TrackWaypointMetadataPage::TrackWaypointMetadataPage(const QList<TrackDataItem *> *items, QWidget *pnt)
    : TrackItemMetadataPage(items, pnt)
{
    qDebug();
    setObjectName("TrackWaypointMetadataPage");
}

//////////////////////////////////////////////////////////////////////////
//									//
//  TrackRouteMetadataPage						//
//									//
//////////////////////////////////////////////////////////////////////////

TrackRouteMetadataPage::TrackRouteMetadataPage(const QList<TrackDataItem *> *items, QWidget *pnt)
    : TrackItemMetadataPage(items, pnt)
{
    qDebug();
    setObjectName("TrackRouteMetadataPage");
}

//////////////////////////////////////////////////////////////////////////
//									//
//  TrackRoutepointMetadataPage						//
//									//
//////////////////////////////////////////////////////////////////////////

TrackRoutepointMetadataPage::TrackRoutepointMetadataPage(const QList<TrackDataItem *> *items, QWidget *pnt)
    : TrackItemMetadataPage(items, pnt)
{
    qDebug();
    setObjectName("TrackRoutepointMetadataPage");
}

//////////////////////////////////////////////////////////////////////////
//									//
//  Page creation interface						//
//									//
//////////////////////////////////////////////////////////////////////////

CREATE_PROPERTIES_PAGE(File, Metadata)
CREATE_PROPERTIES_PAGE(Track, Metadata)
CREATE_PROPERTIES_PAGE(Segment, Metadata)
CREATE_PROPERTIES_PAGE(Trackpoint, Metadata)
CREATE_PROPERTIES_PAGE(Folder, Metadata)
CREATE_PROPERTIES_PAGE(Waypoint, Metadata)
CREATE_PROPERTIES_PAGE(Route, Metadata)
CREATE_PROPERTIES_PAGE(Routepoint, Metadata)
