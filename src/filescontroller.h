// -*-mode:c++ -*-

#ifndef FILESCONTROLLER_H
#define FILESCONTROLLER_H
 
#include <qobject.h>

class KConfig;
class KUrl;

class FilesView;
class FilesModel;
class MainWindow;
class TrackDataFile;

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

    static QString allImportFilters();
    static QString allExportFilters();
    static QString allProjectFilters(bool includeAllFiles);

public slots:               
    void slotTrackProperties();
    void slotSplitSegment();
    void slotMergeSegments();
    void slotMoveSegment();
    void slotAddTrack();
    void slotAddPoint();
    void slotDeleteItems();

signals:
    void statusMessage(const QString &text);
    void modified();
    void updateActionState();
    void updateMap();

private:
    MainWindow *mainWindow() const;
    void reportFileError(bool saving, const QString &msg);

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
