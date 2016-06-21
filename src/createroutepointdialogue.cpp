
#include "createroutepointdialogue.h"

#include <qformlayout.h>
#include <qtreeview.h>
#include <qlineedit.h>
#include <qdebug.h>

#include <klocalizedstring.h>
// #include <kglobal.h>
// #include <kconfiggroup.h>

#include "filescontroller.h"
#include "filesmodel.h"
#include "filesview.h"
#include "trackfiltermodel.h"
#include "trackdata.h"
#include "latlongwidget.h"


CreateRoutepointDialogue::CreateRoutepointDialogue(FilesController *fc, QWidget *pnt)
    : DialogBase(pnt)
{
    setObjectName("CreateRoutepointDialogue");

    setModal(true);
    setWindowTitle(i18nc("@title:window", "Create Route Point"));
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

    mRouteList = new QTreeView(this);
    mRouteList->setRootIsDecorated(true);
    mRouteList->setSortingEnabled(true);
    mRouteList->setAlternatingRowColors(true);
    mRouteList->setSelectionBehavior(QAbstractItemView::SelectRows);
    mRouteList->setSelectionMode(QAbstractItemView::SingleSelection);
    mRouteList->setAllColumnsShowFocus(true);
    mRouteList->setUniformRowHeights(true);
    mRouteList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    mRouteList->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    mRouteList->setHeaderHidden(true);
 
    TrackFilterModel *trackModel = new TrackFilterModel(this);
    trackModel->setSourceModel(fc->model());
    trackModel->setMode(TrackData::Route);
    mRouteList->setModel(trackModel);
    mRouteList->expandToDepth(9);

    QList<TrackDataItem *> items = fc->view()->selectedItems();
    if (items.count()==1)
    {
        const TrackDataItem *item = items.first();
        mRouteList->setCurrentIndex(trackModel->mapFromSource(fc->model()->indexForItem(item)));
    }

    connect(mRouteList->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &,const QItemSelection &)),
            SLOT(slotSetButtonStates()));

    fl->addRow(i18nc("@title:row", "Route:"), mRouteList);

    setMainWidget(w);
    slotSetButtonStates();
    mNameEdit->setFocus();

    setMinimumSize(360, 280);
//     KConfigGroup grp = KGlobal::config()->group(objectName());
//     restoreDialogSize(grp);
}


// CreateRoutepointDialogue::~CreateRoutepointDialogue()
// {
// //     KConfigGroup grp = KGlobal::config()->group(objectName());
// //     saveDialogSize(grp);
// }


void CreateRoutepointDialogue::setSourcePoint(const TrackDataAbstractPoint *point)
{
    Q_ASSERT(point!=NULL);
    qDebug() << point->name();
    mLatLongEdit->setLatLong(point->latitude(), point->longitude());
    slotSetButtonStates();
}


void CreateRoutepointDialogue::setSourceLatLong(double lat, double lon)
{
    qDebug() << lat << lon;
    mLatLongEdit->setLatLong(lat, lon);
    slotSetButtonStates();
}


void CreateRoutepointDialogue::setDestinationRoute(const TrackDataRoute *route)
{
    Q_ASSERT(route!=NULL);
    qDebug() << route->name();
    // nothing to do, it will be selected automatically via the source model
}


QString CreateRoutepointDialogue::routepointName() const
{
    return (mNameEdit->text());
}


void CreateRoutepointDialogue::routepointPosition(qreal *latp, qreal *lonp)
{
    *latp = mLatLongEdit->latitude();
    *lonp = mLatLongEdit->longitude();
}


TrackDataRoute *CreateRoutepointDialogue::selectedRoute() const
{
    QModelIndexList selIndexes = mRouteList->selectionModel()->selectedIndexes();
    if (selIndexes.count()!=1) return (NULL);

    TrackFilterModel *trackModel = qobject_cast<TrackFilterModel *>(mRouteList->model());
    FilesModel *filesModel = qobject_cast<FilesModel *>(trackModel->sourceModel());
    TrackDataItem *item = filesModel->itemForIndex(trackModel->mapToSource(selIndexes.first()));
    return (dynamic_cast<TrackDataRoute *>(item));
}


void CreateRoutepointDialogue::slotSetButtonStates()
{
    setButtonEnabled(QDialogButtonBox::Ok, !mNameEdit->text().isEmpty() &&
                                           mLatLongEdit->hasAcceptableInput() &&
                                           selectedRoute()!=NULL);
}
