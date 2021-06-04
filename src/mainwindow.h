// -*-mode:c++ -*-

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <kxmlguiwindow.h>

#include "filescontroller.h"
#include "importerexporterbase.h"


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
    MainWindow(QWidget *pnt = nullptr);
    ~MainWindow();

    MapController *mapController() const	{ return (mMapController); }
    FilesController *filesController() const	{ return (mFilesController); }
    bool isReadOnly() const			{ return (mReadOnly); }

    bool loadProject(const QUrl &loadFrom);

    void executeCommand(QUndoCommand *cmd);

public slots:               
    void slotStatusMessage(const QString &text);
    void slotSetModified(bool mod = true);

protected:
    void saveProperties(KConfigGroup &grp) override;
    void readProperties(const KConfigGroup &grp) override;
    bool queryClose() override;

    void closeEvent(QCloseEvent *ev) override;
    void dragEnterEvent(QDragEnterEvent *ev) override;
    void dropEvent(QDropEvent *ev) override;

protected slots:
    void slotNewProject();
    void slotOpenProject();
    void slotSaveProject();
    void slotSaveAs();
    void slotSaveCopy();
    void slotExportFile();
    void slotImportFile();
    void slotPreferences();
    void slotImportPhoto();
    void slotCopy();
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

    void slotResetAndCancel();
    void slotReadOnly(bool on);

private:
    void init();
    void setupActions();
    void setupStatusBar();

    bool save(const QUrl &to, ImporterExporterBase::Options options);
    FilesController::Status load(const QUrl &from);

    bool acceptMimeData(const QMimeData *mimeData);

private slots:
    void slotUpdateActionState();
    void slotUpdatePasteState();

private:
    Project *mProject;
    FilesController *mFilesController;
    MapController *mMapController;
    bool mReadOnly;

    KSqueezedTextLabel *mStatusMessage;
    QLabel *mModifiedIndicator;

    QAction *mSaveProjectAction;
    QAction *mSaveProjectAsAction;
    QAction *mSaveProjectCopyAction;
    QAction *mExportAction;
    QAction *mImportAction;
    QAction *mPhotoAction;

    QAction *mCopyAction;
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
    QAction *mAddRoutepointAction;
    QAction *mAddRouteAction;
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
