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

#ifndef WAYPOINTSFILTERMODEL_H
#define WAYPOINTSFILTERMODEL_H
 
#include <qsortfilterproxymodel.h>


class TrackDataItem;


/**
 * @short A model to filter a list of track data items and accept only waypoints.
 */
class WaypointsFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    explicit WaypointsFilterModel(QObject *pnt = nullptr);
    virtual ~WaypointsFilterModel() = default;

    virtual bool filterAcceptsRow(int row, const QModelIndex &pnt) const override;

    TrackDataItem *itemForIndex(const QModelIndex &idx) const;
};
 
#endif							// WAYPOINTSFILTERMODEL_H
