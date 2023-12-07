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

#ifndef TIMEZONEDISPLAY_H
#define TIMEZONEDISPLAY_H

#include <qframe.h>


class QLineEdit;
class QPushButton;
class TrackDataItem;


class TimeZoneDisplay : public QFrame
{
    Q_OBJECT

public:
    explicit TimeZoneDisplay(QWidget *pnt = nullptr);
    virtual ~TimeZoneDisplay() = default;

    void setTimeZone(const QString &zone);
    QString timeZone() const;

    void setItems(const QList<TrackDataItem *> *items);

protected slots:
    void slotChangeZone();
    void slotGuessZone();

private slots:
    void slotGuessJobFinished(const QString &zone);

signals:
    void zoneChanged(const QString &zone);

private:
    QLineEdit *mZoneDisplay;
    QPushButton *mGuessButton;
    double mItemsLat;
    double mItemsLon;
};
 
#endif							// TIMEZONEDISPLAY_H
