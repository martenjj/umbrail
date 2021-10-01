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

#ifndef STATISTICSWIDGET_H
#define STATISTICSWIDGET_H
 
#include <kfdialog/dialogbase.h>
#include "applicationdatainterface.h"


class QGridLayout;
class TrackDataAbstractPoint;


class StatisticsWidget : public DialogBase, public ApplicationDataInterface
{
    Q_OBJECT

public:
    explicit StatisticsWidget(QWidget *pnt = nullptr);
    virtual ~StatisticsWidget() = default;

private:
    void getPointData(const TrackDataAbstractPoint *point);
    void addRow(const QString &text, int num, bool withPercent = true);

private:
    QWidget *mWidget;
    QGridLayout *mLayout;

    int mTotalPoints;
    int mWithTime;
    int mWithElevation;
    int mWithGpsSpeed;
    int mWithGpsHdop;
    int mWithGpsHeading;
};

#endif							// STATISTICSWIDGET_H
