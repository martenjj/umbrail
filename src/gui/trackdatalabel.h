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

#ifndef TRACKDATALABEL_H
#define TRACKDATALABEL_H

#include <qlabel.h>
#include <qdatetime.h>

class QTimeZone;
class TrackDataItem;


class TrackDataLabel : public QLabel
{
    Q_OBJECT

public:
    explicit TrackDataLabel(const QString &str, QWidget *pnt = nullptr);
    explicit TrackDataLabel(int i, QWidget *pnt = nullptr);
    explicit TrackDataLabel(const QDateTime &dt, QWidget *pnt = nullptr);
    explicit TrackDataLabel(double lat, double lon, bool blankIfUnknown, QWidget *pnt = nullptr);
    virtual ~TrackDataLabel() = default;

    void setDateTime(const QDateTime &dt);
    void setTimeZone(const QTimeZone *tz);

private:
    void init();
    void updateDateTime();

private:
    QDateTime mDateTime;
    const QTimeZone *mTimeZone;
};

#endif							// TRACKDATALABEL_H
