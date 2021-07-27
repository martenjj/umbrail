// -*-mode:c++ -*-

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
#ifdef SORTABLE_VIEW
class QSortFilterProxyModel;
#endif
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
    void slotMapDraggedPoints(qreal latOff, qreal lonOff);

private:
    FilesView *mView;
    FilesModel *mDataModel;
#ifdef SORTABLE_VIEW
    QSortFilterProxyModel *mProxyModel;
#endif
    bool mWarnedNoTimezone;
    bool mSettingTimeZone;
};
 
#endif							// FILESCONTROLLER_H
