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

#ifndef LATLONGWIDGET_H
#define LATLONGWIDGET_H

#include <qframe.h>
#include <qvector.h>

class QTabWidget;
class AbstractCoordinateHandler;


class LatLongWidget : public QFrame
{
    Q_OBJECT

public:
    explicit LatLongWidget(QWidget *pnt = nullptr);
    virtual ~LatLongWidget();

    void setLatLong(double lat, double lon);
    double latitude() const			{ return (mLatitude); }
    double longitude() const			{ return (mLongitude); }

    bool hasAcceptableInput() const;

protected slots:
    void slotPasteCoordinates();

signals:
    void positionChanged(double lat, double lon);
    void positionValid(bool valid);

private slots:
    void slotValueChanged();

private:
    void textChanged();

private:
    QTabWidget *mTabs;

    double mLatitude;
    double mLongitude;

    QVector<AbstractCoordinateHandler *> mHandlers;
};

#endif							// LATLONGWIDGET_H
