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

#ifndef CATEGORY_H
#define CATEGORY_H

#include <qmap.h>
#include <qcolor.h>


class CategoryData
{
public:
    CategoryData() = default;
    explicit CategoryData(const QColor &colour)		{ mColour = colour; }

    void setColour(const QColor &colour)		{ mColour = colour; }
    void setIcon(const QString &iconName) 		{ mIconName = iconName; }
    void setShape(const QString &shape) 		{ mShape = shape; }

    QColor colour() const				{ return (mColour); }
    QString icon() const				{ return (mIconName); }
    QString shape() const				{ return (mShape); }

private:
    QColor mColour;
    QString mIconName;
    QString mShape;
};


class CategoryList
{
public:
    CategoryList() = default;

    void addCategory(const QString &name, const CategoryData &cat, bool overwrite = true);
    void addCategories(const CategoryList *cats, bool overwrite = true);

    void clear()						{ mCategoryMap.clear(); mCategoryList.clear(); }

    const CategoryData *category(const QString &name) const	{ return (mCategoryMap.contains(name) ? &mCategoryList[mCategoryMap[name]] : nullptr); }
    int count() const						{ return (mCategoryMap.count()); }
    QStringList allNames() const				{ return (mCategoryMap.keys()); }

private:
    // This two-level mapping is needed so that that the address of a stored
    // CategoryData item can be returned as a pointer - or a null value from
    // a nonexistent lookup.  QVector has 'const_reference operator[] const'
    // which returns a const reference to the actual list item, so its address
    // can be taken.  However, QMap only has 'T& operator[]' which returns a
    // non-const rvalue whose address cannot be taken.
    //
    // mCategoryMap maps a category name to an index in the mCategoryList
    // array.  Since there is no API to delete a category from a CategoryList
    // (other than by clearing it completely), there is no need to be able to
    // remove an entry from both lists or to track removals from the QVector.
    QMap<QString,int> mCategoryMap;
    QVector<CategoryData> mCategoryList;
};

#endif							// CATEGORY_H
