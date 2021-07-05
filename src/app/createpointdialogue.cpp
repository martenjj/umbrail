
#include "createpointdialogue.h"

#include <qformlayout.h>
#include <qtreeview.h>
#include <qlineedit.h>
#include <qdebug.h>

#include <klocalizedstring.h>

#include "filescontroller.h"
#include "filesmodel.h"
#include "filesview.h"
#include "trackfiltermodel.h"
#include "trackdata.h"
#include "latlongwidget.h"


// from https://stackoverflow.com/questions/39153835/how-to-loop-over-qabstractitemview-indexes
static int findSelectableItems(const QModelIndex &index, const QAbstractItemModel *model, QModelIndex *theItem)
{
    int result = 0;

    if (index.isValid())
    {
        const Qt::ItemFlags flags = model->flags(index);
        if (flags & Qt::ItemIsSelectable)
        {
            ++result;
            if (theItem!=nullptr && !theItem->isValid()) *theItem = index;
        }
    }

    if (model->hasChildren(index))
    {
        auto rows = model->rowCount(index);
        auto cols = model->columnCount(index);
        for (int i = 0; i < rows; ++i)
        {
            for (int j = 0; j < cols; ++j)
            {
                result += findSelectableItems(model->index(i, j, index), model, theItem);
            }
        }
    }

    return (result);
}


CreatePointDialogue::CreatePointDialogue(FilesController *fc, bool routeMode, QWidget *pnt)
    : DialogBase(pnt)
{
    setObjectName("CreatePointDialogue");

    setModal(true);
    setWindowTitle(routeMode ? i18nc("@title:window", "Create Route Point") :
                               i18nc("@title:window", "Create Waypoint"));
    setButtons(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
    setButtonEnabled(QDialogButtonBox::Ok, false);
    setButtonText(QDialogButtonBox::Ok, i18nc("@action:button", "Create"));

    QWidget *w = new QWidget(this);
    QFormLayout *fl = new QFormLayout(w);

    mNameEdit = new QLineEdit(w);
    mNameEdit->setPlaceholderText(i18n("(Default name)"));
    connect(mNameEdit, SIGNAL(textChanged(const QString &)), SLOT(slotSetButtonStates()));
    fl->addRow(i18nc("@title:row", "Name:"), mNameEdit);

    mLatLongEdit = new LatLongWidget(w);
    connect(mLatLongEdit, SIGNAL(positionValid(bool)), SLOT(slotSetButtonStates()));
    fl->addRow(i18nc("@title:row", "Position:"), mLatLongEdit);

    mContainerList = new QTreeView(this);
    mContainerList->setRootIsDecorated(true);
    mContainerList->setSortingEnabled(true);
    mContainerList->setAlternatingRowColors(true);
    mContainerList->setSelectionBehavior(QAbstractItemView::SelectRows);
    mContainerList->setSelectionMode(QAbstractItemView::SingleSelection);
    mContainerList->setAllColumnsShowFocus(true);
    mContainerList->setUniformRowHeights(true);
    mContainerList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    mContainerList->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    mContainerList->setHeaderHidden(true);
 
    TrackFilterModel *trackModel = new TrackFilterModel(this);
    trackModel->setSourceModel(fc->model());
    trackModel->setMode(routeMode ? TrackData::Route : TrackData::Waypoint);
    mContainerList->setModel(trackModel);
    mContainerList->expandToDepth(9);

    // Try to preselect a destination for the new item.
    // Firstly, if there is only a single selectable destination,
    // then use that.
    mCanCreate = true;					// assume so initially
    QModelIndex theItem;
    const int selectableItems = findSelectableItems(mContainerList->rootIndex(), trackModel, &theItem);
    if (selectableItems==1 && theItem.isValid())
    {
        mContainerList->setCurrentIndex(theItem);
    }
    // If there are no selectable destinations, then nothing can
    // be created.
    else if (selectableItems==0)
    {
        mCanCreate = false;
    }
    else
    {
        // If there is more than one selectable destination, then if
        // the tree's selected item is valid then preselect that.
        QList<TrackDataItem *> items = fc->view()->selectedItems();
        if (items.count()==1)
        {
            const TrackDataItem *item = items.first();
            mContainerList->setCurrentIndex(trackModel->mapFromSource(fc->model()->indexForItem(item)));
        }
    }

    connect(mContainerList->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &,const QItemSelection &)),
            SLOT(slotSetButtonStates()));

    fl->addRow(routeMode ? i18nc("@title:row", "Route:") : i18nc("@title:row", "Folder:"), mContainerList);

    setMainWidget(w);
    slotSetButtonStates();
    mNameEdit->setFocus();

    setMinimumSize(360, 280);
}


void CreatePointDialogue::setSourcePoint(const TrackDataAbstractPoint *point)
{
    Q_ASSERT(point!=nullptr);
    qDebug() << point->name();
    mLatLongEdit->setLatLong(point->latitude(), point->longitude());
    slotSetButtonStates();
}


void CreatePointDialogue::setSourceLatLong(double lat, double lon)
{
    qDebug() << lat << lon;
    mLatLongEdit->setLatLong(lat, lon);
    slotSetButtonStates();
}


void CreatePointDialogue::setDestinationContainer(const TrackDataItem *item)
{
    Q_ASSERT(item!=nullptr);
    qDebug() << item->name();
    // nothing to do, it will be selected automatically via the source model
}


QString CreatePointDialogue::pointName() const
{
    return (mNameEdit->text());
}


void CreatePointDialogue::pointPosition(qreal *latp, qreal *lonp)
{
    *latp = mLatLongEdit->latitude();
    *lonp = mLatLongEdit->longitude();
}


TrackDataItem *CreatePointDialogue::selectedContainer() const
{
    QModelIndexList selIndexes = mContainerList->selectionModel()->selectedIndexes();
    if (selIndexes.count()!=1) return (nullptr);

    TrackFilterModel *trackModel = qobject_cast<TrackFilterModel *>(mContainerList->model());
    FilesModel *filesModel = qobject_cast<FilesModel *>(trackModel->sourceModel());
    TrackDataItem *item = filesModel->itemForIndex(trackModel->mapToSource(selIndexes.first()));
    return (item);
}


void CreatePointDialogue::slotSetButtonStates()
{
    setButtonEnabled(QDialogButtonBox::Ok, mLatLongEdit->hasAcceptableInput() &&
                                           selectedContainer()!=nullptr);
}
