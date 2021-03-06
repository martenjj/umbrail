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

#include "trackfiltermodel.h"

#include <qfont.h>

#include <qdebug.h>

#include "filesmodel.h"
#include "trackdata.h"



TrackFilterModel::TrackFilterModel(QObject *pnt)
    : QSortFilterProxyModel(pnt)
{
    mSourceItems = nullptr;
    mMode = TrackData::None;
}


void TrackFilterModel::setSource(const QList<TrackDataItem *> *items)
{
    mSourceItems = items;

    Q_ASSERT(!items->isEmpty());
    const TrackDataItem *item = items->first();

    if (dynamic_cast<const TrackDataSegment *>(item)!=nullptr) mMode = TrackData::Segment;
    else if (dynamic_cast<const TrackDataFolder *>(item)!=nullptr) mMode = TrackData::Folder;
    else if (dynamic_cast<const TrackDataWaypoint *>(item)!=nullptr) mMode = TrackData::Waypoint;
    Q_ASSERT(mMode!=TrackData::None);
}


void TrackFilterModel::setMode(TrackData::Type mode)
{
    mMode = mode;
}


bool TrackFilterModel::filterAcceptsRow(int row, const QModelIndex &pnt) const
{
    FilesModel *filesModel = qobject_cast<FilesModel *>(sourceModel());
    QModelIndex idx = filesModel->index(row, 0, pnt);
    const TrackDataItem *item = filesModel->itemForIndex(idx);

    if (dynamic_cast<const TrackDataFile *>(item)!=nullptr) return (true);
    switch (mMode)
    {
case TrackData::Segment:
        if (dynamic_cast<const TrackDataTrack *>(item)!=nullptr) return (true);
        if (dynamic_cast<const TrackDataSegment *>(item)!=nullptr) return (true);
        break;

case TrackData::Folder:
        if (dynamic_cast<const TrackDataFolder *>(item)!=nullptr) return (true);
        break;

case TrackData::Route:
        if (dynamic_cast<const TrackDataRoute *>(item)!=nullptr) return (true);
        break;

case TrackData::Waypoint:
        if (dynamic_cast<const TrackDataFolder *>(item)!=nullptr) return (true);
        if (dynamic_cast<const TrackDataWaypoint *>(item)!=nullptr) return (true);
        break;

default:
        break;
    }

    return (false);
}


Qt::ItemFlags TrackFilterModel::flags(const QModelIndex &idx) const
{
    FilesModel *filesModel = qobject_cast<FilesModel *>(sourceModel());
    const TrackDataItem *item = filesModel->itemForIndex(mapToSource(idx));

    bool sourceOk = true;
    switch (mMode)
    {
case TrackData::Segment:
        // In segment mode, only tracks which are not the immediate parent
        // of a source can be selected.  Files are enabled but cannot be
        // selected.

        if (dynamic_cast<const TrackDataFile *>(item)!=nullptr)
        {
            return (Qt::ItemIsEnabled);
        }
        else if (dynamic_cast<const TrackDataTrack *>(item)!=nullptr)
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

        if (dynamic_cast<const TrackDataFile *>(item)!=nullptr)
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
        else if (dynamic_cast<const TrackDataFolder *>(item)!=nullptr)
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

        if (dynamic_cast<const TrackDataFolder *>(item)!=nullptr)
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

        if (dynamic_cast<const TrackDataRoute *>(item)!=nullptr)
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


QVariant TrackFilterModel::data(const QModelIndex &idx, int role) const
{
    if (role!=Qt::FontRole) return (QSortFilterProxyModel::data(idx, role));

    FilesModel *filesModel = qobject_cast<FilesModel *>(sourceModel());
    TrackDataItem *item = filesModel->itemForIndex(mapToSource(idx));

    // Everything apart from source items is left unchanged
    if (mSourceItems==nullptr || !mSourceItems->contains(item)) return (QSortFilterProxyModel::data(idx, role));

    // Source items are shown in bold
    QFont f = QSortFilterProxyModel::data(idx,role).value<QFont>();
    f.setBold(true);
    return (f);
}
