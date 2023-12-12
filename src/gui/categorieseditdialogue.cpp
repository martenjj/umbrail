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

#include "categorieseditdialogue.h"

#include <qtreewidget.h>
#include <qheaderview.h>
#include <qpushbutton.h>
#include <qdebug.h>

#include <klocalizedstring.h>
#include <kstandardguiitem.h>
#include <kmessagebox.h>
#include <kconfiggroup.h>

#include "category.h"


enum COLUMN
{
    COL_PRIMARY,                                        // primary checkbox
    COL_SECONDARY,                                      // secondary checkbox
    COL_NAME,                                           // name
    COL_COUNT                                           // how many - must be last
};


CategoriesEditDialogue::CategoriesEditDialogue(const QStringList *itemCats, const CategoryList *allCats, QWidget *pnt)
    : DialogBase(pnt),
      DialogStateSaver(this)
{
    setObjectName("CategoriesEditDialogue");
    setWindowTitle(i18n("Select Categories"));
    setButtons(QDialogButtonBox::Ok|QDialogButtonBox::Cancel|QDialogButtonBox::Reset);
    setButtonGuiItem(QDialogButtonBox::Reset, KStandardGuiItem::clear());
    connect(buttonBox()->button(QDialogButtonBox::Reset), &QPushButton::clicked, this, &CategoriesEditDialogue::slotClear);

    mList = new QTreeWidget(this);
    mList->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    mList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    mList->setRootIsDecorated(false);
    mList->setColumnCount(COL_COUNT);

    // Guard against the possibility of not having the full list
    // of all categories available;  in this case, display the
    // item's categories only.  Disable the list to indicate that
    // there is no point trying to change anything.
    const bool haveList = (allCats!=nullptr);
    if (!haveList) mList->setEnabled(false);

    const QStringList list = (haveList ? allCats->allNames() : *itemCats);
    for (const QString &cat : list)
    {
        QTreeWidgetItem *item = new QTreeWidgetItem(QStringList() << "" << "" << cat);
        item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled|Qt::ItemIsUserCheckable);

        const int idx = itemCats->indexOf(cat);
        item->setCheckState(COL_PRIMARY, idx==0 ? Qt::Checked : Qt::Unchecked);
        item->setCheckState(COL_SECONDARY, idx>=1 ? Qt::Checked : Qt::Unchecked);

        mList->addTopLevelItem(item);
    }

    QStringList hdrs;
    hdrs << i18nc("@title:column abbreviation for PRIMARY", "Pri")
         << i18nc("@title:column abbreviation for SECONDARY", "Sec")
         << i18nc("@title:column", "Name");
    mList->setHeaderLabels(hdrs);

    QTreeWidgetItem *hdrItem = mList->headerItem();
    hdrItem->setData(COL_PRIMARY, Qt::ToolTipRole, i18nc("@info:tooltip", "<div>The primary category.<br>For those applications (e.g. OsmAnd) that only use one category.</div>"));
    hdrItem->setData(COL_SECONDARY, Qt::ToolTipRole, i18nc("@info:tooltip", "<div>Additional secondary categories.<br>For those applications (e.g. Garmin) that can use multiple categories.</div>"));

    setMainWidget(mList);
    connect(mList, &QTreeWidget::itemChanged, this, &CategoriesEditDialogue::slotItemChanged);

    setMinimumSize(350,320);
    setStateSaver(this);
}


QStringList CategoriesEditDialogue::categories()
{
    QStringList cats;
    int primaryIndex = -1;

    for (int i = 0; i<mList->topLevelItemCount(); ++i)
    {
        QTreeWidgetItem *item = mList->topLevelItem(i);
        if (item==nullptr) continue;

        if (item->checkState(COL_PRIMARY)==Qt::Checked)	// the primary category
        {
            primaryIndex = i;				// just note index for now
        }
        else if (item->checkState(COL_SECONDARY)==Qt::Checked)
        {						// a secondary category
            cats.append(item->text(COL_NAME));
        }
    }

    if (primaryIndex!=-1)				// there was a primary category,
    {							// put it at front of list
        cats.prepend(mList->topLevelItem(primaryIndex)->text(COL_NAME));
    }
    else if (!cats.isEmpty())				// no primary category,
    {							// but secondary categories set
        KMessageBox::information(this, xi18nc("@info", "No primary category is set.<nl/>The first secondary category, <resource>%1</resource>,<nl/>will be taken as the primary category.", cats.first()),
                                 QString(), "noprimarycat");
    }

    return (cats);
}


void CategoriesEditDialogue::slotItemChanged(QTreeWidgetItem *item, int col)
{
    if (item->checkState(col)!=Qt::Checked) return;	// only when turning on

    if (col==COL_PRIMARY)
    {
        // Setting a category as primary turns off itself as secondary
        // and all others as primary.  Clearing the primary category
        // does nothing.
        int idx = mList->indexOfTopLevelItem(item);	// index of this item
        for (int i = 0; i<mList->topLevelItemCount(); ++i)
        {
            QTreeWidgetItem *otherItem = mList->topLevelItem(i);
            if (i==idx)					// this is the primary item
            {
                otherItem->setCheckState(COL_SECONDARY, Qt::Unchecked);
            }
            else					// not this primary item
            {
                otherItem->setCheckState(COL_PRIMARY, Qt::Unchecked);
            }
        }
    }
    else if (col==COL_SECONDARY)
    {
        // Setting a category as secondary turns itself off as primary.
        // Clearing a secondary category does nothing.
        item->setCheckState(COL_PRIMARY, Qt::Unchecked);
    }
}


void CategoriesEditDialogue::slotClear()
{
    for (int i = 0; i<mList->topLevelItemCount(); ++i)
    {
        QTreeWidgetItem *item = mList->topLevelItem(i);
        if (item==nullptr) continue;
        item->setCheckState(COL_PRIMARY, Qt::Unchecked);
        item->setCheckState(COL_SECONDARY, Qt::Unchecked);
    }
}


void CategoriesEditDialogue::restoreConfig(QDialog *dlg, const KConfigGroup &grp)
{
    DialogStateSaver::restoreConfig(dlg, grp);
    QString colStates = grp.readEntry("State");
    if (!colStates.isEmpty()) mList->header()->restoreState(QByteArray::fromHex(colStates.toLocal8Bit()));
}


void CategoriesEditDialogue::saveConfig(QDialog *dlg, KConfigGroup &grp) const
{
    grp.writeEntry("State", mList->header()->saveState().toHex());
    DialogStateSaver::saveConfig(dlg, grp);
}
