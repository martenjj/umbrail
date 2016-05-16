//////////////////////////////////////////////////////////////////////////
//									//
//  Project:	NavTracks						//
//  Edit:	16-May-16						//
//									//
//////////////////////////////////////////////////////////////////////////
//									//
//  Copyright (c) 2012-2014 Jonathan Marten <jjm@keelhaul.me.uk>	//
//  Home and download page:  http://www.keelhaul.me.uk/TBD/		//
//									//
//  This program is free software; you can redistribute it and/or	//
//  modify it under the terms of the GNU General Public License as	//
//  published by the Free Software Foundation; either version 2 of	//
//  the License, or (at your option) any later version.			//
//									//
//  It is distributed in the hope that it will be useful, but		//
//  WITHOUT ANY WARRANTY; without even the implied warranty of		//
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the	//
//  GNU General Public License for more details.			//
//									//
//  You should have received a copy of the GNU General Public		//
//  License along with this program; see the file COPYING for further	//
//  details.  If not, write to the Free Software Foundation, Inc.,	//
//  59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.		//
//									//
//////////////////////////////////////////////////////////////////////////

#include "timezoneselector.h"

#include <qpushbutton.h>
#include <qdebug.h>
#include <qlineedit.h>
#include <qboxlayout.h>

#include <klocalizedstring.h>

#include "timezonedialogue.h"


TimeZoneSelector::TimeZoneSelector(QWidget *pnt)
    : QFrame(pnt)
{
    setObjectName("TimeZoneSelector");

    QHBoxLayout *hb = new QHBoxLayout(this);
    hb->setMargin(0);

    mZoneDisplay = new QLineEdit(this);
    mZoneDisplay->setReadOnly(true);
    mZoneDisplay->setPlaceholderText(i18n("(UTC)"));
    hb->addWidget(mZoneDisplay);
    connect(mZoneDisplay, SIGNAL(textChanged(const QString &)), SIGNAL(zoneChanged(const QString &)));

    QPushButton *b = new QPushButton(i18nc("@action:button", "Change..."), this);
    hb->addWidget(b);
    connect(b, SIGNAL(clicked()), SLOT(slotChangeZone()));

    setFocusProxy(b);
    setFocusPolicy(Qt::StrongFocus);
}


void TimeZoneSelector::setTimeZone(const QString &zone)
{
    mZoneDisplay->setText(zone);
}


QString TimeZoneSelector::timeZone() const
{
    return (mZoneDisplay->text());
}


void TimeZoneSelector::slotChangeZone()
{
    TimeZoneDialogue d(this);
    d.setTimeZone(timeZone().toLatin1());
    if (d.exec()) setTimeZone(d.timeZone());		// will emit the signal
}
