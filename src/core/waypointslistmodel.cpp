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

#include "waypointslistmodel.h"

#include <qdebug.h>

#include "trackdata.h"
#include "fileslistmodel.h"


WaypointsListModel::WaypointsListModel(QObject *pnt)
    : QSortFilterProxyModel(pnt)
{
    qDebug();
}


// TODO: to avoid all models in the chain having to implement itemForIndex()
// in order to be able to get the TrackDataItem corresponding to a model
// index, FilesModel could make the pointer available via data() with a unique
// role.  May still have to implement indexForItem() though.

TrackDataItem *WaypointsListModel::itemForIndex(const QModelIndex &idx) const
{
    const FilesListModel *flm = qobject_cast<const FilesListModel *>(sourceModel());
    Q_ASSERT(flm!=nullptr);
    return (flm->itemForIndex(mapToSource(idx)));
}


bool WaypointsListModel::filterAcceptsRow(int row, const QModelIndex &pnt) const
{
    // The 'row' and 'pnt' refer to the source model.  There is therefore
    // no need to use mapToSource() here.

    const FilesListModel *flm = qobject_cast<const FilesListModel *>(sourceModel());
    Q_ASSERT(flm!=nullptr);
    const TrackDataItem *item = flm->itemForIndex(flm->index(row, 0, pnt));
    if (item==nullptr) return false;

    const TrackDataWaypoint *tdw = dynamic_cast<const TrackDataWaypoint *>(item);
    return (tdw!=nullptr);
}
