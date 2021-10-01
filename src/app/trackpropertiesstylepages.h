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

#ifndef TRACKPROPERTIESSTYLEPAGES_H
#define TRACKPROPERTIESSTYLEPAGES_H

#include "trackpropertiespage.h"


class QCheckBox;
class KColorButton;
class TrackDataItem;


class TrackItemStylePage : public TrackPropertiesPage
{
    Q_OBJECT

public:
    virtual ~TrackItemStylePage() = default;
    void refreshData() override;

protected:
    TrackItemStylePage(const QList<TrackDataItem *> *items, QWidget *pnt);

    void addLineColourButton(const QString &text = QString());
    void addPointColourButton(const QString &text = QString());

protected:
    KColorButton *mLineColourButton;
    QCheckBox *mLineInheritCheck;

    KColorButton *mPointColourButton;
    QCheckBox *mPointInheritCheck;

    bool mIsTopLevel;

protected slots:
    void slotColourChanged(const QColor &col);
    void slotInheritChanged(bool on);

private:
    QColor getColourData(bool isLine);
    void setColourData(bool isLine, const QColor &col);
    void setColourButtons(KColorButton *colBut, QCheckBox *inheritBut, bool isLine);
};


class TrackFileStylePage : public TrackItemStylePage
{
    Q_OBJECT

public:
    TrackFileStylePage(const QList<TrackDataItem *> *items, QWidget *pnt);
    virtual ~TrackFileStylePage() = default;
};


class TrackTrackStylePage : public TrackItemStylePage
{
    Q_OBJECT

public:
    TrackTrackStylePage(const QList<TrackDataItem *> *items, QWidget *pnt);
    virtual ~TrackTrackStylePage() = default;
};


class TrackSegmentStylePage : public TrackItemStylePage
{
    Q_OBJECT

public:
    TrackSegmentStylePage(const QList<TrackDataItem *> *items, QWidget *pnt);
    virtual ~TrackSegmentStylePage() = default;
};


class TrackWaypointStylePage : public TrackItemStylePage
{
    Q_OBJECT

public:
    TrackWaypointStylePage(const QList<TrackDataItem *> *items, QWidget *pnt);
    virtual ~TrackWaypointStylePage() = default;
};


class TrackRouteStylePage : public TrackItemStylePage
{
    Q_OBJECT

public:
    TrackRouteStylePage(const QList<TrackDataItem *> *items, QWidget *pnt);
    virtual ~TrackRouteStylePage() = default;
};

#endif							// TRACKPROPERTIESSTYLEPAGES_H
