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

#ifndef STOPDETECTDIALOGUE_H
#define STOPDETECTDIALOGUE_H


#include <kfdialog/dialogbase.h>
#include "applicationdatainterface.h"

#include <qtimezone.h>


class QTimer;
class QListWidget;
class QLineEdit;
class QPushButton;
class QShowEvent;
class ValueSlider;
class TrackDataItem;
class TrackDataAbstractPoint;
class TrackDataWaypoint;
class FolderSelectWidget;


class StopDetectDialogue : public DialogBase, public ApplicationDataInterface
{
    Q_OBJECT

public:
    explicit StopDetectDialogue(QWidget *pnt = nullptr);
    virtual ~StopDetectDialogue();

protected:
    void showEvent(QShowEvent *ev) override;

protected slots:
    void slotShowOnMap();
    void slotMergeStops();
    void slotCommitResults();

private slots:
    void slotDetectStops();
    void slotSetButtonStates();

private:
    void updateResults();

private:
    QListWidget *mResultsList;
    ValueSlider *mTimeSlider;
    ValueSlider *mDistanceSlider;
    ValueSlider *mNoiseSlider;
    QPushButton *mShowOnMapButton;
    QPushButton *mMergeStopsButton;
    FolderSelectWidget *mFolderSelect;

    QTimer *mIdleTimer;

    QTimeZone mTimeZone;

    QVector<const TrackDataAbstractPoint *> mInputPoints;
    QList<const TrackDataWaypoint *> mResultPoints;
};

#endif							// STOPDETECTDIALOGUE_H
