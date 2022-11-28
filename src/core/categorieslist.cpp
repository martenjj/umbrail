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

#include "categorieslist.h"

#include <qdebug.h>


void CategoriesList::addCategory(const QString &cat, const QColor &col, bool overwrite)
{
    if (!overwrite && mCategoryMap.contains(cat))
    {
        qDebug() << "not overwriting" << cat;
        return;
    }

    qDebug() << "adding" << cat;
    mCategoryMap.insert(cat, col);
}


void CategoriesList::addCategories(const CategoriesList *cats, bool overwrite)
{
    //qDebug() << "starting with" << mCategoryMap.count() << "categories";
    for (QMap<QString,QColor>::const_iterator it = cats->mCategoryMap.constBegin();
         it!=cats->mCategoryMap.constEnd(); ++it)
    {
        addCategory(it.key(), it.value(), overwrite);
    }
    //qDebug() << "finished with" << mCategoryMap.count() << "categories";
}


// // TODO: case insensitivity needed?
// void CategoriesList::scanForNew(const PointsModel *model)
// {
//     const int cnt = model->pointsCount();
//     const int oldCnt = mCategories.count();
// 
//     for (int i = 0; i<cnt; ++i)
//     {
//         const PointData *pnt = model->pointAt(i);
//         const QStringList *cats = pnt->categories();
//         for (QStringList::const_iterator it = cats->constBegin(); it!=cats->constEnd(); ++it)
//         {
//             const QString cat = (*it);
//             if (!mCategories.contains(Category(cat)))
//             {
//                 qDebug() << "new category" << cat << "from" << pnt->name() << "colour" << pnt->colour();
//                 mCategories.append(Category(cat, pnt->colour()));
//             }
//         }
//     }
// 
//     const int added = mCategories.count()-oldCnt;
//     qDebug() << "scanned" << cnt << "points, added" << added << "categories";
//     if (added>0) std::sort(mCategories.begin(), mCategories.end());
// }
