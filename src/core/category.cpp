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

#include "category.h"

#include <qdebug.h>












void CategoryList::addCategory(const QString &name, const CategoryData &cat, bool overwrite)
{
    if (mCategoryMap.contains(name))			// name already known
    {
        if (!overwrite)					// overwrite not allowed
        {
            qDebug() << "not overwriting" << name;
            return;
        }

        const int idx = mCategoryMap[name];		// index of current category item
        qDebug() << "overwriting" << name << "@" << idx;
        mCategoryList[idx] = cat;			// overwrite existing entry
    }
    else						// name not already known
    {
        const int idx = mCategoryList.count();		// index of appended category item
        qDebug() << "adding" << name << "@" << idx;
        mCategoryList.append(cat);
        mCategoryMap.insert(name, idx);
    }
}


void CategoryList::addCategories(const CategoryList *cats, bool overwrite)
{
    qDebug() << "starting with" << mCategoryMap.count() << "categories";
    for (const QString &name : cats->allNames())
    {
        addCategory(name, *cats->category(name), overwrite);
    }
    qDebug() << "finished with" << mCategoryMap.count() << "categories";
}
