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

#ifndef ABSTRACTCOORDINATEHANDLER_H
#define ABSTRACTCOORDINATEHANDLER_H

#include <qobject.h>

// For plugin implementation details and example, see
// the QPluginLoader API documentation and
// http://doc.qt.io/qt-5/qtwidgets-tools-echoplugin-example.html

#ifndef PLUGIN_EXPORT
#define PLUGIN_EXPORT
#endif


class AbstractCoordinateHandler;
Q_DECLARE_INTERFACE(AbstractCoordinateHandler, ACH_PLUGIN_IID)


class PLUGIN_EXPORT AbstractCoordinateHandler : public QObject
{
    Q_OBJECT
    Q_INTERFACES(AbstractCoordinateHandler)

public:
    virtual ~AbstractCoordinateHandler() = default;

    virtual QWidget *createWidget(QWidget *pnt = nullptr) = 0;
    virtual bool hasAcceptableInput() const = 0;
    virtual QString tabName() const = 0;

    void setLatLong(double lat, double lon);
    double getLatitude() const				{ return (mLatitude); }
    double getLongitude() const				{ return (mLongitude); }

signals:
    void valueChanged();
    void statusMessage(const QString &msg);

protected:
    AbstractCoordinateHandler(QObject *pnt = nullptr);

    virtual void updateGUI(double lat, double lon) = 0;
    void updateValues(double lat, double lon);
    virtual void checkError();
    void setError(const QString &msg);

private:
    double mLatitude;
    double mLongitude;
};

#endif							// ABSTRACTCOORDINATEHANDLER_H
