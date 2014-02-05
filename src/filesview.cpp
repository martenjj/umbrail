
#include "filesview.h"

#include <qheaderview.h>
#include <qsortfilterproxymodel.h>
#include <qevent.h>
#include <qmenu.h>

#include <kdebug.h>
#include <klocale.h>
#include <kconfiggroup.h>
#include <kxmlguifactory.h>

#include "mainwindow.h"
#include "autotooltipdelegate.h"


#define GROUP_FILESVIEW		"FilesView"
#define CONFIG_COLSTATES	"ColumnStates"



FilesView::FilesView(QWidget *pnt)
    : QTreeView(pnt)
{
    kDebug();

    setObjectName("FilesView");

    mMainWindow = qobject_cast<MainWindow *>(pnt);

    setRootIsDecorated(true);
    setSortingEnabled(true);
    setAlternatingRowColors(true);
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
}


FilesView::~FilesView()
{
    kDebug() << "done";
}


void FilesView::readProperties(const KConfigGroup &grp)
{
    KConfigGroup ourGroup = grp.config()->group(GROUP_FILESVIEW);
    kDebug() << "from" << ourGroup.name();

#ifdef SORTABLE_VIEW
    QString colStates = ourGroup.readEntry(CONFIG_COLSTATES, QString());
    if (!colStates.isEmpty())
    {
        header()->restoreState(QByteArray::fromHex(colStates.toAscii()));
    }
#endif
}


void FilesView::saveProperties(KConfigGroup &grp)
{
    KConfigGroup ourGroup = grp.config()->group(GROUP_FILESVIEW);
    kDebug() << "to" << ourGroup.name();

    ourGroup.writeEntry(CONFIG_COLSTATES, header()->saveState().toHex());
}


void FilesView::selectionChanged(const QItemSelection &sel,
                                 const QItemSelection &desel)
{
    QTreeView::selectionChanged(sel, desel);		// visually update

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

            if (dynamic_cast<const TrackDataPoint *>(mSelectedItem)!=NULL) mSelectedType = TrackData::Point;
            else if (dynamic_cast<const TrackDataSegment *>(mSelectedItem)!=NULL) mSelectedType = TrackData::Segment;
            else if (dynamic_cast<const TrackDataTrack *>(mSelectedItem)!=NULL) mSelectedType = TrackData::Track;
            else if (dynamic_cast<const TrackDataFile *>(mSelectedItem)!=NULL) mSelectedType = TrackData::File;
            else mSelectedType = TrackData::None;
        }
    }

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
