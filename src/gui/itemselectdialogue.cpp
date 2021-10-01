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

#include "itemselectdialogue.h"

#include <qtreeview.h>
#include <qtimer.h>

#include <klocalizedstring.h>

#include "filesview.h"
#include "filesmodel.h"
#include "trackfiltermodel.h"
#include "trackdata.h"


ItemSelectDialogue::ItemSelectDialogue(QWidget *pnt)
    : DialogBase(pnt),
      ApplicationDataInterface(pnt)
{
    setObjectName("ItemSelectDialogue");

    setModal(true);

    mTrackList = new QTreeView(this);
    mTrackList->setRootIsDecorated(true);
    mTrackList->setSortingEnabled(true);
    mTrackList->setAlternatingRowColors(true);
    mTrackList->setSelectionBehavior(QAbstractItemView::SelectRows);
    mTrackList->setSelectionMode(QAbstractItemView::SingleSelection);
    mTrackList->setAllColumnsShowFocus(true);
    mTrackList->setUniformRowHeights(true);
    mTrackList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    mTrackList->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    mTrackList->setHeaderHidden(true);

    TrackFilterModel *trackModel = new TrackFilterModel(this);
    trackModel->setSourceModel(filesView()->model());
    mTrackList->setModel(trackModel);

    connect(mTrackList->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &ItemSelectDialogue::slotSelectionChanged);

    setMainWidget(mTrackList);

    QTimer::singleShot(0, this, &ItemSelectDialogue::slotExpandTree);

    setMinimumSize(360, 280);
}


ItemSelectDialogue::~ItemSelectDialogue()
{
    // Explicitly delete the filter model to avoid a crash:
    //
    // ASSERT failure in QPersistentModelIndex::~QPersistentModelIndex:
    //	 "persistent model indexes corrupted", file kernel/qabstractitemmodel.cpp, line 544
    //
    // QAbstractItemModelPrivate::removePersistentIndexData at kernel/qabstractitemmodel.cpp:543
    // QPersistentModelIndexData::destroy at kernel/qabstractitemmodel.cpp:83
    // QPersistentModelIndex::~QPersistentModelIndex at kernel/qabstractitemmodel.cpp:155
    // ~QHashNode at qhash.h:216
    // QHash<QPersistentModelIndex, QHashDummyValue>::deleteNode2 at qhash.h:521
    // QHashData::free_helper at tools/qhash.cpp:275
    // freeData at qhash.h:570
    // ~QHash at qhash.h:283
    // ~QSet at qset.h:54
    // ~QTreeViewPrivate at qtreeview_p.h:96
    // QTreeViewPrivate::~QTreeViewPrivate at qtreeview_p.h:96
    // cleanup at qscopedpointer.h:62
    // ~QScopedPointer at qscopedpointer.h:100
    // QObject::~QObject at kernel/qobject.cpp:816
    // QWidget::~QWidget at kernel/qwidget.cpp:1554
    // QFrame::~QFrame at widgets/qframe.cpp:240
    // QAbstractScrollArea::~QAbstractScrollArea at widgets/qabstractscrollarea.cpp:521
    // QAbstractItemView::~QAbstractItemView at itemviews/qabstractitemview.cpp:598
    // QTreeView::~QTreeView at itemviews/qtreeview.cpp:207
    // QTreeView::~QTreeView at itemviews/qtreeview.cpp:209
    // QObjectPrivate::deleteChildren at kernel/qobject.cpp
    // QWidget::~QWidget at kernel/qwidget.cpp
    // QDialog::~QDialog at dialogs/qdialog.cpp:320
    // KDialog::~KDialog at kdelibs/kdeui/dialogs/kdialog.cpp:201
    // MoveSegmentDialogue::~MoveSegmentDialogue at movesegmentdialogue.cpp:63

    delete trackModel();
}


void ItemSelectDialogue::setSelectedItem(const TrackDataItem *item)
{
    if (item==nullptr)					// clear selection only
    {
        mTrackList->selectionModel()->select(QModelIndex(), QItemSelectionModel::Clear);
        return;
    }

    QModelIndex idx = trackModel()->mapFromSource(filesModel()->indexForItem(item));
    mTrackList->expand(idx.parent());
    mTrackList->selectionModel()->select(idx, QItemSelectionModel::ClearAndSelect);
}


TrackDataItem *ItemSelectDialogue::selectedItem() const
{
    QModelIndexList selIndexes = mTrackList->selectionModel()->selectedIndexes();
    if (selIndexes.count()!=1) return (nullptr);

    TrackDataItem *item = filesModel()->itemForIndex(trackModel()->mapToSource(selIndexes.first()));
    return (item);
}


void ItemSelectDialogue::slotSelectionChanged(const QItemSelection &sel, const QItemSelection &desel)
{
    setButtonEnabled(QDialogButtonBox::Ok, mTrackList->selectionModel()->selectedIndexes().count()==1);
    emit selectionChanged();
}


TrackFilterModel *ItemSelectDialogue::trackModel() const
{
    return (qobject_cast<TrackFilterModel *>(mTrackList->model()));
}


FilesModel *ItemSelectDialogue::filesModel() const
{
    return (qobject_cast<FilesModel *>(trackModel()->sourceModel()));
}


void ItemSelectDialogue::slotExpandTree()
{
    mTrackList->expandToDepth(9);
}
