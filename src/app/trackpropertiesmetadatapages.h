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

#ifndef TRACKPROPERTIESMETADATAPAGES_H
#define TRACKPROPERTIESMETADATAPAGES_H

#include "trackpropertiespage.h"


class QTableView;

class TrackDataItem;
class MetadataModel;


class TrackItemMetadataPage : public TrackPropertiesPage
{
    Q_OBJECT

public:
    virtual ~TrackItemMetadataPage() = default;
    void refreshData() override;

protected:
    TrackItemMetadataPage(const QList<TrackDataItem *> *items, QWidget *pnt);

private:
    QTableView *mView;
};


class TrackFileMetadataPage : public TrackItemMetadataPage
{
    Q_OBJECT

public:
    TrackFileMetadataPage(const QList<TrackDataItem *> *items, QWidget *pnt);
    virtual ~TrackFileMetadataPage() = default;
};


class TrackTrackMetadataPage : public TrackItemMetadataPage
{
    Q_OBJECT

public:
    TrackTrackMetadataPage(const QList<TrackDataItem *> *items, QWidget *pnt);
    virtual ~TrackTrackMetadataPage() = default;
};


class TrackSegmentMetadataPage : public TrackItemMetadataPage
{
    Q_OBJECT

public:
    TrackSegmentMetadataPage(const QList<TrackDataItem *> *items, QWidget *pnt);
    virtual ~TrackSegmentMetadataPage() = default;
};


class TrackTrackpointMetadataPage : public TrackItemMetadataPage
{
    Q_OBJECT

public:
    TrackTrackpointMetadataPage(const QList<TrackDataItem *> *items, QWidget *pnt);
    virtual ~TrackTrackpointMetadataPage() = default;
};


class TrackFolderMetadataPage : public TrackItemMetadataPage
{
    Q_OBJECT

public:
    TrackFolderMetadataPage(const QList<TrackDataItem *> *items, QWidget *pnt);
    virtual ~TrackFolderMetadataPage() = default;
};


class TrackWaypointMetadataPage : public TrackItemMetadataPage
{
    Q_OBJECT

public:
    TrackWaypointMetadataPage(const QList<TrackDataItem *> *items, QWidget *pnt);
    virtual ~TrackWaypointMetadataPage() = default;
};


class TrackRouteMetadataPage : public TrackItemMetadataPage
{
    Q_OBJECT

public:
    TrackRouteMetadataPage(const QList<TrackDataItem *> *items, QWidget *pnt);
    virtual ~TrackRouteMetadataPage() = default;
};


class TrackRoutepointMetadataPage : public TrackItemMetadataPage
{
    Q_OBJECT

public:
    TrackRoutepointMetadataPage(const QList<TrackDataItem *> *items, QWidget *pnt);
    virtual ~TrackRoutepointMetadataPage() = default;
};

#endif							// TRACKPROPERTIESMETADATAPAGES_H
