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

#include "timezoneselector.h"

#include <qpushbutton.h>
#include <qdebug.h>
#include <qlineedit.h>
#include <qgridlayout.h>

#include <klocalizedstring.h>
#include <kmessagebox.h>

#include "timezonedialogue.h"
#include "timezoneprovider.h"
#include "trackdata.h"


TimeZoneSelector::TimeZoneSelector(QWidget *pnt)
    : QFrame(pnt)
{
    setObjectName("TimeZoneSelector");

    QGridLayout *gl = new QGridLayout(this);
    gl->setMargin(0);

    mZoneDisplay = new QLineEdit(this);
    mZoneDisplay->setReadOnly(true);
    mZoneDisplay->setPlaceholderText(i18n("(UTC)"));
    gl->addWidget(mZoneDisplay, 0, 0);
    connect(mZoneDisplay, SIGNAL(textChanged(const QString &)), SIGNAL(zoneChanged(const QString &)));

    QPushButton *b = new QPushButton(QIcon::fromTheme("document-edit"), i18nc("@action:button", "Change..."), this);
    gl->addWidget(b, 0, 1);
    connect(b, SIGNAL(clicked()), SLOT(slotChangeZone()));
    setFocusProxy(b);
    setFocusPolicy(Qt::StrongFocus);

    mGuessButton = new QPushButton(QIcon::fromTheme("preferences-system-network"), i18nc("@action:button", "Get from Location"), this);
    mGuessButton->setEnabled(false);
    gl->addWidget(mGuessButton, 1, 1);
    connect(mGuessButton, SIGNAL(clicked()), SLOT(slotGuessZone()));
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


void TimeZoneSelector::slotGuessZone()
{
    qDebug() << "for lat" << mItemsLat << "lon" << mItemsLon;

    TimeZoneProvider *job = new TimeZoneProvider(mItemsLat, mItemsLon, this);
    if (!job->isValid()) return;			// failed to start job
    connect(job, SIGNAL(result(const QString &)), SLOT(slotGuessJobFinished(const QString &)));
}


void TimeZoneSelector::setItems(const QList<TrackDataItem *> *items)
{
    BoundingArea bb = TrackData::unifyBoundingAreas(items);
    if (!bb.isValid()) return;

    mItemsLat = (bb.north()+bb.south())/2;		// centre of area of interest
    mItemsLon = (bb.west()+bb.east())/2;
    mGuessButton->setEnabled(true);			// now can use this
}


void TimeZoneSelector::slotGuessJobFinished(const QString &zone)
{
    if (!zone.isEmpty()) mZoneDisplay->setText(zone);
}
