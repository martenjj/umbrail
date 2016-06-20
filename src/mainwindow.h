// -*-mode:c++ -*-

#ifndef MAINWINDOW_H
#define MAINWINDOW_H


#include <kxmlguiwindow.h>
#include <kurl.h>

#include "filescontroller.h"

class QLabel;
class QUndoStack;
class QUndoCommand;
class QSplitter;

class QCloseEvent;
class QDragEnterEvent;
class QDropEvent;
class QMimeData;

class KAction;
class KToggleAction;
class KSelectAction;
class KSqueezedTextLabel;
class KUrl;

class MapController;
class Project;
class TrackDataItem;


class MainWindow : public KXmlGuiWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *pnt = NULL);
    ~MainWindow();

    MapController *mapController() const	{ return (mMapController); }
    FilesController *filesController() const	{ return (mFilesController); }

    bool loadProject(const KUrl &loadFrom);

    void executeCommand(QUndoCommand *cmd);

public slots:               
    void slotStatusMessage(const QString &text);
    void slotSetModified(bool mod = true);

protected:
    virtual void saveProperties(KConfigGroup &grp);
    virtual void readProperties(const KConfigGroup &grp);
    virtual bool queryClose();

    virtual void closeEvent(QCloseEvent *ev);
    virtual void dragEnterEvent(QDragEnterEvent *ev);
    virtual void dropEvent(QDropEvent *ev);

protected slots:
    void slotNewProject();
    void slotOpenProject();
    void slotSaveProject();
    void slotSaveAs();
    void slotExportFile();
    void slotImportFile();
    void slotPreferences();
    void slotImportPhoto();
    void slotPaste();

    void slotCanUndoChanged(bool can);
    void slotCanRedoChanged(bool can);
    void slotUndoTextChanged(const QString &text);
    void slotRedoTextChanged(const QString &text);
    void slotCleanUndoChanged(bool clean);

    void slotMapZoomChanged(bool canZoomIn, bool canZoomOut);
    void slotMapGotoSelection();
    void slotMapMovePoints();
    void slotTrackProfile();
    void slotTrackStatistics();
    void slotTrackStopDetect();
    void slotPlayMedia();
    void slotOpenMedia();
    void slotSaveMedia();

private:
    void init();
    void setupActions();
    void setupStatusBar();

    bool save(const KUrl &to);
    FilesController::Status load(const KUrl &from);

    bool acceptMimeData(const QMimeData *mimeData);

private slots:
    void slotUpdateActionState();
    void slotUpdatePasteState();

private:
    Project *mProject;
    FilesController *mFilesController;
    MapController *mMapController;

    KSqueezedTextLabel *mStatusMessage;
    QLabel *mModifiedIndicator;

    KAction *mSaveProjectAction;
    KAction *mSaveProjectAsAction;
    KAction *mExportAction;
    KAction *mImportAction;

    KAction *mPasteAction;

    KAction *mUndoAction;
    KAction *mRedoAction;
    QString mUndoText;
    QString mRedoText;

    KAction *mSelectAllAction;
    KAction *mClearSelectAction;
    KAction *mAddTrackAction;
    KAction *mAddFolderAction;
    KAction *mAddPointAction;
    KAction *mAddWaypointAction;
    KAction *mAddRoutepointAction;
    KAction *mAddRouteAction;
    KAction *mDeleteItemsAction;

    KSelectAction *mWaypointStatusAction;

    KAction *mSplitTrackAction;
    KAction *mMergeTrackAction;
    KAction *mMoveItemAction;
    KAction *mStopDetectAction;
    KAction *mPropertiesAction;
    KAction *mProfileAction;
    KAction *mStatisticsAction;

    KAction *mPlayMediaAction;
    KAction *mOpenMediaAction;
    KAction *mSaveMediaAction;

    KAction *mMapZoomInAction;
    KAction *mMapZoomOutAction;
    KAction *mMapGoToAction;
    KToggleAction *mMapDragAction;

    QSplitter *mSplitter;
    QUndoStack *mUndoStack;

    const TrackDataItem *mSelectedContainer;
};
 
#endif							// MAINWINDOW_H
