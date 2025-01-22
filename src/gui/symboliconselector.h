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

#ifndef SYMBOLICONSELECTOR_H
#define SYMBOLICONSELECTOR_H

#include <kfdialog/dialogbase.h>
#include <kfdialog/dialogstatesaver.h>

#include "pointicon.h"


class QListWidget;
class QComboBox;
class KListWidgetSearchLine;


class SymbolIconSelector : public DialogBase, public DialogStateSaver
{
    Q_OBJECT

public:
    explicit SymbolIconSelector(const QString &sym, PointIcon::IconNamespace nsp, QWidget *pnt = nullptr);
    virtual ~SymbolIconSelector();

    QString selectedIconName() const;
    PointIcon::IconNamespace selectedNamespace() const;

    void saveConfig(QDialog *dialog, KConfigGroup &grp) const override;
    void restoreConfig(QDialog *dialog, const KConfigGroup &grp) override;

private slots:
    void slotSourceChanged();
    void slotSelectionChanged();
    void slotClearIcon();

private:
    QListWidget *mList;
    QComboBox *mSourceCombo;
    KListWidgetSearchLine *mSearchLine;

    QString mSelectedName;
    bool mHadInitialNamespace;
};

#endif							// SYMBOLICONSELECTOR_H
