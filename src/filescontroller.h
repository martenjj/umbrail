// -*-mode:c++ -*-

#ifndef FILESCONTROLLER_H
#define FILESCONTROLLER_H
 
#include <qobject.h>
#include "mainwindowinterface.h"

#include <math.h>
#include <kurl.h>

class QDateTime;

class KConfig;

class FilesView;
class FilesModel;
class TrackDataFile;
class ErrorReporter;

#ifdef SORTABLE_VIEW
class QSortFilterProxyModel;
#endif


class FilesController : public QObject, public MainWindowInterface
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

    FilesController(QObject *pnt = NULL);
    ~FilesController();

    FilesView *view() const			{ return (mView); }
    FilesModel *model() const			{ return (mDataModel); }

    void readProperties();
    void saveProperties();

    FilesController::Status importFile(const KUrl &importFrom);
    FilesController::Status exportFile(const KUrl &exportTo, const TrackDataFile *tdf);
    FilesController::Status importPhoto(const KUrl::List &urls);

    static QString allImportFilters();
    static QString allExportFilters();
    static QString allProjectFilters(bool includeAllFiles);


public slots:               
    void slotTrackProperties();
    void slotSplitSegment();
    void slotMergeSegments();
    void slotMoveItem();
    void slotAddTrack();
    void slotAddFolder();
    void slotAddPoint();
    void slotDeleteItems();
    void slotAddWaypoint(qreal lat = NAN, qreal lon = NAN);
    void slotSetWaypointStatus();

signals:
    void statusMessage(const QString &text);
    void modified();
    void updateActionState();
    void updateMap();

private:
    bool reportFileError(bool saving, const KUrl &file, const QString &msg);
    bool reportFileError(bool saving, const KUrl &file, const ErrorReporter *rep);

    bool fileWarningsIgnored(const KUrl &file) const;
    void setFileWarningsIgnored(const KUrl &file, bool ignore = true);

    bool adjustTimeSpec(QDateTime &dt);
    FilesController::Status importPhotoInternal(const KUrl &importFrom, bool multiple);

private slots:
    void slotUpdateActionState();

private:
    FilesView *mView;
    FilesModel *mDataModel;
#ifdef SORTABLE_VIEW
    QSortFilterProxyModel *mProxyModel;
#endif
    bool mWarnedNoTimezone;
};
 
#endif							// FILESCONTROLLER_H
