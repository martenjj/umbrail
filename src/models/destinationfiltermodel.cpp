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

#include "destinationfiltermodel.h"

#include <qfont.h>


DestinationFilterModel::DestinationFilterModel(QObject *pnt)
    : QSortFilterProxyModel(pnt),
      ItemIndexInterface(this)
{
    mSourceItems = nullptr;
    mMode = TrackData::None;
}


void DestinationFilterModel::setSource(const QList<TrackDataItem *> *items)
{
    mSourceItems = items;

    Q_ASSERT(!items->isEmpty());
    const TrackDataItem *item = items->first();

    if (IS(TrackDataSegment, item)) mMode = TrackData::Segment;
    else if (IS(TrackDataFolder, item)) mMode = TrackData::Folder;
    else if (IS(TrackDataWaypoint, item)) mMode = TrackData::Waypoint;
    Q_ASSERT(mMode!=TrackData::None);
}


void DestinationFilterModel::setMode(TrackData::Type mode)
{
    mMode = mode;
}


bool DestinationFilterModel::filterAcceptsRow(int row, const QModelIndex &pnt) const
{
    const TrackDataItem *item = itemForSourceIndex(sourceModel()->index(row, 0, pnt));

    if (IS(TrackDataFile, item)) return (true);
    switch (mMode)
    {
case TrackData::Segment:
        if (IS(TrackDataTrack, item)) return (true);
        if (IS(TrackDataSegment, item)) return (true);
        break;

case TrackData::Folder:
        if (IS(TrackDataFolder, item)) return (true);
        break;

case TrackData::Route:
        if (IS(TrackDataRoute, item)) return (true);
        break;

case TrackData::Waypoint:
        if (IS(TrackDataFolder, item)) return (true);
        if (IS(TrackDataWaypoint, item)) return (true);
        break;

default:
        break;
    }

    return (false);
}


Qt::ItemFlags DestinationFilterModel::flags(const QModelIndex &idx) const
{
    const TrackDataItem *item = itemForIndex(idx);

    bool sourceOk = true;
    switch (mMode)
    {
case TrackData::Segment:
        // In segment mode, only tracks which are not the immediate parent
        // of a source can be selected.  Files are enabled but cannot be
        // selected.
        if (IS(TrackDataFile, item)) return (Qt::ItemIsEnabled);
        else if (IS(TrackDataTrack, item))
        {
            if (mSourceItems!=nullptr)
            {
                for (int i = 0; i<mSourceItems->count(); ++i)
                {
                    const TrackDataItem *srcItem = mSourceItems->at(i);
                    if (srcItem->parent()==item)
                    {
                        sourceOk = false;
                        break;
                    }
                }
            }

            if (sourceOk) return (Qt::ItemIsSelectable|Qt::ItemIsEnabled);
        }
        break;

case TrackData::Folder:
        // In folder mode, top level files can be selected unless they
        // are the immediate parent of a source folder.  Folders can
        // be selected unless they are a source folder, or an immediate
        // parent or any child of one.
        if (IS(TrackDataFile, item))
        {
            if (mSourceItems!=nullptr)
            {
                for (int i = 0; i<mSourceItems->count(); ++i)
                {
                    const TrackDataItem *srcItem = mSourceItems->at(i);
                    if (srcItem->parent()==item)
                    {
                        sourceOk = false;
                        break;
                    }
                }
            }
        }
        else if (IS(TrackDataFolder, item))
        {
            if (mSourceItems!=nullptr)
            {
                for (int i = 0; i<mSourceItems->count(); ++i)
                {
                    const TrackDataItem *srcItem = mSourceItems->at(i);
                    if (srcItem==item) sourceOk = false;
                    else if (srcItem->parent()==item) sourceOk = false;
                    else
                    {
                        const TrackDataItem *pnt = item->parent();
                        while (pnt!=nullptr)
                        {
                            if (pnt==srcItem)
                            {
                                sourceOk = false;
                                break;
                            }
                            pnt = pnt->parent();
                        }
                    }
                }
            }
        }
        else sourceOk = false;

        if (sourceOk) return (Qt::ItemIsSelectable|Qt::ItemIsEnabled);
        break;

case TrackData::Waypoint:
        // In waypoint mode, any folder can be selected unless it
        // it the immediate parent of a source waypoint.
        if (IS(TrackDataFolder, item))
        {
            if (mSourceItems!=nullptr)
            {
                for (int i = 0; i<mSourceItems->count(); ++i)
                {
                    const TrackDataItem *srcItem = mSourceItems->at(i);
                    if (srcItem->parent()==item)
                    {
                        sourceOk = false;
                        break;
                    }
                }
            }
        }
        else sourceOk = false;

        if (sourceOk) return (Qt::ItemIsSelectable|Qt::ItemIsEnabled);
        break;

case TrackData::Route:
        // In route mode, any route can be selected unless it
        // it the immediate parent of a source point.
        if (IS(TrackDataRoute, item))
        {
            if (mSourceItems!=nullptr)
            {
                for (int i = 0; i<mSourceItems->count(); ++i)
                {
                    const TrackDataItem *srcItem = mSourceItems->at(i);
                    if (srcItem->parent()==item)
                    {
                        sourceOk = false;
                        break;
                    }
                }
            }
        }
        else sourceOk = false;

        if (sourceOk) return (Qt::ItemIsSelectable|Qt::ItemIsEnabled);
        break;

default:
        break;
    }

    return (Qt::NoItemFlags);
}


QVariant DestinationFilterModel::data(const QModelIndex &idx, int role) const
{
    if (role!=Qt::FontRole) return (QSortFilterProxyModel::data(idx, role));

    TrackDataItem *item = itemForIndex(idx);

    // Everything apart from source items is left unchanged
    if (mSourceItems==nullptr || !mSourceItems->contains(item)) return (QSortFilterProxyModel::data(idx, role));

    // Source items are shown in bold
    QFont f = QSortFilterProxyModel::data(idx, role).value<QFont>();
    f.setBold(true);
    return (f);
}
