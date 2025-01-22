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

#ifndef SYMBOLICONBUTTON_H
#define SYMBOLICONBUTTON_H

#include <kiconbutton.h>

#include "pointicon.h"


class SymbolIconButton : public KIconButton
{
    Q_OBJECT

public:
    explicit SymbolIconButton(QWidget *pnt = nullptr);
    virtual ~SymbolIconButton() = default;

    PointIcon::IconNamespace iconNamespace() const	{ return (mNsp); }

public slots:
    void setSymbol(const QString &iconName, PointIcon::IconNamespace nsp = PointIcon::NamespaceAuto);

protected:
    virtual bool eventFilter(QObject *obj, QEvent *ev) override;

signals:
    void symbolSelected(const QString &iconName, PointIcon::IconNamespace nsp);

private:
    QString mIconName;
    PointIcon::IconNamespace mNsp;
};

#endif							// SYMBOLICONBUTTON_H
