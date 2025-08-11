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

#include "itemcounter.h"


// ItemCounter::ItemCounter(const QList<const TrackDataItem *> *items, ItemCounter::CountFlags flags)
// {
    // for (const TrackDataItem *item : std::as_const(*items)) countItem(item, flags);
// }


ItemCounter::ItemCounter(const QList<TrackDataItem *> *items, ItemCounter::CountFlags flags)
{
    for (const TrackDataItem *item : std::as_const(*items)) countItem(item, flags);
}


/* private */ void ItemCounter::countItem(const TrackDataItem *item, ItemCounter::CountFlags flags)
{
    if (IS(TrackDataFile, item)) ++mItemCounts[ItemCounter::File];
    else if (IS(TrackDataTrack, item)) ++mItemCounts[ItemCounter::Track];
    else if (IS(TrackDataRoute, item)) ++mItemCounts[ItemCounter::Route];
    else if (IS(TrackDataSegment, item)) ++mItemCounts[ItemCounter::Segment];
    else if (IS(TrackDataTrackpoint, item)) ++mItemCounts[ItemCounter::Trackpoint];
    else if (IS(TrackDataFolder, item)) ++mItemCounts[ItemCounter::Folder];
    else if (IS(TrackDataWaypoint, item)) ++mItemCounts[ItemCounter::Waypoint];
    else if (IS(TrackDataRoutepoint, item)) ++mItemCounts[ItemCounter::Routepoint];

    if (flags & (ItemCounter::RecurseOnce|ItemCounter::RecurseAll))
    {
        const TrackDataContainer *tdc = AS(TrackDataContainer, item);
        if (tdc!=nullptr)
        {
            // If the RecurseOnce flag is set, turn it off so that
            // the iterations over these child items do not recurse.
            flags &= ~ItemCounter::RecurseOnce;
            for (int j = 0; j<tdc->childCount(); ++j) countItem(tdc->childAt(j), flags);
        }
    }
}
