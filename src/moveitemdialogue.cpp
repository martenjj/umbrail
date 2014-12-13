
#include "moveitemdialogue.h"

#include <qgridlayout.h>
#include <qlabel.h>
#include <qtreeview.h>
#include <qitemselectionmodel.h>

#include <kdebug.h>
#include <klocale.h>
#include <kglobal.h>
#include <kconfiggroup.h>

#include "filescontroller.h"
#include "filesmodel.h"
#include "trackfiltermodel.h"
#include "trackdata.h"


MoveItemDialogue::MoveItemDialogue(FilesController *fc, QWidget *pnt)
    : KDialog(pnt)
{
    setObjectName("MoveItemDialogue");

    setModal(true);
    setCaption(i18nc("@title:window", "Move Item"));
    setButtons(KDialog::Ok|KDialog::Cancel);
    showButtonSeparator(true);
    enableButtonOk(false);
    setButtonText(KDialog::Ok, i18nc("@action:button", "Move"));

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
    trackModel->setSourceModel(fc->model());
    mTrackList->setModel(trackModel);

    connect(mTrackList->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &,const QItemSelection &)),
            SLOT(slotSelectionChanged(const QItemSelection &,const QItemSelection &)));

    setMainWidget(mTrackList);

    setMinimumSize(360, 280);
    KConfigGroup grp = KGlobal::config()->group(objectName());
    restoreDialogSize(grp);
}


MoveItemDialogue::~MoveItemDialogue()
{
    KConfigGroup grp = KGlobal::config()->group(objectName());
    saveDialogSize(grp);

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

    TrackFilterModel *trackModel = qobject_cast<TrackFilterModel *>(mTrackList->model());
    delete trackModel;
}


void MoveItemDialogue::setSource(const QList<TrackDataItem *> *items)
{
    TrackFilterModel *trackModel = qobject_cast<TrackFilterModel *>(mTrackList->model());
    trackModel->setSource(items);
    mTrackList->expandToDepth(9);
}


TrackDataItem *MoveItemDialogue::selectedDestination() const
{
    QModelIndexList selIndexes = mTrackList->selectionModel()->selectedIndexes();
    if (selIndexes.count()!=1) return (NULL);

    TrackFilterModel *trackModel = qobject_cast<TrackFilterModel *>(mTrackList->model());
    FilesModel *filesModel = qobject_cast<FilesModel *>(trackModel->sourceModel());
    TrackDataItem *item = filesModel->itemForIndex(trackModel->mapToSource(selIndexes.first()));
    return (item);
}


void MoveItemDialogue::slotSelectionChanged(const QItemSelection &sel, const QItemSelection &desel)
{
    enableButtonOk(mTrackList->selectionModel()->selectedIndexes().count()==1);
}
