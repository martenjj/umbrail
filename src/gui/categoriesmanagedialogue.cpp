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

#include "categoriesmanagedialogue.h"

#include <qtreewidget.h>
#include <qheaderview.h>
#include <qgridlayout.h>
#include <qpushbutton.h>
#include <qpainter.h>
#include <qabstractitemdelegate.h>
#include <qdebug.h>
#include <qformlayout.h>
#include <qlineedit.h>

#include <klocalizedstring.h>
#include <kmessagebox.h>
#include <kconfiggroup.h>
#include <kcolorbutton.h>

//////////////////////////////////////////////////////////////////////////
//									//
//  CategoryEditDialogue						//
//									//
//////////////////////////////////////////////////////////////////////////

CategoryEditDialogue::CategoryEditDialogue(const QString &name, const CategoryData *cat, QWidget *pnt)
    : DialogBase(pnt)
{
    setObjectName("CategoryEditDialogue");
    setWindowTitle(i18n("Edit Category"));
    setButtons(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);

    QWidget *w = new QWidget(this);
    QFormLayout *lay = new QFormLayout(w);

    mNameEdit = new QLineEdit(w);
    mNameEdit->setText(name);
    connect(mNameEdit, &QLineEdit::textEdited, this, &CategoryEditDialogue::slotUpdateButtonStates);
    lay->addRow(i18n("Name:"), mNameEdit);

    mColourButton = new KColorButton(w);
    lay->addRow(i18n("Colour:"), mColourButton);

    if (cat!=nullptr)					// original category data provided
    {
        mColourButton->setColor(cat->colour());
    }

    setMainWidget(w);
    w->setMinimumWidth(250);
    slotUpdateButtonStates();
    mNameEdit->setFocus(Qt::OtherFocusReason);
}


QString CategoryEditDialogue::name() const
{
    return (mNameEdit->text());
}


CategoryData CategoryEditDialogue::category() const
{
    CategoryData res;
    res.setColour(mColourButton->color());
    return (res);
}


void CategoryEditDialogue::slotUpdateButtonStates()
{
    setButtonEnabled(QDialogButtonBox::Ok, !mNameEdit->text().isEmpty());
}

//////////////////////////////////////////////////////////////////////////
//									//
//  CategoriesManageDialogueItemDelegate				//
//									//
//////////////////////////////////////////////////////////////////////////

void CategoriesManageDialogueItemDelegate::paint(QPainter *p, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QColor col = index.data(Qt::UserRole).value<QColor>();
    QRect r = option.rect.adjusted(2, 1, -2, -1);

    p->setPen(option.palette.color(QPalette::Normal, QPalette::WindowText));
    if (col.isValid()) p->setBrush(col);
    else p->setBrush(Qt::Dense4Pattern);
    p->drawRect(r);
}

//////////////////////////////////////////////////////////////////////////
//									//
//  CategoriesManageDialogue						//
//									//
//////////////////////////////////////////////////////////////////////////

enum COLUMN
{
    COL_COLOUR,						// colour square
    COL_NAME,                                           // category name
    COL_COUNT                                           // how many - must be last
};


CategoriesManageDialogue::CategoriesManageDialogue(const CategoryList *cats, QWidget *pnt)
    : DialogBase(pnt),
      DialogStateSaver(this)
{
    if (cats!=nullptr) mCategories.addCategories(cats);	// copy original categories

    setObjectName("CategoriesManageDialogue");
    setWindowTitle(i18n("Manage Categories"));
    setButtons(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);

    QWidget *w = new QWidget(this);
    QGridLayout *lay = new QGridLayout(w);

    mList = new QTreeWidget(w);
    mList->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    mList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    mList->setRootIsDecorated(false);
    mList->setSelectionMode(QAbstractItemView::ExtendedSelection);
    mList->setColumnCount(COL_COUNT);
    mList->setAllColumnsShowFocus(true);
    mList->setAlternatingRowColors(false);
    mList->setItemDelegateForColumn(COL_COLOUR, new CategoriesManageDialogueItemDelegate(this));
    connect(mList, &QTreeWidget::itemSelectionChanged, this, &CategoriesManageDialogue::slotUpdateButtonStates);

    QStringList hdrs;
    hdrs << i18nc("@title:column", "Colour") << i18nc("@title:column", "Category");
    mList->setHeaderLabels(hdrs);

    QTreeWidgetItem *hdrItem = mList->headerItem();
    hdrItem->setData(COL_COLOUR, Qt::ToolTipRole, i18nc("@info:tooltip", "The colour used by OsmAnd+ to display the category icons"));
    hdrItem->setData(COL_NAME, Qt::ToolTipRole, i18nc("@info:tooltip", "The name of the category"));

    lay->addWidget(mList, 0, 0, 4, 1);

    mNewButton = new QPushButton(i18n("Add..."), this);
    mNewButton->setIcon(QIcon::fromTheme("list-add"));
    connect(mNewButton, &QAbstractButton::clicked, this, &CategoriesManageDialogue::slotNewCategory);
    lay->addWidget(mNewButton, 0, 2);

    mEditButton = new QPushButton(i18n("Edit..."), this);
    mEditButton->setIcon(QIcon::fromTheme("document-edit"));
    connect(mEditButton, &QAbstractButton::clicked, this, &CategoriesManageDialogue::slotEditCategory);
    lay->addWidget(mEditButton, 1, 2);

    mDeleteButton = new QPushButton(i18n("Delete"), this);
    mDeleteButton->setIcon(QIcon::fromTheme("edit-delete"));
    connect(mDeleteButton, &QAbstractButton::clicked, this, &CategoriesManageDialogue::slotDeleteCategory);
    lay->addWidget(mDeleteButton, 2, 2);

    lay->setColumnMinimumWidth(1, DialogBase::verticalSpacing());
    lay->setColumnStretch(0, 1);
    lay->setRowStretch(3, 1);

    setMainWidget(w);
    setStateSaver(this);

    createDisplay();
    slotUpdateButtonStates();
}


