//////////////////////////////////////////////////////////////////////////
//									//
//  Project:	Umbrail - GPX track viewer and editor			//
//									//
//////////////////////////////////////////////////////////////////////////
//									//
//  Copyright (c) 2014-2022 Jonathan Marten <jjm@keelhaul.me.uk>	//
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

#ifndef TRACKPROPERTIESGENERALPAGES_H
#define TRACKPROPERTIESGENERALPAGES_H

#include "trackpropertiespage.h"
#include "trackdata.h"

class QLabel;
class QDateTime;
class QFormLayout;
class QComboBox;
class QLineEdit;

class KTextEdit;

class ItemTypeCombo;
class TrackDataItem;
class TrackDataAbstractPoint;
class TrackDataWaypoint;
class TimeZoneSelector;
class TrackDataLabel;


class TrackItemGeneralPage : public TrackPropertiesPage
{
    Q_OBJECT

public:
    virtual ~TrackItemGeneralPage() = default;

    virtual QString typeText(int count) const = 0;
    virtual bool isDataValid() const override;
    virtual void refreshData() override;

protected:
    TrackItemGeneralPage(const QList<TrackDataItem *> *items, QWidget *pnt);

    void addTimeSpanFields(const QList<TrackDataItem *> *items);
    void addTypeField(const QList<TrackDataItem *> *items);
    void addDescField(const QList<TrackDataItem *> *items);
    void addPositionFields(const QList<TrackDataItem *> *items);
    void addTimeField(const QList<TrackDataItem *> *items);

protected slots:
    void slotNameChanged(const QString &text);
    void slotTypeChanged(const QString &text);
    void slotDescChanged();

    void slotChangePosition();

signals:
    void timeZoneChanged(const QString &zone);
    void pointPositionChanged(double newLat, double newLon);

protected:
    QLineEdit *mNameEdit;
    ItemTypeCombo *mTypeCombo;
    KTextEdit *mDescEdit;
    QLabel *mPositionLabel;

    TrackDataLabel *mTimeLabel;
    TrackDataLabel *mTimeStartLabel;
    TrackDataLabel *mTimeEndLabel;

private:
    bool mHasExplicitName;
};


class TrackFileGeneralPage : public TrackItemGeneralPage
{
    Q_OBJECT

public:
    TrackFileGeneralPage(const QList<TrackDataItem *> *items, QWidget *pnt);
    virtual ~TrackFileGeneralPage() = default;

    bool isDataValid() const override;
    QString typeText(int count) const override;
    void refreshData() override;

private slots:
    void slotTimeZoneChanged(const QString &zoneName);

private:
    QLineEdit *mUrlRequester;
    TimeZoneSelector *mTimeZoneSel;
};


class TrackTrackGeneralPage : public TrackItemGeneralPage
{
    Q_OBJECT

public:
    TrackTrackGeneralPage(const QList<TrackDataItem *> *items, QWidget *pnt);
    virtual ~TrackTrackGeneralPage() = default;

    QString typeText(int count) const override;
};


class TrackSegmentGeneralPage : public TrackItemGeneralPage
{
    Q_OBJECT

public:
    TrackSegmentGeneralPage(const QList<TrackDataItem *> *items, QWidget *pnt);
    virtual ~TrackSegmentGeneralPage() = default;

    QString typeText(int count) const override;
};


class TrackTrackpointGeneralPage : public TrackItemGeneralPage
{
    Q_OBJECT

public:
    TrackTrackpointGeneralPage(const QList<TrackDataItem *> *items, QWidget *pnt);
    virtual ~TrackTrackpointGeneralPage() = default;

    QString typeText(int count) const override;
};


class TrackFolderGeneralPage : public TrackItemGeneralPage
{
    Q_OBJECT

public:
    TrackFolderGeneralPage(const QList<TrackDataItem *> *items, QWidget *pnt);
    virtual ~TrackFolderGeneralPage() = default;

    QString typeText(int count) const override;
};


class TrackWaypointGeneralPage : public TrackItemGeneralPage
{
    Q_OBJECT

public:
    TrackWaypointGeneralPage(const QList<TrackDataItem *> *items, QWidget *pnt);
    virtual ~TrackWaypointGeneralPage() = default;

    QString typeText(int count) const override;
    void refreshData() override;

private:
    void addStatusField(const QList<TrackDataItem *> *items);

private slots:
    void slotStatusChanged(int idx);

private:
    QComboBox *mStatusCombo;
};


class TrackRouteGeneralPage : public TrackItemGeneralPage
{
    Q_OBJECT

public:
    TrackRouteGeneralPage(const QList<TrackDataItem *> *items, QWidget *pnt);
    virtual ~TrackRouteGeneralPage() = default;

    QString typeText(int count) const override;
};


class TrackRoutepointGeneralPage : public TrackItemGeneralPage
{
    Q_OBJECT

public:
    TrackRoutepointGeneralPage(const QList<TrackDataItem *> *items, QWidget *pnt);
    virtual ~TrackRoutepointGeneralPage() = default;

    QString typeText(int count) const override;
};

#endif							// TRACKPROPERTIESGENERALPAGES_H
