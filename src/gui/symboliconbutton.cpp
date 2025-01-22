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

#include "symboliconbutton.h"

#include <qtimer.h>
#include <qevent.h>

#include <klocalizedstring.h>

#include "iconselector.h"


SymbolIconButton::SymbolIconButton(QWidget *pnt)
    : KIconButton(pnt)
{
    setObjectName("SymbolIconButton");
    setIconSize(KIconLoader::SizeMedium);
    setButtonIconSize(KIconLoader::SizeMedium);

    connect(this, &SymbolIconButton::symbolSelected, this, &SymbolIconButton::setSymbol);
    installEventFilter(this);
}


void SymbolIconButton::setSymbol(const QString &iconName, PointIcon::IconNamespace nsp)
{
    mIconName = iconName;
    mNsp = nsp;

    if (!mIconName.isEmpty())
    {
        const PointIcon *pi = PointIcon::create(mIconName, mNsp);
        if (pi!=nullptr)
        {
            if (mNsp==PointIcon::NamespaceAuto) mNsp = pi->nsp();
            setIcon(pi->icon());
        }
        else setIcon("unknown");
    }
    else
    {
        // Set an explicit icon so that the button will initially
        // show at the specified size.
        setIcon("symbol-blank");
    }

    if (isEnabled())					// only if clicking will do anything
    {
        QString toolTip = mIconName;
        if (!toolTip.isEmpty() && mNsp!=PointIcon::NamespaceAuto) toolTip += " ("+PointIcon::namespaceDisplayName(mNsp)+')';
        else toolTip = i18nc("@info:tooltip", "Select icon...");
        setToolTip(toolTip);
    }
}


bool SymbolIconButton::eventFilter(QObject *obj, QEvent *ev)
{
    // We do not want the KIconButton to open the standard KIconDialog
    // on a click, but rather to replace it with our own IconSelector
    // with the repertoire of GPS icons.  Therefore we intercept the
    // button click and handle it here, without passing the event on.
    if (ev->type()!=QEvent::MouseButtonRelease) return (false);
    QMouseEvent *mev = static_cast<QMouseEvent *>(ev);
    if (mev->button()!=Qt::LeftButton) return (false);
    if (!isEnabled()) return (false);			// no action if not enabled

    // To avoid any potential problems with nested event loops, execute
    // the dialogue and emit the signal outside of the event filter.
    QTimer::singleShot(0, this, [this]()
    {
        IconSelector dlg(mIconName, mNsp, this);
        if (!dlg.exec()) return;

        emit symbolSelected(dlg.selectedIconName(), dlg.selectedNamespace());
    });

    return (true);					// have handled the event
}
