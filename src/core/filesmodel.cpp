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

#include "filesmodel.h"

#include <qfont.h>
#include <qicon.h>
#include <qitemselectionmodel.h>
#include <qmimedata.h>
#include <qdebug.h>
#include <qfontdatabase.h>

#include <klocalizedstring.h>
#include <kcolorscheme.h>

#include "pointicon.h"
#include "trackdata.h"


#define SIZE_ICON		QSize(16, 16)
#define SIZE_HINT		QSize(18, 18)


FilesModel::FilesModel(QObject *pnt)
    : QAbstractItemModel(pnt)
{
    qDebug();
    mRootFileItem = nullptr;

    // Drag and drop depends on being able to encode and serialise a pointer.
    Q_ASSERT(sizeof(TrackDataItem *)<=sizeof(qulonglong));
}


FilesModel::~FilesModel()
{
    delete mRootFileItem;
    qDebug() << "done";
}


/* static */ TrackDataItem *FilesModel::itemForIndex(const QModelIndex &idx)
{
    return (static_cast<TrackDataItem *>(idx.internalPointer()));
}


QModelIndex FilesModel::indexForItem(const TrackDataItem *tdi) const
{
    Q_ASSERT(tdi!=nullptr);
    const TrackDataItem *pnt = tdi->parent();
    int row = (pnt==nullptr ? 0 : pnt->childIndex(tdi));
    // The two casts are necessary, the only alternative
    // is an old-style cast.
    return (row==-1 ? QModelIndex() : createIndex(row, 0, const_cast<void *>(static_cast<const void *>(tdi))));
}


QModelIndex FilesModel::index(int row, int col, const QModelIndex &pnt) const
{
    const TrackDataItem *tdi = itemForIndex(pnt);
    if (tdi==nullptr)
    {
        if (isEmpty()) return (QModelIndex());
        if (row>0) return (QModelIndex());
        return (createIndex(row, col, mRootFileItem));
    }

    if (row>=tdi->childCount())				// only during initialisation
    {							// without SORTABLE_VIEW
        qDebug() << "requested index for nonexistent row" << row << "of" << tdi->childCount();
        return (QModelIndex());
    }

    return (createIndex(row, col, tdi->childAt(row)));
}


QModelIndex FilesModel::parent(const QModelIndex &idx) const
{
    const TrackDataItem *tdi = itemForIndex(idx);
    if (tdi->parent()==nullptr) return (QModelIndex());
    return (indexForItem(tdi->parent()));
}


int FilesModel::rowCount(const QModelIndex &pnt) const
{
   if (pnt==QModelIndex()) return (!isEmpty() ? 1 : 0);
   const TrackDataItem *tdi = itemForIndex(pnt);
   Q_ASSERT(tdi!=nullptr);
   return (tdi->childCount());
}


int FilesModel::columnCount(const QModelIndex &pnt) const
{
    return (ColumnCount);
}


static QVariant formatCoordinates(const TrackDataItem *item)
{
    const TrackDataAbstractPoint *tdp = dynamic_cast<const TrackDataAbstractPoint *>(item);
    if (tdp==nullptr) return (QVariant());
    return (tdp->formattedPosition());
}


static QVariant formatAddress(const TrackDataItem *item)
{
    const TrackDataWaypoint *tdw = dynamic_cast<const TrackDataWaypoint *>(item);
    if (tdw==nullptr) return (QVariant());
    return (tdw->formattedAddress().join(", "));
}


