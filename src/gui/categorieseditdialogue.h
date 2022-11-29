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

#ifndef CATEGORIESEDITDIALOGUE_H
#define CATEGORIESEDITDIALOGUE_H
 

#include <kfdialog/dialogbase.h>
#include <kfdialog/dialogstatesaver.h>


class QTreeWidget;
class QTreeWidgetItem;
class CategoriesList;


class CategoriesEditDialogue : public DialogBase, public DialogStateSaver
{
    Q_OBJECT

public:
    explicit CategoriesEditDialogue(const QStringList *itemCats, const CategoriesList *allCats, QWidget *pnt = NULL);
    virtual ~CategoriesEditDialogue() = default;

    QStringList categories();

    void saveConfig(QDialog *dlg, KConfigGroup &grp) const override;
    void restoreConfig(QDialog *dlg, const KConfigGroup &grp) override;

protected slots:
    void slotClear();

private slots:
    void slotItemChanged(QTreeWidgetItem *item, int col);

private:
    QTreeWidget *mList;
    const CategoriesList *mAllCategories;
    const QStringList *mItemCategories;
};

#endif							// CATEGORIESEDITDIALOGUE_H
