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

#include "categoriesmanager.h"

#include <qdebug.h>

// #include <kconfig.h>
// #include <kconfiggroup.h>
// #include <klocalizedstring.h>

// #include "pointsmodel.h"
// #include "pointdata.h"
// 
// 
// #define GROUP_CATEGORIES	"Categories"
// #define CONFIG_LIST		"List"
// #define CONFIG_NAME		"Name%1"
// #define CONFIG_COLOUR		"Colour%1"



CategoriesManager *CategoriesManager::self()
{
    static CategoriesManager *instance = new CategoriesManager();
    return (instance);
}


CategoriesManager::CategoriesManager()
{
    qDebug();
}


void CategoriesManager::addCategory(const QString &cat, const QColor &col, bool overwrite)
{
    if (!overwrite && mCategoryMap.contains(cat))
    {
        qDebug() << "not overwriting" << cat;
        return;
    }

    qDebug() << "adding" << cat;
    mCategoryMap.insert(cat, col);
}








// QString CategoriesManager::save(KConfig *conf)
// {
//     qDebug() << "saving" << mCategories.count() << "to" << conf->name();
// 
//     KConfigGroup grp = conf->group(GROUP_CATEGORIES);
//     for (int i = 0; i<mCategories.count(); ++i)
//     {
//         const Category cat = mCategories[i];
//         grp.writeEntry(QString(CONFIG_NAME).arg(i), cat.name());
// 
//         const QColor col = cat.colour();
//         if (col.isValid()) grp.writeEntry(QString(CONFIG_COLOUR).arg(i), col);
//     }
// 
//     return (QString());
// }


// QString CategoriesManager::load(const KConfig *conf)
// {
//     const KConfigGroup grp = conf->group(GROUP_CATEGORIES);
// 
//     for (int i = 0; ; ++i)				// try new format first
//     {
//         const QString name = grp.readEntry(QString(CONFIG_NAME).arg(i), "");
//         if (name.isEmpty()) break;
// 
//         const QColor col = grp.readEntry(QString(CONFIG_COLOUR).arg(i), QColor());
//         mCategories.append(Category(name, col));
//     }
// 
//     if (mCategories.isEmpty())				// fallback to old format?
//     {
//         QStringList names = grp.readEntry(CONFIG_LIST, QStringList());
//         if (!names.isEmpty())
//         {
//             for (QStringList::const_iterator it = names.constBegin(); it!=names.constEnd(); ++it)
//             {
//                 const QString name = (*it);
//                 mCategories.append(Category(name));
//             }
//         }
//     }
// 
//     qDebug() << "loaded" << mCategories.count() << "from" << conf->name();
//     return (QString());
// }


// void CategoriesManager::clear()
// {
//     mCategories.clear();
// }


// // TODO: case insensitivity needed?
// void CategoriesManager::scanForNew(const PointsModel *model)
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


// QColor CategoriesManager::colourFor(const QString &cat) const
// {
//     int idx = mCategories.indexOf(Category(cat));
//     if (idx<0) return (QColor());
//     return (mCategories[idx].colour());
// }
