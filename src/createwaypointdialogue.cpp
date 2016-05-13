
#include "createwaypointdialogue.h"

#include <qformlayout.h>
#include <qtreeview.h>
#include <qlineedit.h>

#include <kdebug.h>
#include <klocalizedstring.h>

#include "filescontroller.h"
#include "filesmodel.h"
#include "filesview.h"
#include "trackfiltermodel.h"
#include "trackdata.h"
#include "latlongwidget.h"


CreateWaypointDialogue::CreateWaypointDialogue(FilesController *fc, QWidget *pnt)
    : DialogBase(pnt)
{
    setObjectName("CreateWaypointDialogue");

    setModal(true);
    setWindowTitle(i18nc("@title:window", "Create Waypoint"));
    setButtons(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
    setButtonEnabled(QDialogButtonBox::Ok, false);
    setButtonText(QDialogButtonBox::Ok, i18nc("@action:button", "Create"));

    QWidget *w = new QWidget(this);
    QFormLayout *fl = new QFormLayout(w);

    mNameEdit = new QLineEdit(w);
    connect(mNameEdit, SIGNAL(textChanged(const QString &)), SLOT(slotSetButtonStates()));
    fl->addRow(i18nc("@title:row", "Name:"), mNameEdit);

    mLatLongEdit = new LatLongWidget(w);
    connect(mLatLongEdit, SIGNAL(positionValid(bool)), SLOT(slotSetButtonStates()));
    fl->addRow(i18nc("@title:row", "Position:"), mLatLongEdit);

    mFolderList = new QTreeView(this);
    mFolderList->setRootIsDecorated(true);
    mFolderList->setSortingEnabled(true);
    mFolderList->setAlternatingRowColors(true);
    mFolderList->setSelectionBehavior(QAbstractItemView::SelectRows);
    mFolderList->setSelectionMode(QAbstractItemView::SingleSelection);
    mFolderList->setAllColumnsShowFocus(true);
    mFolderList->setUniformRowHeights(true);
    mFolderList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    mFolderList->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    mFolderList->setHeaderHidden(true);
 
    TrackFilterModel *trackModel = new TrackFilterModel(this);
    trackModel->setSourceModel(fc->model());
    trackModel->setMode(TrackData::Folder);
    mFolderList->setModel(trackModel);
    mFolderList->expandToDepth(9);

    QList<TrackDataItem *> items = fc->view()->selectedItems();
    if (items.count()==1)
    {
        const TrackDataItem *item = items.first();
        mFolderList->setCurrentIndex(trackModel->mapFromSource(fc->model()->indexForItem(item)));
    }

    connect(mFolderList->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &,const QItemSelection &)),
            SLOT(slotSetButtonStates()));

    fl->addRow(i18nc("@title:row", "Folder:"), mFolderList);

    setMainWidget(w);
    slotSetButtonStates();
    mNameEdit->setFocus();

    setMinimumSize(360, 280);
}


void CreateWaypointDialogue::setSourcePoint(const TrackDataAbstractPoint *point)
{
    Q_ASSERT(point!=NULL);
    kDebug() << point->name();
    mLatLongEdit->setLatLong(point->latitude(), point->longitude());
    slotSetButtonStates();
}


void CreateWaypointDialogue::setSourceLatLong(double lat, double lon)
{
    kDebug() << lat << lon;
    mLatLongEdit->setLatLong(lat, lon);
    slotSetButtonStates();
}


void CreateWaypointDialogue::setDestinationFolder(const TrackDataFolder *folder)
{
    Q_ASSERT(folder!=NULL);
    kDebug() << folder->name();
    // nothing to do, it will be selected automatically via the source model
}


QString CreateWaypointDialogue::waypointName() const
{
    return (mNameEdit->text());
}


void CreateWaypointDialogue::waypointPosition(qreal *latp, qreal *lonp)
{
    *latp = mLatLongEdit->latitude();
    *lonp = mLatLongEdit->longitude();
}


TrackDataFolder *CreateWaypointDialogue::selectedFolder() const
{
    QModelIndexList selIndexes = mFolderList->selectionModel()->selectedIndexes();
    if (selIndexes.count()!=1) return (NULL);

    TrackFilterModel *trackModel = qobject_cast<TrackFilterModel *>(mFolderList->model());
    FilesModel *filesModel = qobject_cast<FilesModel *>(trackModel->sourceModel());
    TrackDataItem *item = filesModel->itemForIndex(trackModel->mapToSource(selIndexes.first()));
    return (dynamic_cast<TrackDataFolder *>(item));
}


void CreateWaypointDialogue::slotSetButtonStates()
{
    setButtonEnabled(QDialogButtonBox::Ok, !mNameEdit->text().isEmpty() &&
                                           mLatLongEdit->hasAcceptableInput() &&
                                           selectedFolder()!=NULL);
}
