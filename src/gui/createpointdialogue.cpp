//////////////////////////////////////////////////////////////////////////
//									//
//  Project:	Umbrail - GPX track viewer and editor			//
//									//
//////////////////////////////////////////////////////////////////////////
//									//
//  Copyright (c) 2014-2022 Jonathan Marten <jjm@keelhaul.me.uk>	//
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

#include "createpointdialogue.h"

#include <qformlayout.h>
#include <qtreeview.h>
#include <qlineedit.h>
#include <qdebug.h>

#include <klocalizedstring.h>

#include "filesview.h"
#include "destinationfiltermodel.h"
#include "trackdata.h"
#include "latlongwidget.h"
#include "itemindexinterface.h"


// from https://stackoverflow.com/questions/39153835/how-to-loop-over-qabstractitemview-indexes
static int findSelectableItems(const QModelIndex &index, const QAbstractItemModel *mod, QModelIndex *theItem)
{
    int result = 0;

    if (index.isValid())
    {
        const Qt::ItemFlags flags = mod->flags(index);
        if (flags & Qt::ItemIsSelectable)
        {
            ++result;
            if (theItem!=nullptr && !theItem->isValid()) *theItem = index;
        }
    }

    if (mod->hasChildren(index))
    {
        auto rows = mod->rowCount(index);
        auto cols = mod->columnCount(index);
        for (int i = 0; i < rows; ++i)
        {
            for (int j = 0; j < cols; ++j)
            {
                result += findSelectableItems(mod->index(i, j, index), mod, theItem);
            }
        }
    }

    return (result);
}


CreatePointDialogue::CreatePointDialogue(bool routeMode, QWidget *pnt)
    : DialogBase(pnt),
      ApplicationDataInterface(pnt)
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
    connect(mNameEdit, &QLineEdit::textChanged, this, &CreatePointDialogue::slotSetButtonStates);
    fl->addRow(i18nc("@title:row", "Name:"), mNameEdit);

    mLatLongEdit = new LatLongWidget(w);
    connect(mLatLongEdit, &LatLongWidget::positionValid, this, &CreatePointDialogue::slotSetButtonStates);
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
 
    DestinationFilterModel *destinationModel = new DestinationFilterModel(this);
    destinationModel->setSourceModel(filesView()->model());
    destinationModel->setMode(routeMode ? TrackData::Route : TrackData::Waypoint);
    mContainerList->setModel(destinationModel);
    mContainerList->expandToDepth(9);

    // Try to preselect a destination for the new item.  This may
    // be overridden by setDestinationContainer() below.
    mCanCreate = true;					// assume so initially
    QModelIndex theItem;
    const int selectableItems = findSelectableItems(mContainerList->rootIndex(), destinationModel, &theItem);
    if (selectableItems==1 && theItem.isValid())
    {
        // Firstly, if there is only a single selectable destination,
        // then use that.
        mContainerList->setCurrentIndex(theItem);
    }
    else if (selectableItems==0)
    {
        // If there are no selectable destinations, then nothing can
        // be created.
        mCanCreate = false;
    }
    else
    {
        // If there is more than one selectable destination, then if
        // the tree's selected item is valid then preselect that.
        QList<TrackDataItem *> items = filesView()->selectedItems();
        if (items.count()==1) setDestinationContainer(items.first());
    }

    connect(mContainerList->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &CreatePointDialogue::slotSetButtonStates);

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
    ItemIndexInterface *iii = dynamic_cast<ItemIndexInterface *>(mContainerList->model());
    Q_ASSERT(iii!=nullptr);
    const QModelIndex idx = iii->indexForItem(item);
    qDebug() << item->name() << "-> idx" << idx;

    if (idx.isValid()) mContainerList->setCurrentIndex(idx);
    mCanCreate = true;
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

    ItemIndexInterface *iii = dynamic_cast<ItemIndexInterface *>(mContainerList->model());
    Q_ASSERT(iii!=nullptr);
    return (iii->itemForIndex(selIndexes.first()));
}


void CreatePointDialogue::slotSetButtonStates()
{
    setButtonEnabled(QDialogButtonBox::Ok, mLatLongEdit->hasAcceptableInput() &&
                                           selectedContainer()!=nullptr);
}
