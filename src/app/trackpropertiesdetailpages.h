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

#ifndef TRACKPROPERTIESDETAILPAGES_H
#define TRACKPROPERTIESDETAILPAGES_H

#include <qmap.h>

#include "trackpropertiespage.h"


class QLabel;

class TrackDataItem;
class TrackDataLabel;
class VariableUnitDisplay;


class TrackItemDetailPage : public TrackPropertiesPage
{
    Q_OBJECT

public:
    virtual ~TrackItemDetailPage() = default;

    void refreshData() override;

    enum DisplayItem
    {
        DisplayPosition = 0x001,			// position or bounding area
        DisplayTime = 0x002,				// time or time span
        DisplayElevation = 0x004,			// elevation or difference
        DisplayTravelDistance = 0x008,			// travel distance
        DisplayAverageSpeed = 0x010,			// average speed
        DisplayRelativeBearing = 0x020,			// relative bearing
        DisplayStraightLine = 0x040,			// straight line distance
        DisplayTravelTime = 0x080,			// total travel time
        DisplayRouteLength = 0x100,			// total route length
    };
    Q_DECLARE_FLAGS(DisplayItems, DisplayItem)

protected:
    TrackItemDetailPage(const QList<TrackDataItem *> *items, QWidget *pnt);

    void addDisplayFields(const QList<TrackDataItem *> *items, DisplayItems disp);
    void addChildCountField(const QList<TrackDataItem *> *items, const QString &labelText);
    void addMetadataField(const QByteArray &key, const QString &label);

private:
    QLabel *mPositionLabel;
    TrackDataLabel *mTimeLabel;
    TrackDataLabel *mTimeStartLabel;
    TrackDataLabel *mTimeEndLabel;
    VariableUnitDisplay *mElevationLabel;
    QMap<int,QWidget *> mMetadataMap;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(TrackItemDetailPage::DisplayItems)


class TrackFileDetailPage : public TrackItemDetailPage
{
    Q_OBJECT

public:
    TrackFileDetailPage(const QList<TrackDataItem *> *items, QWidget *pnt);
    virtual ~TrackFileDetailPage() = default;
};


class TrackTrackDetailPage : public TrackItemDetailPage
{
    Q_OBJECT

public:
    TrackTrackDetailPage(const QList<TrackDataItem *> *items, QWidget *pnt);
    virtual ~TrackTrackDetailPage() = default;
};


class TrackSegmentDetailPage : public TrackItemDetailPage
{
    Q_OBJECT

public:
    TrackSegmentDetailPage(const QList<TrackDataItem *> *items, QWidget *pnt);
    virtual ~TrackSegmentDetailPage() = default;
};


class TrackTrackpointDetailPage : public TrackItemDetailPage
{
    Q_OBJECT

public:
    TrackTrackpointDetailPage(const QList<TrackDataItem *> *items, QWidget *pnt);
    virtual ~TrackTrackpointDetailPage() = default;
};


class TrackFolderDetailPage : public TrackItemDetailPage
{
    Q_OBJECT

public:
    TrackFolderDetailPage(const QList<TrackDataItem *> *items, QWidget *pnt);
    virtual ~TrackFolderDetailPage() = default;
    void refreshData() override;

private:
    QLabel *mPathDisplay;
    QString mFolderParent;
};


class TrackWaypointDetailPage : public TrackItemDetailPage
{
    Q_OBJECT

public:
    TrackWaypointDetailPage(const QList<TrackDataItem *> *items, QWidget *pnt);
    virtual ~TrackWaypointDetailPage() = default;
};


class TrackRouteDetailPage : public TrackItemDetailPage
{
    Q_OBJECT

public:
    TrackRouteDetailPage(const QList<TrackDataItem *> *items, QWidget *pnt);
    virtual ~TrackRouteDetailPage() = default;
};


class TrackRoutepointDetailPage : public TrackItemDetailPage
{
    Q_OBJECT

public:
    TrackRoutepointDetailPage(const QList<TrackDataItem *> *items, QWidget *pnt);
    virtual ~TrackRoutepointDetailPage() = default;
};

#endif							// TRACKPROPERTIESDETAILPAGES_H
