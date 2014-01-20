// -*-mode:c++ -*-

#ifndef MAINWINDOW_H
#define MAINWINDOW_H
 
#include <kxmlguiwindow.h>
#include <kurl.h>

class QLabel;
class QCloseEvent;
class QUndoStack;
class QSplitter;

class KAction;
class KSqueezedTextLabel;
class KUrl;

class FilesController;
class MapController;
class Project;
class CommandBase;


class MainWindow : public KXmlGuiWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = NULL);
    ~MainWindow();

    MapController *mapController() const	{ return (mMapController); }
    FilesController *filesController() const	{ return (mFilesController); }

    void loadProject(const KUrl &loadFrom);

    void executeCommand(CommandBase *cmd);

public slots:               
    void slotStatusMessage(const QString &text);
    void slotSetModified(bool mod = true);

protected:
    virtual void saveProperties(KConfigGroup &grp);
    virtual void readProperties(const KConfigGroup &grp);
    virtual void closeEvent(QCloseEvent *ev);
    virtual bool queryClose();

protected slots:
    void slotNewProject();
    void slotOpenProject();
    void slotSaveProject();
    void slotSaveAs();
    void slotExportFile();
    void slotImportFile();

    void slotCanUndoChanged(bool can);
    void slotCanRedoChanged(bool can);
    void slotUndoTextChanged(const QString &text);
    void slotRedoTextChanged(const QString &text);
    void slotCleanUndoChanged(bool clean);

    void slotMapZoomChanged(bool canZoomIn, bool canZoomOut);

private:
    void init();
    void setupActions();
    void setupStatusBar();

    bool save(const KUrl &to);
    bool load(const KUrl &from);
    void reportFileError(bool saving,const QString &msg);
    void clear();

private slots:
    void slotUpdateActionState();

private:
    Project *mProject;
    FilesController *mFilesController;
    MapController *mMapController;

    KSqueezedTextLabel *mStatusMessage;
    QLabel *mModifiedIndicator;

    KAction *mSaveProjectAction;
    KAction *mExportAction;
    KAction *mImportAction;

    KAction *mUndoAction;
    KAction *mRedoAction;
    QString mUndoText;
    QString mRedoText;

    KAction *mSelectAllAction;
    KAction *mClearSelectAction;

    KAction *mMapZoomInAction;
    KAction *mMapZoomOutAction;

    QSplitter *mSplitter;
    QUndoStack *mUndoStack;
};
 
#endif							// MAINWINDOW_H
