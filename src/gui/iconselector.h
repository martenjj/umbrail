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

#ifndef ICONSELECTOR_H
#define ICONSELECTOR_H

#include <kfdialog/dialogbase.h>

#include "pointicon.h"


class QListWidget;
class QComboBox;


class IconSelector : public DialogBase
{
    Q_OBJECT

public:
    explicit IconSelector(const QString &sym, PointIcon::IconNamespace nsp, QWidget *pnt = nullptr);
    virtual ~IconSelector() = default;

    QString selectedIconName() const;
    PointIcon::IconNamespace selectedNamespace() const;

private slots:
    void slotSourceChanged();
    void slotSelectionChanged();
    void slotClearIcon();

private:
    QListWidget *mList;
    QComboBox *mSourceCombo;

    QString mSelectedName;
};

#endif							// ICONSELECTOR_H
