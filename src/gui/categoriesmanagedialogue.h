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

#ifndef CATEGORIESMANAGEDIALOGUE_H
#define CATEGORIESMANAGEDIALOGUE_H

#include <qstyleditemdelegate.h>
#include <kfdialog/dialogbase.h>
#include <kfdialog/dialogstatesaver.h>

#include "category.h"

class QTreeWidget;
class QTreeWidgetItem;
class QPushButton;
class QLineEdit;
class KColorButton;

//////////////////////////////////////////////////////////////////////////
//									//
//  CategoryEditDialogue						//
//									//
//////////////////////////////////////////////////////////////////////////

class CategoryEditDialogue : public DialogBase
{
    Q_OBJECT

public:
    explicit CategoryEditDialogue(const QString &name, const CategoryData *cat, QWidget *pnt = nullptr);
    virtual ~CategoryEditDialogue() = default;

    QString name() const;
    CategoryData category() const;

private slots:
    void slotUpdateButtonStates();

private:
    QLineEdit *mNameEdit;
    KColorButton *mColourButton;
};

//////////////////////////////////////////////////////////////////////////
//									//
//  CategoriesManageDialogue						//
//									//
//////////////////////////////////////////////////////////////////////////

class CategoriesManageDialogue : public DialogBase, public DialogStateSaver
{
    Q_OBJECT

public:
    explicit CategoriesManageDialogue(const CategoryList *cats, QWidget *pnt = nullptr);
    virtual ~CategoriesManageDialogue() = default;

    const CategoryList *categories() const		{ return (&mCategories); }

    void saveConfig(QDialog *dlg, KConfigGroup &grp) const override;
    void restoreConfig(QDialog *dlg, const KConfigGroup &grp) override;

protected slots:
    void slotNewCategory();
    void slotEditCategory();
    void slotDeleteCategory();

    virtual void accept() override;

private:
    void createDisplay();
    QTreeWidgetItem *addCategoryItem(const QString &name, const CategoryData &cat);

private slots:
    void slotUpdateButtonStates();

private:
    QTreeWidget *mList;
    QPushButton *mNewButton;
    QPushButton *mEditButton;
    QPushButton *mDeleteButton;

    CategoryList mCategories;
};

//////////////////////////////////////////////////////////////////////////
//									//
//  CategoriesManageDialogueItemDelegate				//
//									//
//////////////////////////////////////////////////////////////////////////

class CategoriesManageDialogueItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit CategoriesManageDialogueItemDelegate(QObject *pnt = nullptr) : QStyledItemDelegate(pnt)	{}
    virtual ~CategoriesManageDialogueItemDelegate() = default;

    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

#endif							// CATEGORIESMANAGEDIALOGUE_H
