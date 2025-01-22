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
#include <qscrollbar.h>
#include <qboxlayout.h>
#include <qtoolbutton.h>

#include <klocalizedstring.h>
#include <kmessagebox.h>
#include <kconfiggroup.h>
#include <kcolorbutton.h>

#include "symboliconbutton.h"

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

    QHBoxLayout *hlay = new QHBoxLayout(w);
    hlay->setContentsMargins(0, 0, 0, 0);

    mColourButton = new KColorButton(w);
    hlay->addWidget(mColourButton);
    hlay->addStretch(1);

    QToolButton *but = new QToolButton(w);
    but->setIcon(QIcon::fromTheme("edit-clear"));
    but->setToolTip(i18nc("@info:tooltip", "Clear the category colour"));
    connect(but, &QAbstractButton::clicked, this, &CategoryEditDialogue::slotClearColour);
    hlay->addWidget(but);

    lay->addRow(i18n("Colour:"), hlay);

    mIconButton = new SymbolIconButton(this);
    connect(mIconButton, &SymbolIconButton::symbolSelected, this, &CategoryEditDialogue::slotSymbolSelected);
    lay->addRow(i18n("Symbol:"), mIconButton);

    mShapeEdit = new QLineEdit(w);
    lay->addRow(i18n("Shape:"), mShapeEdit);

    if (cat!=nullptr)					// original category data provided
    {
        mColourButton->setColor(cat->colour());
        mIconButton->setSymbol(cat->icon());
        mShapeEdit->setText(cat->shape());
    }

    setMainWidget(w);
    w->setMinimumWidth(250);
    slotUpdateButtonStates();
    mNameEdit->setFocus(Qt::OtherFocusReason);
}


void CategoryEditDialogue::slotSymbolSelected(const QString &iconName, PointIcon::IconNamespace nsp)
{
    mCategory.setIcon(iconName);
}


QString CategoryEditDialogue::name() const
{
    return (mNameEdit->text());
}


void CategoryEditDialogue::slotUpdateButtonStates()
{
    setButtonEnabled(QDialogButtonBox::Ok, !mNameEdit->text().isEmpty());
}


void CategoryEditDialogue::slotClearColour()
{
    mColourButton->setColor(QColor());
}


void CategoryEditDialogue::accept()
{
    mCategory.setColour(mColourButton->color());
    mCategory.setShape(mShapeEdit->text());		// TODO: shape combo

    DialogBase::accept();
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
    COL_ICON,                                           // default icon
    COL_SHAPE,						// background shape
    COL_COUNT                                           // how many - must be last
};


CategoriesManageDialogue::CategoriesManageDialogue(const CategoryList *cats, QWidget *pnt)
    : DialogBase(pnt),
      DialogStateSaver(this)
{
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
    hdrs << i18nc("@title:column", "Icon") << i18nc("@title:column", "Shape");
    mList->setHeaderLabels(hdrs);

    QTreeWidgetItem *hdrItem = mList->headerItem();
    hdrItem->setData(COL_COLOUR, Qt::ToolTipRole, i18nc("@info:tooltip", "The colour used by OsmAnd+ to display the category icons"));
    hdrItem->setData(COL_NAME, Qt::ToolTipRole, i18nc("@info:tooltip", "The name of the category"));
    hdrItem->setData(COL_ICON, Qt::ToolTipRole, i18nc("@info:tooltip", "The icon for the category"));
    hdrItem->setData(COL_SHAPE, Qt::ToolTipRole, i18nc("@info:tooltip", "The background shape used by OsmAnd+"));

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

    mList->clear();
    if (cats!=nullptr)
    {
        const QStringList catNames = cats->allNames();
        for (const QString &name : std::as_const(catNames)) addCategoryItem(name, cats->category(name));
    }

    slotUpdateButtonStates();
}


static inline void setItemData(QTreeWidgetItem *item, const QString &name, const CategoryData *cat)
{
    item->setText(COL_NAME, name);
    item->setData(COL_COLOUR, Qt::UserRole, cat->colour());

    const QString icn = cat->icon();
    const QString shp = cat->shape();

    item->setData(COL_ICON, Qt::UserRole, icn);
    item->setIcon(COL_ICON, QIcon::fromTheme("symbol-blank"));
    if (!icn.isEmpty())
    {
        item->setText(COL_ICON, icn);
        // This call must pass a null QVariant if there is no shape,
        // otherwise OsmandIconProvider will warn that it is unknown.
        const PointIcon *ic = PointIcon::create(icn, PointIcon::NamespaceAuto,
                                                cat->colour(),
                                                (!shp.isEmpty() ? shp : QVariant()));
        if (ic!=nullptr) item->setIcon(COL_ICON, ic->icon());
    }
    else
    {
        item->setText(COL_ICON, i18nc("@item:intable value not set", "(none)"));
    }

    item->setData(COL_SHAPE, Qt::UserRole, shp);
    item->setText(COL_SHAPE, (!shp.isEmpty() ? shp : i18nc("@item:intable value not set", "(none)")));
}


static inline QString getItemData(const QTreeWidgetItem *item, CategoryData *cat)
{
    cat->setColour(item->data(COL_COLOUR, Qt::UserRole).value<QColor>());
    cat->setIcon(item->data(COL_ICON, Qt::UserRole).value<QString>());
    cat->setShape(item->data(COL_SHAPE, Qt::UserRole).value<QString>());
    return (item->text(COL_NAME));
}


QTreeWidgetItem *CategoriesManageDialogue::addCategoryItem(const QString &name, const CategoryData *cat)
{
    QTreeWidgetItem *item = new QTreeWidgetItem;
    setItemData(item, name, cat);
    item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
    mList->addTopLevelItem(item);
    return (item);
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
    QString name = getItemData(item, &cat);

    CategoryEditDialogue d(name, &cat, this);
    if (!d.exec()) return;

    setItemData(item, d.name(), d.category());
    mList->sortItems(COL_NAME, Qt::AscendingOrder);
    mList->scrollToItem(item);
}


void CategoriesManageDialogue::slotDeleteCategory()
{
    QList<QTreeWidgetItem *> sel = mList->selectedItems();
    const int num = sel.count();
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

    // Avoids an annoying jump to the left if the list widget is too small.
    mList->horizontalScrollBar()->setValue(0);
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
        QString name = getItemData(item, &cat);
        mCategories.addCategory(name, cat);
    }

    DialogBase::accept();
}
