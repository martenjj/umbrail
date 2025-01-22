//////////////////////////////////////////////////////////////////////////
//									//
//  Project:	Umbrail - GPX track viewer and editor			//
//									//
//////////////////////////////////////////////////////////////////////////
//									//
//  Copyright (c) 2014-2025 Jonathan Marten <jjm@keelhaul.me.uk>	//
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

#include "symbolshapecombo.h"

#include <klocalizedstring.h>


SymbolShapeCombo::SymbolShapeCombo(QWidget *pnt)
    : QComboBox(pnt)
{
    setObjectName("SymbolShapeCombo");

    addItem(QIcon::fromTheme("edit-delete"), i18nc("@item:inlistbox for icon shape", "(None)"), "");
    addItem(QIcon::fromTheme("shape-circle"), i18nc("@item:inlistbox for icon shape", "Circle"), "circle");
    addItem(QIcon::fromTheme("shape-square"), i18nc("@item:inlistbox for icon shape", "Square"), "square");
    addItem(QIcon::fromTheme("shape-octagon"), i18nc("@item:inlistbox for icon shape", "Octagon"), "octagon");

    connect(this, &QComboBox::currentIndexChanged, this, &SymbolShapeCombo::slotIconShapeChanged);
    connect(this, &SymbolShapeCombo::shapeSelected, this, &SymbolShapeCombo::setShape);
}


void SymbolShapeCombo::setShape(const QString &shape)
{
    QSignalBlocker block(this);
    const int idx = findData(shape);
    if (idx!=-1) setCurrentIndex(idx);
}


void SymbolShapeCombo::slotIconShapeChanged(int idx)
{
    emit shapeSelected(currentData().toString());
}
