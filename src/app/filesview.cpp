
#include "filesview.h"

#include <qheaderview.h>
#include <qsortfilterproxymodel.h>
#include <qevent.h>
#include <qmenu.h>
#include <qdebug.h>

#include <klocalizedstring.h>
#include <kxmlguifactory.h>

#include "mainwindow.h"
#include "autotooltipdelegate.h"
#include "settings.h"
#include "filesmodel.h"


FilesView::FilesView(QWidget *pnt)
    : QTreeView(pnt),
      ApplicationDataInterface(pnt)
{
    qDebug();

    setObjectName("FilesView");

    setRootIsDecorated(true);
    setSortingEnabled(true);
    setAlternatingRowColors(true);
    setUniformRowHeights(true);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setAllColumnsShowFocus(true);
    setDropIndicatorShown(true);

    setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

#ifdef SORTABLE_VIEW
    setHeaderHidden(false);
    header()->setStretchLastSection(true);
    header()->setDefaultSectionSize(100);
    header()->setResizeMode(QHeaderView::Interactive);
#else
    setHeaderHidden(true);
#endif
    setItemDelegate(new AutoToolTipDelegate(this));

    mSelectedCount = 0;
    mSelectedType = TrackData::None;
    mSelectedItem = nullptr;

    // The selection ID is a value which is incremented each time the selection
    // changes.  Selected items have the current selection ID stored within them,
    // and the map painter compares their stored selection ID to our current ID
    // when checking whether an item is selected.  The advantage of this over
    // just a simple selected/unselected flag within the item is that there is
    // no need to reset the flag when a item becomes unselected, the old ID
    // can just remain there.  This eliminates having to store the current selection
    // list here and run through it to reset the selection flags before emptying
    // it and refilling with the new selection, one extra loop and two list operations.
    //
    // The default selection ID used by the map painter if none is available (which
    // should never happen) is 0.  The selection ID of a newly created TrackDataItem,
    // set by its constructor, is 1.  So setting our initial selection ID just past
    // those values ensures that there will be no problem with clashes or use of
    // out-of-date IDs until of the order of 4 billion selection/deselection
    // operations (assuming 32-bit longs) have been performed.  That will keep the
    // user occupied for a while...

    mSelectionId = 2;
}


FilesView::~FilesView()
{
    qDebug() << "done";
}


void FilesView::readProperties()
{
    qDebug();

#ifdef SORTABLE_VIEW
    QString colStates = Settings::filesViewColumnStates();
    if (!colStates.isEmpty())
    {
        header()->restoreState(QByteArray::fromHex(colStates.toAscii()));
    }
#endif
}


void FilesView::saveProperties()
{
    qDebug();

    Settings::setFilesViewColumnStates(header()->saveState().toHex());
}


void FilesView::selectionChanged(const QItemSelection &sel,
                                 const QItemSelection &desel)
{
    QTreeView::selectionChanged(sel, desel);		// visually update

    ++mSelectionId;					// invalidate previous selection
    qDebug() << "selection ID is now" << mSelectionId;

    //   int numSelected		how many rows are selected
    //
    //   TrackData::Type itemType	What type if item is selected, or 'None' if
    //					nothing is selected, or 'Mixed' if the
    //					selection is mixed.
    //
    // The selection is considered to be "mixed" if the selected items are
    // of more than one type, or items of the same type but spanning boundaries.
    // The test for the latter is that not all selected items/ranges
    // have the same model parent;  however, it is still necessary to check the
    // item type because some containers can hold items of mixed type (e.g.
    // a file can contain either tracks or folders).

    QModelIndexList selIndexes = selectionModel()->selectedIndexes();
    mSelectedCount = selIndexes.count();		// get a flattened list

    if (selIndexes.isEmpty())				// quick test for no selection
    {
        mSelectedType = TrackData::None;
        mSelectedItem = nullptr;
    }
    else
    {
        mSelectedItem = FilesModel::itemForIndex(selIndexes.first());
        Q_ASSERT(mSelectedItem!=nullptr);
        mSelectedType = mSelectedItem->type();

        QModelIndex firstParent = selIndexes.first().parent();
        for (int i = 1; i<mSelectedCount; ++i)
        {
            const TrackDataItem *item = FilesModel::itemForIndex(selIndexes[i]);
            Q_ASSERT(item!=nullptr);
            if (selIndexes[i].parent()!=firstParent || item->type()!=mSelectedType)
            {
                mSelectedType = TrackData::Mixed;
                break;
            }
        }
    }

    // Mark the current selection with the current selection ID.
    for (int i = 0; i<mSelectedCount; ++i)
    {
        TrackDataItem *tdi = static_cast<FilesModel *>(model())->itemForIndex(selIndexes[i]);
        Q_ASSERT(tdi!=nullptr);
        tdi->setSelectionId(mSelectionId);

        // Selecting a point automatically sets its parent container
        // (normally a segment for trackpoints or folder for waypoints, but
        // this is not enforced) to be selected also.  Only for drawing
        // purposes, not for any user operations.
        TrackDataAbstractPoint *tdp = dynamic_cast<TrackDataAbstractPoint *>(tdi);
        if (tdp!=nullptr)				// this is a point
        {
            TrackDataItem *par = tdp->parent();
            Q_ASSERT(par!=nullptr);
            par->setSelectionId(mSelectionId);		// select its parent
        }
    }

// TODO: more selective region updating
// emit a different signal, remove connection in MainWindow

    emit updateActionState();				// actions and status
}


