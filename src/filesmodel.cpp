
#include "filesmodel.h"

#include <qfont.h>
#include <qicon.h>
#include <qitemselectionmodel.h>
#include <qmimedata.h>
#include <qdebug.h>

#include <klocalizedstring.h>
#include <kcolorscheme.h>

#include "trackdata.h"






enum COLUMN
{
    COL_NAME,						// icon/description
    COL_COUNT						// how many - must be last
};


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
    // static_cast will not work here due to const'ness
    return (row==-1 ? QModelIndex() : createIndex(row, 0, (void *) tdi));
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
    return (COL_COUNT);
}


QVariant FilesModel::data(const QModelIndex &idx, int role) const
{
    const TrackDataItem *tdi = itemForIndex(idx);

    switch (role)
    {
case Qt::DisplayRole:
        switch (idx.column())
        {
case COL_NAME:     return (tdi->name());
        }
        break;

case Qt::DecorationRole:
        switch (idx.column())
        {
case COL_NAME:     return (tdi->icon());
        }
        break;

case Qt::ForegroundRole:
        switch (idx.column())
        {
case COL_NAME:
            QVariant status = tdi->metadata("status");
            if (!status.isNull())
            {
                TrackData::WaypointStatus s = static_cast<TrackData::WaypointStatus>(status.toInt());
                KColorScheme sch(QPalette::Normal);
                if (s==TrackData::StatusTodo) return (sch.foreground(KColorScheme::NegativeText));
                if (s==TrackData::StatusDone) return (sch.foreground(KColorScheme::PositiveText));
                if (s==TrackData::StatusQuestion) return (sch.foreground(KColorScheme::NeutralText));
                if (s==TrackData::StatusUnwanted) return (sch.foreground(KColorScheme::NeutralText));
            }
        }
        break;

case Qt::UserRole:					// data for sorting
        switch (idx.column())
        {
default:           return (data(idx, Qt::DisplayRole));
        }
        break;

case Qt::ToolTipRole:
        switch (idx.column())
        {
case COL_NAME:
            {
                QString tip;

                if (dynamic_cast<const TrackDataFolder *>(tdi)!=nullptr) tip = i18np("Folder with %1 item", "Folder with %1 items", tdi->childCount());
                else if (dynamic_cast<const TrackDataTrack *>(tdi)!=nullptr) tip = i18np("Track with %1 segment", "Track with %1 segments", tdi->childCount());
                else if (dynamic_cast<const TrackDataSegment *>(tdi)!=nullptr) tip = i18np("Segment with %1 point", "Segment with %1 points", tdi->childCount());
                else if (dynamic_cast<const TrackDataRoute *>(tdi)!=nullptr) tip = i18np("Route with %1 point", "Route with %1 points", tdi->childCount());
                else
                {
                    const TrackDataFile *tdf = dynamic_cast<const TrackDataFile *>(tdi);
                    if (tdf!=nullptr) tip = i18np("File %2 with %1 item", "File %2 with %1 items", tdf->childCount(), tdf->fileName().toDisplayString());
                    else
                    {
                        const TrackDataAbstractPoint *tdp = dynamic_cast<const TrackDataAbstractPoint *>(tdi);
                        if (dynamic_cast<const TrackDataTrackpoint *>(tdp)!=nullptr) tip = i18n("Point at %1, elevation %2", tdp->formattedTime(true), tdp->formattedElevation());
                        else if (dynamic_cast<const TrackDataWaypoint *>(tdp)!=nullptr)
                        {
                            const QString wptStatus = TrackData::formattedWaypointStatus(
                                static_cast<TrackData::WaypointStatus>(tdi->metadata("status").toInt()), true);
                            if (!wptStatus.isEmpty()) tip = i18n("Waypoint at %1, elevation %2 (%3)", tdp->formattedTime(true), tdp->formattedElevation(), wptStatus);
                            else tip = i18n("Waypoint at %1, elevation %2", tdp->formattedTime(true), tdp->formattedElevation());
                        }
                        else if (dynamic_cast<const TrackDataRoutepoint *>(tdp)!=nullptr) tip = i18n("Routepoint, elevation %1", tdp->formattedElevation());
                    }
                }

                if (!tip.isEmpty())
                {
                    QString desc = tdi->metadata("desc").toString();
                    if (!desc.isEmpty())
                    {
                        desc.replace('\n', ";&nbsp;");
                        tip = i18n("<div style=\"white-space:nowrap\">%1</div><div>\"%2\"</div>", tip, desc);
                    }

                    return (tip);
                }

            }
            break;
        }
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
case COL_NAME:		return (i18n("Name"));
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
    emit layoutAboutToBeChanged();
}


void FilesModel::endLayoutChange()
{
    emit layoutChanged();
}


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

static const QString itemMimeType = QStringLiteral("application/x-navtracks-internal");


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
