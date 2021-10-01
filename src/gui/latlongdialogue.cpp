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

#include "latlongdialogue.h"

#include <klocalizedstring.h>

#include "latlongwidget.h"


LatLongDialogue::LatLongDialogue(QWidget *pnt)
    : DialogBase(pnt)
{
    setObjectName("LatLongDialogue");
    setWindowTitle(i18n("Edit Position"));
    setButtons(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);

    mWidget = new LatLongWidget(this);
    connect(mWidget, SIGNAL(positionChanged(double, double)), SLOT(slotUpdateButtonState()));

    setMainWidget(mWidget);
    setMinimumWidth(400);
}


void LatLongDialogue::setLatLong(double lat, double lon)
{
    mWidget->setLatLong(lat, lon);
    slotUpdateButtonState();				// verify acceptable values
}


void LatLongDialogue::slotUpdateButtonState()
{
    setButtonEnabled(QDialogButtonBox::Ok, mWidget->hasAcceptableInput());
}


double LatLongDialogue::latitude() const
{
    return (mWidget->latitude());
}


double LatLongDialogue::longitude() const
{
    return (mWidget->longitude());
}
