// -*-mode:c++ -*-

#ifndef FILESCONTROLLER_H
#define FILESCONTROLLER_H
 
#include <qobject.h>

//#include "pointdata.h"


//class QItemSelection;
//class QUndoStack;

class KConfig;
class KConfigGroup;
class KUrl;

class FilesView;
class FilesModel;
//class ImporterBase;
//class ExporterBase;
//class CommandBase;
class MainWindow;


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

    void readProperties(const KConfigGroup &grp);
    void saveProperties(KConfigGroup &grp);

    void importFile(const KUrl &importFrom);
    void exportFile(const KUrl &exportTo);

//    ImporterBase *createGpxImporter();
//    ExporterBase *createGpxExporter();

    QString save(KConfig *conf);
    QString load(const KConfig *conf);
    void clear();

    static QString allImportFilters();
    static QString allProjectFilters(bool includeAllFiles);

public slots:               
//    void slotNewPoint();
//    void slotDeleteSelection();
//    void slotMergeSelection();
    void slotTrackProperties();
//    void slotManageCategories();
//    void slotManageIcons();
//    void slotManageSources();

protected slots:

signals:
    void statusMessage(const QString &text);
    void modified();
    void updateActionState();

private:
//    void executeCommand(CommandBase *cmd);
    MainWindow *mainWindow() const;

private slots:
    void slotUpdateActionState();

private:
    FilesView *mView;
    FilesModel *mDataModel;
#ifdef SORTABLE_VIEW
    QSortFilterProxyModel *mProxyModel;
#endif
//    QUndoStack *mUndoStack;
};
 
#endif							// FILESCONTROLLER_H
