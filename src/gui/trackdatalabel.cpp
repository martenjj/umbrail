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

#include "trackdatalabel.h"

#include <qdebug.h>
#include <qtimezone.h>

#include <klocalizedstring.h>

#include "trackdata.h"


TrackDataLabel::TrackDataLabel(const QString &str, QWidget *pnt)
    : QLabel(str, pnt)
{
    init();
}


TrackDataLabel::TrackDataLabel(const QDateTime &dt, QWidget *pnt)
    : QLabel(pnt)
{
    mDateTime = dt;
    init();
}


void TrackDataLabel::setDateTime(const QDateTime &dt)
{
    mDateTime = dt;
    updateDateTime();
}


void TrackDataLabel::setTimeZone(const QTimeZone *tz)
{
    mTimeZone = tz;
    updateDateTime();
}


void TrackDataLabel::updateDateTime()
{
    if (mDateTime.isNull()) setText(QString());
    else if (!mDateTime.isValid()) setText(i18nc("an invalid time", "(invalid)"));
    else if (mTimeZone!=nullptr && !mTimeZone->isValid()) setText(i18nc("an invalid time zone", "(invalid time zone)"));
    else setText(TrackData::formattedTime(mDateTime, mTimeZone));
}


TrackDataLabel::TrackDataLabel(double lat, double lon, bool blankIfUnknown, QWidget *pnt)
    : QLabel(TrackData::formattedLatLong(lat, lon, blankIfUnknown), pnt)
{
    init();
}


TrackDataLabel::TrackDataLabel(int i, QWidget *pnt)
    : QLabel(QString::number(i), pnt)
{
    init();
}


void TrackDataLabel::init()
{
    mTimeZone = nullptr;
    setTextInteractionFlags(Qt::TextSelectableByMouse|Qt::TextSelectableByKeyboard);
}
