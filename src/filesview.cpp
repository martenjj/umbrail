
#include "filesview.h"

#include <qheaderview.h>
#include <qsortfilterproxymodel.h>
#include <qevent.h>
#include <qmenu.h>

#include <kdebug.h>
#include <klocale.h>
#include <kxmlguifactory.h>

#include "mainwindow.h"
#include "autotooltipdelegate.h"
#include "settings.h"
#include "filesmodel.h"


FilesView::FilesView(QWidget *pnt)
    : QTreeView(pnt)
{
    kDebug();

    setObjectName("FilesView");

    mMainWindow = qobject_cast<MainWindow *>(pnt);

    setRootIsDecorated(true);
    setSortingEnabled(true);
    setAlternatingRowColors(true);
    setUniformRowHeights(true);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setAllColumnsShowFocus(true);

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
    mSelectedItem = NULL;

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
    kDebug() << "done";
}


void FilesView::readProperties()
{
    kDebug();

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
    kDebug();

    Settings::setFilesViewColumnStates(header()->saveState().toHex());
}


void FilesView::selectionChanged(const QItemSelection &sel,
                                 const QItemSelection &desel)
{
    QTreeView::selectionChanged(sel, desel);		// visually update

    ++mSelectionId;					// invalidate previous selection
    kDebug() << "selection ID is now" << mSelectionId;

    //   int numSelected		how many rows are selected
    //
    //   TrackData::Type itemType	What type if item is selected, or 'None' if
    //					nothing is selected, or 'Mixed' if the
    //					selection is mixed.
    //
    // The selection is considered to be "mixed" if the selected items are
    // of more than one type, or items of the same type but spanning boundaries.
    // In practice, the test for this is that not all selected items/ranges
    // have the same model parent.

    QModelIndexList selIndexes = selectionModel()->selectedIndexes();
    mSelectedCount = selIndexes.count();		// get a flattened list

    if (selIndexes.isEmpty())				// quick test for no selection
    {
        mSelectedType = TrackData::None;
        mSelectedItem = NULL;
    }
    else
    {
        bool isMixed = false;
        QModelIndex firstParent = selIndexes.first().parent();
        for (int i = 1; i<mSelectedCount; ++i)
        {
            if (selIndexes[i].parent()!=firstParent)
            {
                isMixed = true;
                break;
            }
        }

        if (isMixed) mSelectedType = TrackData::Mixed;
        else
        {
            mSelectedItem = static_cast<const TrackDataItem *>(selIndexes.first().internalPointer());
            Q_ASSERT(mSelectedItem!=NULL);

            // TODO: mSelectedItem->itemType() a virtual of TrackDataItem
            if (dynamic_cast<const TrackDataTrackpoint *>(mSelectedItem)!=NULL) mSelectedType = TrackData::Point;
            else if (dynamic_cast<const TrackDataSegment *>(mSelectedItem)!=NULL) mSelectedType = TrackData::Segment;
            else if (dynamic_cast<const TrackDataTrack *>(mSelectedItem)!=NULL) mSelectedType = TrackData::Track;
            else if (dynamic_cast<const TrackDataFile *>(mSelectedItem)!=NULL) mSelectedType = TrackData::File;
            else if (dynamic_cast<const TrackDataFolder *>(mSelectedItem)!=NULL) mSelectedType = TrackData::Folder;
            else if (dynamic_cast<const TrackDataWaypoint *>(mSelectedItem)!=NULL) mSelectedType = TrackData::Waypoint;
            else mSelectedType = TrackData::None;
        }
    }

    // Mark the current selection with the current selection ID.
    for (int i = 0; i<mSelectedCount; ++i)
    {
        TrackDataItem *tdi = static_cast<FilesModel *>(model())->itemForIndex(selIndexes[i]);
        Q_ASSERT(tdi!=NULL);
        tdi->setSelectionId(mSelectionId);

        // Selecting a point automatically sets its parent container
        // (normally a segment for trackpoints or folder for waypoints, but
        // this is not enforced) to be selected also.  Only for drawing
        // purposes, not for any user operations.
        TrackDataAbstractPoint *tdp = dynamic_cast<TrackDataAbstractPoint *>(tdi);
        if (tdp!=NULL)					// this is a point
        {
            TrackDataItem *par = tdp->parent();
            Q_ASSERT(par!=NULL);
            par->setSelectionId(mSelectionId);		// select its parent
        }
    }

// TODO: more selective region updating
// emit a different signal, remove connection in MainWindow

    emit updateActionState();				// actions and status
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



QList<TrackDataItem *> FilesView::selectedItems() const
{
    QModelIndexList selIndexes = selectionModel()->selectedIndexes();
    Q_ASSERT(selIndexes.count()==mSelectedCount);

    QList<TrackDataItem *> list;
    foreach(const QModelIndex idx, selIndexes)
    {
        TrackDataItem *tdi = static_cast<TrackDataItem *>(idx.internalPointer());
        list.append(tdi);
    }

    qStableSort(list.begin(), list.end(), &lessThanByIndexRow);
    return (list);
}






//// Map the selection in the proxy model to the base model.
//// This contains one model index for each selected item (all
//// columns), so consider only those for the first column.
//// Return the row index in the base model of each.
//FilesView::RowList FilesView::selectedRows() const
//{
//    FilesView::RowList result;
//
//    // Could also do this with
//    //
//    //   QModelIndexList indexes = mProxyModel->mapSelectionToSource(
//    //                               selectionModel()->selection()).indexes();
//    //
//    // but, since we have to iterate over the returned list anyway,
//    // it should be more efficient to filter the list for column 0
//    // first and then do the model mapping.
//
//    QAbstractProxyModel *pm = static_cast<QAbstractProxyModel *>(model());
//    QModelIndexList indexes = selectedIndexes();
//    for (QModelIndexList::const_iterator it = indexes.constBegin();
//         it!=indexes.constEnd(); ++it)
//    {
//        QModelIndex idx = (*it);
//        if (idx.column()==0) result.append(pm->mapToSource(idx).row());
//    }
//
//    return (result);
//}
//
//
//void FilesView::selectRows(const QList<int> &rows)
//{
//    kDebug() << rows;
//
//    selectionModel()->reset();
//    if (!rows.isEmpty())
//    {
//        QAbstractProxyModel *pm = static_cast<QAbstractProxyModel *>(model());
//        QAbstractItemModel *sm = pm->sourceModel();
//
//        for (int i = 0; i<rows.count(); ++i)
//        {
//            kDebug() << "selecting row" << rows[i];
//            selectionModel()->select(pm->mapFromSource(sm->index(rows[i], 0)),
//                                     QItemSelectionModel::Select|QItemSelectionModel::Rows);
//        }
//
//        scrollTo(pm->mapFromSource(sm->index(rows.first(), 0)));
//    }
//}
//
//
//void FilesView::selectRows(int fromRow, int toRow)
//{
//    kDebug() << fromRow << "-" << toRow;
//
//    QAbstractProxyModel *pm = static_cast<QAbstractProxyModel *>(model());
//    QAbstractItemModel *sm = pm->sourceModel();
//
//    selectionModel()->reset();
//    for (int i = fromRow; i<=toRow; ++i)
//    {
//        kDebug() << "selecting row" << i;
//        selectionModel()->select(pm->mapFromSource(sm->index(i, 0)),
//                                 QItemSelectionModel::Select|QItemSelectionModel::Rows);
//    }
//
//    scrollTo(pm->mapFromSource(sm->index(fromRow, 0)));
//}


void FilesView::contextMenuEvent(QContextMenuEvent *ev)
{
    QMenu *popup = static_cast<QMenu *>(
        mainWindow()->factory()->container("filesview_contextmenu",
                                           mainWindow()));
    if (popup!=NULL) popup->exec(ev->globalPos());
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
    scrollTo(index);
}


void FilesView::selectItem(const TrackDataItem *item, bool combine)
{
    QModelIndex idx = qobject_cast<FilesModel *>(model())->indexForItem(item);
    kDebug() << "index" << idx << "combine?" << combine;
    if (!idx.isValid()) return;

    if (!combine) selectionModel()->clear();
    selectionModel()->select(QItemSelection(idx, idx), QItemSelectionModel::Select);
    scrollTo(idx);					// also expand if necessary
}
