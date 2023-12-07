//////////////////////////////////////////////////////////////////////////
//									//
//  Project:	Umbrail - GPX track viewer and editor			//
//									//
//////////////////////////////////////////////////////////////////////////
//									//
//  Copyright (c) 2014-2023 Jonathan Marten <jjm@keelhaul.me.uk>	//
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

#include "timezonesettingdialogue.h"

#include <qlabel.h>
#include <qboxlayout.h>
#include <qicon.h>

#include <klocalizedstring.h>
#include <kmessagewidget.h>

#include "timezonedisplay.h"


TimeZoneSettingDialogue::TimeZoneSettingDialogue(QWidget *pnt)
    : DialogBase(pnt),
      ApplicationDataInterface(pnt)
{
    setObjectName("TimeZoneSettingDialogue");

    setModal(true);
    setButtons(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
    setWindowTitle(i18n("Set File Time Zone"));

    QWidget *w = new QWidget(this);
    QVBoxLayout *vb = new QVBoxLayout(w);

    QLabel *l = new QLabel(i18n("File time zone:"), w);
    vb->addWidget(l);

    mTimeZoneDisplay = new TimeZoneDisplay(this);
    vb->addWidget(mTimeZoneDisplay);

    vb->addSpacerItem(DialogBase::verticalSpacerItem());

    if (isReadOnly())
    {
        KMessageWidget *msg = new KMessageWidget(this);
        msg->setMessageType(KMessageWidget::Warning);
        msg->setText(i18n("The file is read only.  The time zone setting will only apply to this session and will not be saved."));
        msg->setIcon(QIcon::fromTheme("dialog-warning"));
        msg->setCloseButtonVisible(false);
        msg->setWordWrap(true);
        vb->addWidget(msg);
    }

    vb->addStretch(1);

    setMainWidget(w);
    setMinimumSize(QSize(380, 250));
}


void TimeZoneSettingDialogue::setItems(const QList<TrackDataItem *> *items)
{
    mTimeZoneDisplay->setItems(items);   
}


void TimeZoneSettingDialogue::setTimeZone(const QString &zone)
{
    mTimeZoneDisplay->setTimeZone(zone);
}


QString TimeZoneSettingDialogue::timeZone() const
{
    return (mTimeZoneDisplay->timeZone());
}
