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

#ifndef TIMEZONEPROVIDER_H
#define TIMEZONEPROVIDER_H

#include <qobject.h>
#include <kio/job.h>


class TimeZoneProvider : public QObject
{
    Q_OBJECT

public:
    TimeZoneProvider(double lat, double lon, QObject *pnt = nullptr);
    virtual ~TimeZoneProvider() = default;

    QString timeZone() const			{ return (mTimeZone); }
    bool isValid() const			{ return (mJobReady); }

signals:
    void result(const QString &timeZone);

protected slots:
    void slotDataReceived(KIO::Job *job, const QByteArray &data);
    void slotDataResult(KJob *job);

private:
    void reportError(const QString &msg);

private:
    QByteArray mReceivedData;
    QString mTimeZone;
    QUrl mUrl;
    bool mJobReady;
};

#endif							// TIMEZONEPROVIDER_H