QVariant FilesModel::data(const QModelIndex &idx, int role) const
{
    const TrackDataItem *item = itemForIndex(idx);
    const int col = idx.column();

    switch (role)
    {
case Qt::DisplayRole:
        switch (col)
        {
case ColumnName:     return (item->name());
case ColumnOrigin:   return (item->metadata("origin").toStringList().join(", "));
case ColumnCoords:   return (formatCoordinates(item));
case ColumnAddress:  return (formatAddress(item));
case ColumnCats:     return (item->metadata("category").toStringList().join(", "));
        }
        break;

case Qt::DecorationRole:
        switch (col)
        {
case ColumnName:
case ColumnSym:      return (item->icon()->icon());
        }
        break;

case Qt::FontRole:
        if (col==ColumnCoords) return (QFontDatabase::systemFont(QFontDatabase::FixedFont));
        break;

case Qt::ForegroundRole:
        if (col==ColumnName)
        {
            const TrackData::WaypointFlags flags = static_cast<TrackData::WaypointFlags>(item->metadata("flags").toInt());
            const KColorScheme sch;
            if (flags & TrackData::NewlyImported) return (sch.foreground(KColorScheme::PositiveText));
            if (flags & TrackData::NoExport) return (sch.foreground(KColorScheme::NegativeText));
        }
        break;

case Qt::ToolTipRole:
        switch (col)
        {
case ColumnSym:      return (item->icon()->name());
case ColumnOrigin:   return (item->metadata("origin").toStringList().join("<br/>"));
        }
        break;

case Qt::UserRole:					// data for sorting
        if (col==ColumnSym) return (item->icon()->name());
        else return (data(idx, Qt::DisplayRole));
        break;

case Qt::SizeHintRole:
        if (col==ColumnSym) return (SIZE_HINT);
        break;
    }

    return (QVariant());
}


QVariant FilesModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role!=Qt::DisplayRole) return (QVariant());
    if (orientation!=Qt::Horizontal) return (QVariant());

    switch (section)
    {
case ColumnName:		return (i18n("Name"));
case ColumnSym:		return (i18n("Sym"));
case ColumnOrigin:	return (i18n("Origin"));
case ColumnCoords:	return (i18n("Lat/Long"));
case ColumnAddress:	return (i18n("Address"));
case ColumnCats:	return (i18n("Categories"));
default:		return (QVariant());
    }
}


TrackDataFile *FilesModel::takeRootFileItem()
{
    TrackDataFile *root = mRootFileItem;
    Q_ASSERT(root!=nullptr);
    beginResetModel();
    mRootFileItem = nullptr;
    endResetModel();
    qDebug() << "removing root" << root->name();
    return (root);
}


void FilesModel::setRootFileItem(TrackDataFile *root)
{
    Q_ASSERT(mRootFileItem==nullptr);
    beginResetModel();
    qDebug() << "setting root" << root->name();
    mRootFileItem = root;
    endResetModel();
}


void FilesModel::changedItem(const TrackDataItem *item)
{
    QModelIndex idx = indexForItem(item);
    emit dataChanged(idx, idx);
}


void FilesModel::startLayoutChange()
{
    // Simply emitting layoutAboutToBeChanged() here and layoutChanged()
    // below does not seem to go far enough, it causes an assert within
    // KDescendantsProxyModel when deleting items.
    beginResetModel();
}


void FilesModel::endLayoutChange()
{
    endResetModel();
}


// TODO: move to eventual destination FilesView
// model should not need to handle UI apart from D&D
void FilesModel::clickedPoint(const TrackDataAbstractPoint *tdp, Qt::KeyboardModifiers mods)
{
    QItemSelectionModel::SelectionFlags selFlags;
    if (mods==Qt::NoModifier) selFlags = QItemSelectionModel::ClearAndSelect;
    else if (mods==Qt::ControlModifier) selFlags = QItemSelectionModel::Toggle;
    else return;

    qDebug() << "click for" << indexForItem(tdp) << "flags" << selFlags;
    emit clickedItem(indexForItem(tdp), static_cast<unsigned int>(selFlags));
}

//////////////////////////////////////////////////////////////////////////
//									//
//  Drag and Drop							//
//									//
//////////////////////////////////////////////////////////////////////////

static const QString itemMimeType = QStringLiteral("application/x-umbrail-dnd-internal");


