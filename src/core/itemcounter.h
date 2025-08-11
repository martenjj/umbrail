//////////////////////////////////////////////////////////////////////////
//									//
//  Project:	Umbrail - GPX track viewer and editor			//
//									//
//////////////////////////////////////////////////////////////////////////
//									//
//  Copyright (c) 2014-2025 Jonathan Marten <jjm@keelhaul.me.uk>	//
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

#ifndef ITEMCOUNTER_H
#define ITEMCOUNTER_H

#include <qmap.h>

#include "trackdata.h"


class ItemCounter
{
public:
    enum ItemType
    {
        File = TrackData::File,
        Track = TrackData::Track,
        Route = TrackData::Route,
        Segment = TrackData::Segment,
        Trackpoint = TrackData::Trackpoint,
        Folder = TrackData::Folder,
        Waypoint = TrackData::Waypoint,
        Routepoint = TrackData::Routepoint
    };

    enum CountFlag
    {
        NoRecurse = 0x00,
        RecurseOnce = 0x01,
        RecurseAll = 0x02,
        RecurseOnly = 0x04,
    };
    Q_DECLARE_FLAGS(CountFlags, CountFlag)

    //ItemCounter(const QList<const TrackDataItem *> *items, ItemCounter::CountFlags flags);
    ItemCounter(const QList<TrackDataItem *> *items, ItemCounter::CountFlags flags);

    int count(ItemCounter::ItemType type) const		{ return (mItemCounts.value(type)); }

private:
    void countItem(const TrackDataItem *item, ItemCounter::CountFlags flags);

private:
    QMap<ItemCounter::ItemType, int> mItemCounts;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(ItemCounter::CountFlags)

#endif							// ITEMCOUNTER_H
