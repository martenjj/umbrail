// -*-mode:c++ -*-

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <kxmlguiwindow.h>

#include "filescontroller.h"


class QLabel;
class QUndoStack;
class QUndoCommand;
class QSplitter;
class QAction;

class QCloseEvent;
class QDragEnterEvent;
class QDropEvent;
class QMimeData;
class QUrl;

class KToggleAction;
class KSelectAction;
class KSqueezedTextLabel;

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

    bool loadProject(const QUrl &loadFrom);

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

    bool save(const QUrl &to);
    FilesController::Status load(const QUrl &from);

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

    QAction *mSaveProjectAction;
    QAction *mSaveProjectAsAction;
    QAction *mExportAction;
    QAction *mImportAction;

    QAction *mPasteAction;

    QAction *mUndoAction;
    QAction *mRedoAction;
    QString mUndoText;
    QString mRedoText;

    QAction *mSelectAllAction;
    QAction *mClearSelectAction;
    QAction *mAddTrackAction;
    QAction *mAddFolderAction;
    QAction *mAddPointAction;
    QAction *mAddWaypointAction;
    QAction *mDeleteItemsAction;

    KSelectAction *mWaypointStatusAction;

    QAction *mSplitTrackAction;
    QAction *mMergeTrackAction;
    QAction *mMoveItemAction;
    QAction *mStopDetectAction;
    QAction *mPropertiesAction;
    QAction *mProfileAction;
    QAction *mStatisticsAction;

    QAction *mPlayMediaAction;
    QAction *mOpenMediaAction;
    QAction *mSaveMediaAction;

    QAction *mMapZoomInAction;
    QAction *mMapZoomOutAction;
    QAction *mMapGoToAction;
    KToggleAction *mMapDragAction;

    QSplitter *mSplitter;
    QUndoStack *mUndoStack;

    const TrackDataItem *mSelectedContainer;
};
 
#endif							// MAINWINDOW_H
