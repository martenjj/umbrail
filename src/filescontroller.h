// -*-mode:c++ -*-

#ifndef FILESCONTROLLER_H
#define FILESCONTROLLER_H
 
#include <math.h>

#include <qobject.h>


class KConfig;
class KUrl;

class FilesView;
class FilesModel;
class MainWindow;
class TrackDataFile;
class ErrorReporter;

#ifdef SORTABLE_VIEW
class QSortFilterProxyModel;
#endif


class FilesController : public QObject
{
    Q_OBJECT

public:
    FilesController(QObject *pnt = NULL);
    ~FilesController();

    FilesView *view() const			{ return (mView); }
    FilesModel *model() const			{ return (mDataModel); }

    void readProperties();
    void saveProperties();

    bool importFile(const KUrl &importFrom);
    bool exportFile(const KUrl &exportTo, const TrackDataFile *tdf);
    bool importPhoto(const KUrl &importFrom);

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

signals:
    void statusMessage(const QString &text);
    void modified();
    void updateActionState();
    void updateMap();

private:
    MainWindow *mainWindow() const;
    bool reportFileError(bool saving, const KUrl &file, const QString &msg);
    bool reportFileError(bool saving, const KUrl &file, const ErrorReporter *rep);

private slots:
    void slotUpdateActionState();

private:
    FilesView *mView;
    FilesModel *mDataModel;
#ifdef SORTABLE_VIEW
    QSortFilterProxyModel *mProxyModel;
#endif
};
 
#endif							// FILESCONTROLLER_H