Qt::ItemFlags FilesModel::flags(const QModelIndex &idx) const
{
    Qt::ItemFlags f = QAbstractItemModel::flags(idx);

    // Any item can be dragged, apart from the top level root item.
    if (idx.parent()!=QModelIndex()) f |= Qt::ItemIsDragEnabled;
    // These flags will say that dragged items can be dropped anywhere.
    // The checks in canDropMimeData() will enforce our restrictions.
    f |= Qt::ItemIsDropEnabled;

    return (f);
}


Qt::DropActions FilesModel::supportedDropActions() const
{
    return (Qt::MoveAction);
}


QStringList FilesModel::mimeTypes() const
{
    return (QStringList() << itemMimeType);
}


QMimeData *FilesModel::mimeData(const QModelIndexList &idxs) const
{
    if (idxs.isEmpty()) return (nullptr);
    qDebug() << "starting to drag" << idxs.count() << "items";

    QByteArray encoded;
    for (const QModelIndex &idx : qAsConst(idxs))
    {
        const TrackDataItem *item = itemForIndex(idx);
        if (item==nullptr) continue;
        qDebug() << "  " << item->name();

        // inspired by https://stackoverflow.com/questions/24345681/
        qulonglong ptrval = reinterpret_cast<qulonglong>(item);
        encoded.append(QByteArray::number(ptrval, 16));
        encoded.append(',');
    }

    //qDebug() << "encoded size" << encoded.size() << "=" << encoded;
    QMimeData *data = new QMimeData;
    data->setData(itemMimeType, encoded);
    return (data);
}


static bool lessThanByIndexRow(const TrackDataItem *a, const TrackDataItem *b)
{
    const TrackDataItem *parentA = a->parent();
    Q_ASSERT(parentA!=NULL);
    const TrackDataItem *parentB = b->parent();
    Q_ASSERT(parentB!=NULL);

    int indexA = parentA->childIndex(a);
    int indexB = parentB->childIndex(b);
    return (indexA<indexB);
}


/* static */ void FilesModel::sortByIndexRow(QList<TrackDataItem *> *list)
{
    std::stable_sort(list->begin(), list->end(), &lessThanByIndexRow);
}


static QList<TrackDataItem *> decodeItemData(const QMimeData *data)
{
    QList<TrackDataItem *> list;

    const QByteArray encoded = data->data(itemMimeType);
    //qDebug() << "decoding" << encoded;
    const QList<QByteArray> ptrs = encoded.split(',');

    for (const QByteArray &b : qAsConst(ptrs))
    {
        if (b.isEmpty()) continue;
        qulonglong ptrval = b.toULongLong(nullptr, 16);
        TrackDataItem *item = reinterpret_cast<TrackDataItem *>(ptrval);
        //qDebug() << "  ->" << item->name();
        list.append(item);
    }

    FilesModel::sortByIndexRow(&list);			// ensure in predictable order
    return (list);
}


