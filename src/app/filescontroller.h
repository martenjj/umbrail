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

#ifndef FILESCONTROLLER_H
#define FILESCONTROLLER_H
 
#include <qobject.h>
#include "applicationdatainterface.h"

#include <math.h>					// need this for 'NAN'


class QDateTime;
class QUrl;

class FilesView;
class PointsView;
class FilesModel;
class WaypointsFilterModel;
class HomePointsDataModel;

class TrackDataFile;
class TrackDataItem;
class TrackDataContainer;

class ErrorReporter;
class ImporterExporterOptions;


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
    FilesController(QObject *pnt = nullptr);
    virtual ~FilesController();

    // File loading or saving status
    enum Status
    {
        StatusOk,
        StatusResave,
        StatusFailed,
        StatusCancelled
    };

    FilesView *filesView() const		{ return (mFilesView); }
    PointsView *pointsView() const		{ return (mPointsView); }
    FilesModel *filesModel() const		{ return (mFilesModel); }
    HomePointsDataModel *homePointsModel();

    void readProperties();
    void saveProperties();

    FilesController::Status importFile(const QUrl &importFrom, const ImporterExporterOptions &options);
    FilesController::Status exportFile(const QUrl &exportTo, const ImporterExporterOptions &options);
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
    void slotMergeItems();
    void slotMoveItem();
    void slotAddTrack();
    void slotAddRoute();
    void slotAddFolder();
    void slotAddTrackpoint();
    void slotDeleteItems();
    void slotAddWaypoint(qreal lat = NAN, qreal lon = NAN);
    void slotAddRoutepoint(qreal lat = NAN, qreal lon = NAN);
    void slotSetWaypointStatus();
    void slotCheckTimeZone();
    void slotSetTimeZone();
    void slotManageCategories();

    void slotMapDraggedPoints(qreal latOff, qreal lonOff);

signals:
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

    void mergeSegmentsInternal(const QList<TrackDataItem *> &items);
    void mergeWaypointsInternal(const QList<TrackDataItem *> &items);

private slots:
    void slotUpdateActionState();
    void slotDragDropItems(const QList<TrackDataItem *> &sourceItems, TrackDataContainer *ontoParent, int row);

private:
    FilesView *mFilesView;
    PointsView *mPointsView;
    FilesModel *mFilesModel;

    WaypointsFilterModel *mWaypointsFilterModel;
    HomePointsDataModel *mHomePointsModel;

    bool mWarnedNoTimezone;
};
 
#endif							// FILESCONTROLLER_H
