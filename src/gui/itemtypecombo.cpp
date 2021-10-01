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

#include "itemtypecombo.h"

#include <qdebug.h>

#include <klocalizedstring.h>
#include <ksharedconfig.h>
#include <kconfiggroup.h>


ItemTypeCombo::ItemTypeCombo(QWidget *pnt)
    : KComboBox(pnt)
{
    setObjectName("ItemTypeCombo");

    setEditable(true);
    setDuplicatesEnabled(false);
    setInsertPolicy(QComboBox::InsertAtBottom);
    setSizePolicy(QSizePolicy::Expanding, sizePolicy().verticalPolicy());

    QStringList defaultTypes(i18n("(none)"));
    defaultTypes << "Walk" << "Car";
    const KConfigGroup grp = KSharedConfig::openConfig()->group(objectName());
    QStringList types = grp.readEntry("types", defaultTypes);
    addItems(types);

    mOriginalCount = count();				// how many when we started
}


ItemTypeCombo::~ItemTypeCombo()
{
    qDebug() << "count" << count() << "original" << mOriginalCount;
    if (count()>mOriginalCount)				// was anything added?
    {
        QStringList types;
        for (int i = 0; i<count(); ++i) types << itemText(i);
        qDebug() << "saving" << types;
        KConfigGroup grp = KSharedConfig::openConfig()->group(objectName());
        grp.writeEntry("types", types);
    }
}


void ItemTypeCombo::setType(const QString &type)
{
    if (type.isEmpty()) setCurrentIndex(0);		// "none" is always first
    else
    {
        int idx = findText(type, Qt::MatchFixedString);
        qDebug() << type << "found at" << idx;
        if (idx==-1)					// not already in combo
        {
            addItem(type);				// add as new item
            idx = count()-1;				// item was added at end
        }

        setCurrentIndex(idx);				// set item as current
    }
}


QString ItemTypeCombo::typeText() const
{
    if (currentIndex()==0) return (QString());		// "none" means blank
    return (currentText());
}