QList<TrackDataItem *> FilesView::selectedItems() const
{
    QModelIndexList selIndexes = selectionModel()->selectedIndexes();
    Q_ASSERT(selIndexes.count()==mSelectedCount);

    QList<TrackDataItem *> list;
    foreach (const QModelIndex idx, selIndexes)
    {
        TrackDataItem *tdi = static_cast<TrackDataItem *>(idx.internalPointer());
        list.append(tdi);
    }

    FilesModel::sortByIndexRow(&list);			// ensure in predictable order
    return (list);
}


static void getPointData(const TrackDataItem *item, QVector<const TrackDataAbstractPoint *> *points)
{
    const TrackDataAbstractPoint *tdp = dynamic_cast<const TrackDataAbstractPoint *>(item);
    if (tdp!=nullptr)					// is this a point?
    {
        if (ISNAN(tdp->latitude())) return;		// check position is valid
        if (ISNAN(tdp->longitude())) return;

        if (dynamic_cast<const TrackDataRoutepoint *>(tdp)==nullptr)
        {						// if not a route point,
            const QVariant dt = tdp->metadata("time");	// check time is valid
            if (!dt.canConvert(QMetaType::QDateTime)) return;
        }

        points->append(tdp);				// add point to list
    }
    else						// not a point, recurse for children
    {
        const int num = item->childCount();
        for (int i = 0; i<num; ++i) getPointData(item->childAt(i), points);
    }
}


QVector<const TrackDataAbstractPoint *> FilesView::selectedPoints() const
{
    QVector<const TrackDataAbstractPoint *> result;

    const QList<TrackDataItem *> items = selectedItems();
    for (int i = 0; i<items.count(); ++i) getPointData(items[i], &result);

    qDebug() << "from" << items.count() << "items got" << result.count() << "points";
    return (result);
}


void FilesView::contextMenuEvent(QContextMenuEvent *ev)
{
    QMenu *popup = static_cast<QMenu *>(
        mainWindow()->factory()->container("filesview_contextmenu",
                                           mainWindow()));
    if (popup!=nullptr) popup->exec(ev->globalPos());
}


void FilesView::slotSelectAllSiblings()
{
    // The usual "Select All" operation is not really useful in this
    // application.  So this operation, where there is a single or
    // non-mixed selection, selects all of the siblings of the selected
    // item(s) - i.e. all of those that have the same parent.
    //
    // This is a model/index operation only - it doesn't need to know
    // anything about the data!

    QModelIndexList selIndexes = selectionModel()->selectedIndexes();
    if (selIndexes.isEmpty()) return;			// get current selection

    QModelIndex firstParent = selIndexes.first().parent();
    if (!firstParent.isValid()) return;

    QModelIndex firstRowIndex = model()->index(0, 0, firstParent);
    QModelIndex lastRowIndex = model()->index(model()->rowCount(firstParent)-1, 0, firstParent);

    selectionModel()->select(QItemSelection(firstRowIndex, lastRowIndex), QItemSelectionModel::Select);
}



void FilesView::slotClickedItem(const QModelIndex &index, unsigned int flags)
{
    selectionModel()->select(QItemSelection(index, index),
                             static_cast<QItemSelectionModel::SelectionFlags>(flags));

    // If the thing clicked on is a track point, only scroll to it if
    // its parent segment is expanded.  This avoids a long list of points
    // suddenly appearing in the view for a stray map click.
    const TrackDataItem *item = static_cast<FilesModel *>(model())->itemForIndex(index);
    if (dynamic_cast<const TrackDataTrackpoint *>(item)!=nullptr)
    {
        QModelIndex pnt = index.parent();
        if (!isExpanded(pnt)) return;
    }

    scrollTo(index);
}


void FilesView::selectItem(const TrackDataItem *item, bool combine)
{
    if (item==nullptr)					// clearing selection
    {
        selectionModel()->clear();
        return;						// no more to do
    }

    QModelIndex idx = qobject_cast<FilesModel *>(model())->indexForItem(item);
    qDebug() << "index" << idx << "combine?" << combine;
    if (!idx.isValid()) return;

    if (!combine) selectionModel()->clear();
    selectionModel()->select(QItemSelection(idx, idx), QItemSelectionModel::Select);
    scrollTo(idx);					// also expand if necessary
}


void FilesView::slotCollapseAll()
{
    // Not directly calling collapseAll(), because that is not a particularly
    // useful action.  Collapse to the second level (one down from the root) only.
    expandToDepth(0);
}


void FilesView::expandItem(const QModelIndex &idx)
{
    const TrackDataItem *item = FilesModel::itemForIndex(idx);

    if (dynamic_cast<const TrackDataSegment *>(item)!=nullptr ||
        dynamic_cast<const TrackDataRoute *>(item)!=nullptr)
    {
        collapse(idx);
        return;
    }

    expand(idx);

    const int num = model()->rowCount(idx);
    for (int i = 0; i<num; ++i) expandItem(model()->index(i, 0, idx));
}


void FilesView::slotExpandAll()
{
    // Again not directly calling expandAll(), because that is also not a
    // useful operation.  Expand everything apart from segments and routes.
    expandItem(rootIndex());
}


void FilesView::setMovePointsMode(bool on)
{
    setDragDropMode(on ? QAbstractItemView::InternalMove : QAbstractItemView::NoDragDrop);
}