bool FilesModel::dropMimeDataInternal(bool doit, const QMimeData *data, int row, const QModelIndex &pnt)
{
    // Drops are not accepted over an item, only between them.
    //if (row==-1) return (false);

    qDebug() << "doit" << doit << "row" << row << "pnt" << pnt;

    // Get the parent item of the drop location.
    TrackDataItem *ontoParent = itemForIndex(pnt);
    if (ontoParent==nullptr) return (false);
    qDebug() << "  onto parent" << ontoParent->name();

    // To avoid confusion as to where the drop will end up, if the drop is
    // over an item then it must be an empty container.  The checks below
    // will ensure that the container is of the appropriate type.
    //
    // It is necessary to enable the drop by doing this test, instead of simply
    // checking that 'row' is not -1 as before, because Qt will not otherwise
    // allow a drop into an item with no children.
    if (row==-1)
    {
        if (ontoParent->childCount()>0) return (false);
    }

    // Get the first drag source item.  The GUI will ensure that move mode
    // cannot be entered unless the selection is consistent;  that is, any
    // additionally selected items will have the same parent.
    QList<TrackDataItem *> sourceItems = decodeItemData(data);
    if (sourceItems.isEmpty()) return (false);
    const TrackDataItem *sourceItem = sourceItems.first();
    qDebug() << "  src item" << sourceItem->name();

    // See what sort of item is being dragged, and then whether it
    // is allowed to be dropped at the destination location.
    const bool toTopLevel = (dynamic_cast<const TrackDataFile *>(ontoParent)!=nullptr);
    const bool toFolder = (dynamic_cast<const TrackDataFolder *>(ontoParent)!=nullptr);

    // A folder can only be dropped at the top level or inside another folder.
    if (dynamic_cast<const TrackDataFolder *>(sourceItem)!=nullptr)
    {
        if (!toTopLevel && !toFolder) return (false);
    }

    // A track or route can only be dropped at the top level.
    else if (dynamic_cast<const TrackDataTrack *>(sourceItem)!=nullptr ||
             dynamic_cast<const TrackDataRoute *>(sourceItem)!=nullptr)
    {
        if (!toTopLevel) return (false);
    }

    // A segment is not allowed to be dragged.  This should be enforced by the
    // "Move Mode" action not being enabled in MainWindow::slotUpdateActionState().
    else if (dynamic_cast<const TrackDataSegment *>(sourceItem)!=nullptr)
    {
        return (false);
    }

    // A track point is not allowed to be dragged.  But unlike a segment as
    // above, "Move Mode" is allowed for track points so that they can be
    // moved on the map.
    //
    // The reason for not allowing segments or track points to be moved around,
    // which would reorder them in time, is that the data analysis operations
    // (Profile, Stop Detect etc) assume that track points will always be
    // in time order within the file.  Allowing tracks to be moved around
    // breaks this, but it is unusual to want to perform those operations
    // over multiple tracks and so it is allowed for presentation purposes.
    else if (dynamic_cast<const TrackDataTrackpoint *>(sourceItem)!=nullptr)
    {
        return (false);
    }

    // A waypoint can only be dropped into a folder.
    else if (dynamic_cast<const TrackDataWaypoint *>(sourceItem)!=nullptr)
    {
        if (!toFolder) return (false);
    }

    // A route point can only be dropped into a route.
    else if (dynamic_cast<const TrackDataRoutepoint *>(sourceItem)!=nullptr)
    {
        if (dynamic_cast<const TrackDataRoute *>(ontoParent)==nullptr) return (false);
    }

    // If the drag and drop is within the same parent container, check that
    // an item is not being dropped onto itself.
    if (sourceItem->parent()==ontoParent)
    {
        // Dropping an item onto or after itself or any of the selected items
        // is confusing, and would be a no-op anyway unless the selection is
        // not contiguous.  It is not allowed, even in this unusual case,
        // to avoid a pointless no-op in the undo history.
        for (const TrackDataItem *item : qAsConst(sourceItems))
        {
            const int sourceRow = ontoParent->childIndex(item);
            if (row==sourceRow || row==(sourceRow+1)) return (false);
        }
    }

    if (doit) emit dragDropItems(sourceItems, ontoParent, row);
    return (true);
}


bool FilesModel::dropMimeData(const QMimeData *data, Qt::DropAction act,
                              int row, int col, const QModelIndex &pnt)
{
    return (dropMimeDataInternal(true, data, row, pnt));
}


bool FilesModel::canDropMimeData(const QMimeData *data, Qt::DropAction act,
                                 int row, int col, const QModelIndex &pnt) const
{
    // This is a const function, but it needs to call dropMimeDataInternal().
    // That function cannot be const because when called from dropMimeData()
    // it needs to be able to emit a (non-const) signal.  Doing this should
    // be safe against undefined behaviour because dropMimeDataInternal() does
    // not actually modify any members when called with 'doit' set to false.
    return (const_cast<FilesModel *>(this)->dropMimeDataInternal(false, data, row, pnt));
}
