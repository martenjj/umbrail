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

#ifndef TIMEZONESETTINGDIALOGUE_H
#define TIMEZONESETTINGDIALOGUE_H

#include <kfdialog/dialogbase.h>

#include "applicationdatainterface.h"

class TimeZoneDisplay;
class TrackDataItem;


class TimeZoneSettingDialogue : public DialogBase, public ApplicationDataInterface
{
    Q_OBJECT

public:
    TimeZoneSettingDialogue(QWidget *pnt = nullptr);
    virtual ~TimeZoneSettingDialogue() = default;

    void setTimeZone(const QString &zone);
    QString timeZone() const;
    void setItems(const QList<TrackDataItem *> *items);

private:
    TimeZoneDisplay *mTimeZoneDisplay;
};

#endif							// TIMEZONESELECTDIALOGUE_H
