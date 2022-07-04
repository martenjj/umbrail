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

#ifndef FILESCONTROLLER_H
#define FILESCONTROLLER_H
 
#include <qobject.h>
#include "applicationdatainterface.h"
#include "importerexporterbase.h"

#include <math.h>					// need this for 'NAN'


class QDateTime;
class QUrl;

class FilesView;
class FilesModel;
class TrackDataFile;
class ErrorReporter;
class TrackDataItem;


class DialogueConstraintFilter : public QObject
{
    Q_OBJECT

public:
    DialogueConstraintFilter(QObject *pnt) : QObject(pnt)	{}
    virtual ~DialogueConstraintFilter()				{}
    bool eventFilter(QObject *obj, QEvent *ev) override;
};


class FilesController : public QObject, public ApplicationDataInterface
{
    Q_OBJECT

public:

    // File loading or saving status
    enum Status
    {
        StatusOk,
        StatusResave,
        StatusFailed,
        StatusCancelled
    };

    FilesController(QObject *pnt = nullptr);
    virtual ~FilesController();

    FilesView *view() const			{ return (mView); }
    FilesModel *model() const			{ return (mDataModel); }
    bool isSettingTimeZone() const		{ return (mSettingTimeZone); }

    void readProperties();
    void saveProperties();

    FilesController::Status importFile(const QUrl &importFrom);
    FilesController::Status exportFile(const QUrl &exportTo, const TrackDataFile *tdf, ImporterExporterBase::Options options);
    FilesController::Status importPhoto(const QList<QUrl> &urls);
    void initNew();

    void doUpdateMap()				{ emit updateMap(); }

    static QString allImportFilters();
    static QString allExportFilters();
    static QString allProjectFilters(bool includeAllFiles);

    static void resetAllFileWarnings();

public slots:               
    void slotTrackProperties();
    void slotSplitSegment();
    void slotMergeSegments();
    void slotMoveItem();
    void slotAddTrack();
    void slotAddRoute();
    void slotAddFolder();
    void slotAddPoint();
    void slotDeleteItems();
    void slotAddWaypoint(qreal lat = NAN, qreal lon = NAN);
    void slotAddRoutepoint(qreal lat = NAN, qreal lon = NAN);
    void slotSetWaypointStatus();
    void slotCheckTimeZone();
    void slotSetTimeZone();

    void slotMapDraggedPoints(qreal latOff, qreal lonOff);

signals:
    void statusMessage(const QString &text);
    void modified();
    void updateActionState();
    void updateMap();

private:
    bool reportFileError(bool saving, const QUrl &file, const QString &msg);
    bool reportFileError(bool saving, const QUrl &file, const ErrorReporter *rep);

    static bool fileWarningIgnored(const QUrl &file, const QByteArray &type);
    static void setFileWarningIgnored(const QUrl &file, const QByteArray &type);

    bool adjustTimeSpec(QDateTime &dt);
    FilesController::Status importPhotoInternal(const QUrl &importFrom, bool multiple);

private slots:
    void slotUpdateActionState();
    void slotDragDropItems(const QList<TrackDataItem *> &sourceItems, TrackDataItem *ontoParent, int row);

private:
    FilesView *mView;
    FilesModel *mDataModel;
    bool mWarnedNoTimezone;
    bool mSettingTimeZone;
};
 
#endif							// FILESCONTROLLER_H
