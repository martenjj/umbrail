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

    QString save(KConfig *conf);
    QString load(const KConfig *conf);
    void clear();
    QStringList modifiedFiles() const;

    static QString allImportFilters();
    static QString allExportFilters();
    static QString allProjectFilters(bool includeAllFiles);

public slots:               
    void slotTrackProperties();

protected slots:

signals:
    void statusMessage(const QString &text);
    void modified();
    void updateActionState();
    void updateMap();

private:
    MainWindow *mainWindow() const;

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