static inline void setItemData(QTreeWidgetItem *item, const QString &name, const QColor &col)
{
    item->setText(COL_NAME, name);
    item->setData(COL_COLOUR, Qt::UserRole, col);
}


QTreeWidgetItem *CategoriesManageDialogue::addCategoryItem(const QString &name, const CategoryData &cat)
{
    QTreeWidgetItem *item = new QTreeWidgetItem;
    setItemData(item, name, cat.colour());
    item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
    mList->addTopLevelItem(item);
    return (item);
}


void CategoriesManageDialogue::createDisplay()
{
    mList->clear();
    const QStringList catNames = mCategories.allNames();
    for (const QString &name : std::as_const(catNames)) addCategoryItem(name, *mCategories.category(name));
}


// TODO: check for duplication
void CategoriesManageDialogue::slotNewCategory()
{
    CategoryEditDialogue d("", nullptr, this);
    if (!d.exec()) return;

    QTreeWidgetItem *item = addCategoryItem(d.name(), d.category());
    mList->sortItems(COL_NAME, Qt::AscendingOrder);
    mList->setCurrentItem(item);
    mList->scrollToItem(item);
}


// TODO: check for duplication
void CategoriesManageDialogue::slotEditCategory()
{
    QList<QTreeWidgetItem *> sel = mList->selectedItems();
    if (sel.count()!=1) return;

    QTreeWidgetItem *item = sel.first();
    CategoryData cat;
    cat.setColour(item->data(COL_COLOUR, Qt::UserRole).value<QColor>());
    CategoryEditDialogue d(item->text(COL_NAME), &cat, this);
    if (!d.exec()) return;

    setItemData(item, d.name(), d.category().colour());
    mList->sortItems(COL_NAME, Qt::AscendingOrder);
    mList->scrollToItem(item);
}


void CategoriesManageDialogue::slotDeleteCategory()
{
    QList<QTreeWidgetItem *> sel = mList->selectedItems();
    int num = sel.count();
    if (num==0) return;

    QString query;
    if (num==1) query = xi18nc("@info", "Delete the selected category <resource>%1</resource>?", sel.first()->text(COL_NAME));
    else query = i18ncp("@info", "Delete the selected category?", "Delete the %1 selected categories?", num);

    if (KMessageBox::warningContinueCancel(this, query,
                                           i18n("Delete Categories"),
                                           KStandardGuiItem::del(),
                                           KStandardGuiItem::cancel(),
                                           "deletecategory",
                                           KMessageBox::Dangerous)!=KMessageBox::Continue) return;
    for (int i = 0; i<num; ++i)
    {
        QTreeWidgetItem *item = sel[i];
        int row = mList->indexOfTopLevelItem(item);
        delete mList->takeTopLevelItem(row);
    }
}


void CategoriesManageDialogue::slotUpdateButtonStates()
{
    int num = mList->selectedItems().count();
    mEditButton->setEnabled(num==1);
    mDeleteButton->setEnabled(num>0);
}


void CategoriesManageDialogue::restoreConfig(QDialog *dlg, const KConfigGroup &grp)
{
    DialogStateSaver::restoreConfig(dlg, grp);
    QString colStates = grp.readEntry("State");
    if (!colStates.isEmpty()) mList->header()->restoreState(QByteArray::fromHex(colStates.toLocal8Bit()));
}


void CategoriesManageDialogue::saveConfig(QDialog *dlg, KConfigGroup &grp) const
{
    grp.writeEntry("State", mList->header()->saveState().toHex());
    DialogStateSaver::saveConfig(dlg, grp);
}


void CategoriesManageDialogue::accept()
{
    const int num = mList->topLevelItemCount();		// how many in GUI list
    mCategories.clear();				// clear all existing categories
    for (int i = 0; i<num; ++i)				// recreate the category list
    {
        const QTreeWidgetItem *item = mList->topLevelItem(i);
        CategoryData cat;
        cat.setColour(item->data(COL_COLOUR, Qt::UserRole).value<QColor>());
        mCategories.addCategory(item->text(COL_NAME), cat);
    }

    DialogBase::accept();
}
