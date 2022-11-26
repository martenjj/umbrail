//////////////////////////////////////////////////////////////////////////
//									//
//  Project:	Umbrail - GPX track viewer and editor			//
//									//
//////////////////////////////////////////////////////////////////////////
//									//
//  Copyright (c) 2014-2021 Jonathan Marten <jjm@keelhaul.me.uk>	//
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

#ifndef CATEGORIESMANAGER_H
#define CATEGORIESMANAGER_H

#include <qmap.h>
#include <qcolor.h>

// #include "categorydata.h"


class QColor;
// class KConfig;
// 
// class PointsModel;


class CategoriesManager
{

public:
    static CategoriesManager *self();

    void addCategory(const QString &cat, const QColor &colour, bool overwrite = true);

//     QString save(KConfig *conf);
//     QString load(const KConfig *conf);

//     void scanForNew(const PointsModel *model);
//     void clear();

//     void setCategories(const Category::List &cats) 		{ mCategories = cats; }
//     const Category::List *categories() const			{ return (&mCategories); }

    QColor colourFor(const QString &cat) const			{ return (mCategoryMap.value(cat)); }
    QStringList allCategories() const				{ return (mCategoryMap.keys()); }

private:
    CategoriesManager();
    ~CategoriesManager() = default;

private:
    QMap<QString, QColor> mCategoryMap;
};

 
#endif							// CATEGORIESMANAGER_H
