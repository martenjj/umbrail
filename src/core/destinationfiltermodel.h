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

#ifndef DESTINATIONFILTERMODEL_H
#define DESTINATIONFILTERMODEL_H
 
#include <qsortfilterproxymodel.h>
#include "itemindexinterface.h"

#include "trackdata.h"


class DestinationFilterModel : public QSortFilterProxyModel, public ItemIndexInterface
{
    Q_OBJECT

public:
    DestinationFilterModel(QObject *pnt = nullptr);
    virtual ~DestinationFilterModel() = default;

    virtual bool filterAcceptsRow(int row, const QModelIndex &pnt) const override;
    virtual Qt::ItemFlags flags(const QModelIndex &idx) const override;

    virtual QVariant data(const QModelIndex &idx, int role = Qt::DisplayRole) const override;
    void setSource(const QList<TrackDataItem *> *items);
    void setMode(TrackData::Type mode);

private:
    const QList<TrackDataItem *> *mSourceItems;
    TrackData::Type mMode;
};

 
#endif							// DESTINATIONFILTERMODEL_H
