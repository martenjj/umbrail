// -*-mode:c++ -*-

#ifndef FILESCONTROLLER_H
#define FILESCONTROLLER_H
 
#include <qobject.h>
#include "mainwindowinterface.h"


class QDateTime;
class QUrl;

class FilesView;
class FilesModel;
class TrackDataFile;
class ErrorReporter;
#ifdef SORTABLE_VIEW
class QSortFilterProxyModel;
#endif


class DialogueConstraintFilter : public QObject
{
    Q_OBJECT

public:
    DialogueConstraintFilter(QObject *pnt) : QObject(pnt)	{}
    virtual ~DialogueConstraintFilter()				{}
    bool eventFilter(QObject *obj, QEvent *ev);
};


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

    FilesController::Status importFile(const QUrl &importFrom);
    FilesController::Status exportFile(const QUrl &exportTo, const TrackDataFile *tdf);
    FilesController::Status importPhoto(const QList<QUrl> &urls);

    static QString allImportFilters();
    static QString allExportFilters();
    static QString allProjectFilters(bool includeAllFiles);

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

signals:
    void statusMessage(const QString &text);
    void modified();
    void updateActionState();
    void updateMap();

private:
    bool reportFileError(bool saving, const QUrl &file, const QString &msg);
    bool reportFileError(bool saving, const QUrl &file, const ErrorReporter *rep);

    bool fileWarningIgnored(const QUrl &file, const QByteArray &type) const;
    void setFileWarningIgnored(const QUrl &file, const QByteArray &type);

    bool adjustTimeSpec(QDateTime &dt);
    FilesController::Status importPhotoInternal(const QUrl &importFrom, bool multiple);

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
