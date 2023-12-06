//////////////////////////////////////////////////////////////////////////
//									//
//  Project:	Umbrail - GPX track viewer and editor			//
//									//
//////////////////////////////////////////////////////////////////////////
//									//
//  Copyright (c) 2014-2023 Jonathan Marten <jjm@keelhaul.me.uk>	//
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

#include "homepointsfiltermodel.h"

#include <qdebug.h>

#include "trackdata.h"


HomePointsFilterModel::HomePointsFilterModel(QObject *pnt)
    : QSortFilterProxyModel(pnt),
      ItemIndexInterface(this)
{
    qDebug();
}


bool HomePointsFilterModel::filterAcceptsRow(int row, const QModelIndex &pnt) const
{
    const TrackDataItem *item = itemForSourceIndex(sourceModel()->index(row, 0, pnt));
    if (item==nullptr) return (false);

    // Assuming that any item presented to this model will have been filtered
    // by the source model and is a waypoint.  This is not a problem if it is
    // not the case, as other items will not normally have "flags" set anyway.
    TrackData::WaypointFlags flags = static_cast<TrackData::WaypointFlags>(item->metadata("flags").toInt());
    return (flags & TrackData::HomePoint);
}
