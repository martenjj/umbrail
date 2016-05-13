
#include "mainwindow.h"

#include <string.h>
#include <errno.h>

#include <qapplication.h>
#include <qgraphicsview.h>
#include <qlabel.h>
#include <qsplitter.h>
#include <qundostack.h>
#include <qdatetime.h>
#include <qevent.h>
#include <qclipboard.h>
#include <qmimedata.h>
#include <qstatusbar.h>

#include <kdebug.h>
#include <klocale.h>
#include <kaction.h>
#include <ktoggleaction.h>
#include <kselectaction.h>
#include <kactioncollection.h>
#include <kstandardaction.h>
#include <kfiledialog.h>
#include <kmessagebox.h>
#include <ksqueezedtextlabel.h>
#include <kmimetype.h>
#include <kurl.h>
#include <kactionmenu.h>
#include <kimageio.h>

#include "filescontroller.h"
#include "filesview.h"
#include "filesmodel.h"
#include "dataindexer.h"
#include "mapcontroller.h"
#include "mapview.h"
#include "project.h"
#include "settings.h"
#include "style.h"
#include "settingsdialogue.h"
#include "profilewidget.h"
#include "statisticswidget.h"
#include "mediaplayer.h"
#include "stopdetectdialogue.h"


static const char CONFIG_GROUP[] = "MainWindow";

static const int sbModified = 0;

static const char notUsefulOverlays[] = "elevationprofile,GpsInfo,routing,speedometer";


 
MainWindow::MainWindow(QWidget *pnt)
    : KXmlGuiWindow(pnt)
{
    kDebug();

    setObjectName("MainWindow");
    init();
}



void MainWindow::init()
{
    setAcceptDrops(true);				// accept file drops

    mSplitter = new QSplitter(Qt::Horizontal, this);
    mSplitter->setChildrenCollapsible(false);
    setCentralWidget(mSplitter);

    mUndoStack = new QUndoStack(this);
    connect(mUndoStack, SIGNAL(canUndoChanged(bool)), SLOT(slotCanUndoChanged(bool)));
    connect(mUndoStack, SIGNAL(canRedoChanged(bool)), SLOT(slotCanRedoChanged(bool)));
    connect(mUndoStack, SIGNAL(undoTextChanged(const QString &)), SLOT(slotUndoTextChanged(const QString &)));
    connect(mUndoStack, SIGNAL(redoTextChanged(const QString &)), SLOT(slotRedoTextChanged(const QString &)));
    connect(mUndoStack, SIGNAL(cleanChanged(bool)), SLOT(slotCleanUndoChanged(bool)));

    mProject = new Project;

    mFilesController = new FilesController(this);
    connect(mFilesController, SIGNAL(statusMessage(const QString &)), SLOT(slotStatusMessage(const QString &)));
    connect(mFilesController, SIGNAL(modified()), SLOT(slotSetModified()));
    connect(mFilesController, SIGNAL(updateActionState()), SLOT(slotUpdateActionState()));

    mMapController = new MapController(this);
    connect(mMapController, SIGNAL(statusMessage(const QString &)), SLOT(slotStatusMessage(const QString &)));
    connect(mMapController, SIGNAL(modified()), SLOT(slotSetModified()));
    connect(mMapController, SIGNAL(mapZoomChanged(bool,bool)), SLOT(slotMapZoomChanged(bool,bool)));

    connect(mFilesController, SIGNAL(updateMap()), mMapController->view(), SLOT(update()));
    // TODO: temp, see FilesView::selectionChanged()
    connect(mFilesController, SIGNAL(updateActionState()), mMapController->view(), SLOT(update()));

    connect(mMapController->view(), SIGNAL(createWaypoint(qreal,qreal)),
            mFilesController, SLOT(slotAddWaypoint(qreal,qreal)));

    mSplitter->addWidget(mFilesController->view());
    mSplitter->addWidget(mMapController->view());

    setupStatusBar();
    setupActions();

    readProperties(Settings::self()->config()->group(CONFIG_GROUP));

    mSelectedContainer = NULL;

    slotSetModified(false);
    slotUpdateActionState();
    slotUpdatePasteState();
}



MainWindow::~MainWindow()
{
    delete mProject;

    kDebug() << "done";
}






void MainWindow::setupActions()
{
    KStandardAction::quit(this, SLOT(close()), actionCollection());
    //KStandardAction::close(this, SLOT(close()), actionCollection());

    QAction *a = KStandardAction::openNew(this, SLOT(slotNewProject()), actionCollection());
    a = KStandardAction::open(this, SLOT(slotOpenProject()), actionCollection());
    mSaveProjectAction = KStandardAction::save(this, SLOT(slotSaveProject()), actionCollection());
    mSaveProjectAsAction = KStandardAction::saveAs(this, SLOT(slotSaveAs()), actionCollection());

    mImportAction = actionCollection()->addAction("file_import");
    mImportAction->setText(i18n("Import..."));
    mImportAction->setIcon(QIcon::fromTheme("document-import"));
    mImportAction->setShortcut(Qt::CTRL+Qt::Key_I);
    connect(mImportAction, SIGNAL(triggered()), SLOT(slotImportFile()));

    mExportAction = actionCollection()->addAction("file_export");
    mExportAction->setText(i18n("Export..."));
    mExportAction->setIcon(QIcon::fromTheme("document-export"));
    mExportAction->setShortcut(Qt::CTRL+Qt::Key_E);
    connect(mExportAction, SIGNAL(triggered()), SLOT(slotExportFile()));
    mExportAction->setEnabled(false);

    a = actionCollection()->addAction("file_add_photo");
    a->setText("Import Photo...");
    a->setIcon(QIcon::fromTheme("image-loading"));
    connect(a, SIGNAL(triggered()), SLOT(slotImportPhoto()));

    mSelectAllAction = KStandardAction::selectAll(filesController()->view(), SLOT(slotSelectAllSiblings()), actionCollection());
    mClearSelectAction = KStandardAction::deselect(filesController()->view(), SLOT(clearSelection()), actionCollection());
    mClearSelectAction->setIcon(QIcon::fromTheme("edit-clear-list"));

    mUndoAction = KStandardAction::undo(mUndoStack, SLOT(undo()), actionCollection());
    mUndoAction->setEnabled(false);
    mUndoText = mUndoAction->text();

    mRedoAction = KStandardAction::redo(mUndoStack, SLOT(redo()), actionCollection());
    mRedoAction->setEnabled(false);
    mRedoText = mRedoAction->text();

    mPasteAction = KStandardAction::paste(this, SLOT(slotPaste()), actionCollection());
    mPasteAction->setEnabled(false);
    connect(QApplication::clipboard(), SIGNAL(dataChanged()), SLOT(slotUpdatePasteState()));

    a = actionCollection()->addAction("track_expand_all");
    a->setText(i18n("Expand All"));
    a->setIcon(QIcon::fromTheme("application_side_tree"));
    a->setShortcut(Qt::CTRL+Qt::Key_Period);
    connect(a, SIGNAL(triggered()), filesController()->view(), SLOT(expandAll()));

    a = actionCollection()->addAction("track_collapse_all");
    a->setText(i18n("Collapse All"));
    a->setIcon(QIcon::fromTheme("application_side_list"));
    a->setShortcut(Qt::CTRL+Qt::Key_Comma);
    connect(a, SIGNAL(triggered()), filesController()->view(), SLOT(collapseAll()));

    mAddTrackAction = actionCollection()->addAction("edit_add_track");
    mAddTrackAction->setText(i18n("Add Track"));
    mAddTrackAction->setIcon(QIcon::fromTheme("list-add"));
    connect(mAddTrackAction, SIGNAL(triggered()), filesController(), SLOT(slotAddTrack()));

    mAddFolderAction = actionCollection()->addAction("edit_add_folder");
    mAddFolderAction->setText(i18n("Add Folder"));
    mAddFolderAction->setIcon(QIcon::fromTheme("bookmark-new-list"));
    connect(mAddFolderAction, SIGNAL(triggered()), filesController(), SLOT(slotAddFolder()));

    mAddPointAction = actionCollection()->addAction("edit_add_point");
    mAddPointAction->setText(i18n("Add Point"));
    mAddPointAction->setIcon(QIcon::fromTheme("list-add"));
    connect(mAddPointAction, SIGNAL(triggered()), filesController(), SLOT(slotAddPoint()));

    mAddWaypointAction = actionCollection()->addAction("edit_add_waypoint");
    mAddWaypointAction->setText(i18n("Add Waypoint..."));
    mAddWaypointAction->setIcon(QIcon::fromTheme("list-add"));
    connect(mAddWaypointAction, SIGNAL(triggered()), filesController(), SLOT(slotAddWaypoint()));

    a = actionCollection()->addAction("map_add_waypoint");
    a->setText(i18n("Create Waypoint..."));
    a->setIcon(QIcon::fromTheme("list-add"));
    connect(a, SIGNAL(triggered()), mapController()->view(), SLOT(slotAddWaypoint()));

    mDeleteItemsAction = actionCollection()->addAction("edit_delete_track");
    mDeleteItemsAction->setText(i18n("Delete"));
    mDeleteItemsAction->setIcon(QIcon::fromTheme("edit-delete"));
    connect(mDeleteItemsAction, SIGNAL(triggered()), filesController(), SLOT(slotDeleteItems()));

    mSplitTrackAction = actionCollection()->addAction("track_split");
    mSplitTrackAction->setText(i18n("Split Segment"));
    mSplitTrackAction->setIcon(QIcon::fromTheme("split"));
    connect(mSplitTrackAction, SIGNAL(triggered()), filesController(), SLOT(slotSplitSegment()));

    mMergeTrackAction = actionCollection()->addAction("track_merge");
    mMergeTrackAction->setText(i18n("Merge Segments"));
    mMergeTrackAction->setIcon(QIcon::fromTheme("merge"));
    connect(mMergeTrackAction, SIGNAL(triggered()), filesController(), SLOT(slotMergeSegments()));

    mMoveItemAction = actionCollection()->addAction("track_move_item");
    mMoveItemAction->setText(i18n("Move Item..."));
    mMoveItemAction->setIcon(QIcon::fromTheme("go-up"));
    connect(mMoveItemAction, SIGNAL(triggered()), filesController(), SLOT(slotMoveItem()));

    mStopDetectAction = actionCollection()->addAction("track_stop_detect");
    mStopDetectAction->setText(i18n("Locate Stops..."));
    mStopDetectAction->setIcon(QIcon::fromTheme("media-playback-stop"));
    connect(mStopDetectAction, SIGNAL(triggered()), SLOT(slotTrackStopDetect()));

    mPropertiesAction = actionCollection()->addAction("track_properties");
    // text set in slotUpdateActionState() below
    QList<QKeySequence> cuts;
    cuts.append(QKeySequence(Qt::CTRL+Qt::Key_Return));
    cuts.append(QKeySequence(Qt::CTRL+Qt::Key_Enter));
    mPropertiesAction->setShortcuts(cuts);
    mPropertiesAction->setIcon(QIcon::fromTheme("document-properties"));
    connect(mPropertiesAction, SIGNAL(triggered()), filesController(), SLOT(slotTrackProperties()));

    mWaypointStatusAction = new KSelectAction(i18nc("@action:inmenu", "Waypoint Status"), this);
    mWaypointStatusAction->setToolBarMode(KSelectAction::MenuMode);
    actionCollection()->addAction("waypoint_status", mWaypointStatusAction);

    a = mWaypointStatusAction->addAction(QIcon::fromTheme("task-reject"), i18n("(None)"));
    a->setData(TrackData::StatusNone);
    connect(a, SIGNAL(triggered(bool)), filesController(), SLOT(slotSetWaypointStatus()));

    a = mWaypointStatusAction->addAction(QIcon::fromTheme("task-ongoing"), i18n("To Do"));
    a->setData(TrackData::StatusTodo);
    connect(a, SIGNAL(triggered(bool)), filesController(), SLOT(slotSetWaypointStatus()));

    a = mWaypointStatusAction->addAction(QIcon::fromTheme("task-complete"), i18n("Done"));
    a->setData(TrackData::StatusDone);
    connect(a, SIGNAL(triggered(bool)), filesController(), SLOT(slotSetWaypointStatus()));

    a = mWaypointStatusAction->addAction(QIcon::fromTheme("dialog-warning"), i18n("Uncertain"));
    a->setData(TrackData::StatusQuestion);
    connect(a, SIGNAL(triggered(bool)), filesController(), SLOT(slotSetWaypointStatus()));

    mProfileAction = actionCollection()->addAction("track_profile");
    mProfileAction->setText(i18n("Elevation/Speed Profile..."));
    mProfileAction->setIcon(QIcon::fromTheme("office-chart-line-stacked"));
    connect(mProfileAction, SIGNAL(triggered()), SLOT(slotTrackProfile()));

    mStatisticsAction = actionCollection()->addAction("track_statistics");
    mStatisticsAction->setText(i18n("Statistics/Quality..."));
    mStatisticsAction->setIcon(QIcon::fromTheme("kt-check-data"));
    connect(mStatisticsAction, SIGNAL(triggered()), SLOT(slotTrackStatistics()));

    a = actionCollection()->addAction("track_play_media");
    a->setText(i18nc("@action:inmenu", "View Media"));
    a->setIcon(QIcon::fromTheme("media-playback-start"));
    a->setShortcut(Qt::CTRL+Qt::Key_P);
    connect(a, SIGNAL(triggered()), SLOT(slotPlayMedia()));
    mPlayMediaAction = a;

    a = actionCollection()->addAction("file_open_media");
    a->setText(i18nc("@action:inmenu", "Open Media With..."));
    a->setIcon(QIcon::fromTheme("document-open"));
    connect(a, SIGNAL(triggered()), SLOT(slotOpenMedia()));
    mOpenMediaAction = a;

    a = actionCollection()->addAction("file_save_media");
    a->setText(i18nc("@action:inmenu", "Save Media As..."));
    a->setIcon(QIcon::fromTheme("file-save-as"));
    connect(a, SIGNAL(triggered()), SLOT(slotSaveMedia()));
    mSaveMediaAction = a;

    a = actionCollection()->addAction("map_save");
    a->setText(i18n("Save As Image..."));
    a->setIcon(QIcon::fromTheme("document-save"));
    connect(a, SIGNAL(triggered()), mapController(), SLOT(slotSaveImage()));

    a = actionCollection()->addAction("map_set_home");
    a->setText(i18n("Set Home Position"));
    a->setIconText(i18n("Set Home"));
    a->setIcon(QIcon::fromTheme("bookmarks"));
    connect(a, SIGNAL(triggered()), mapController(), SLOT(slotSetHome()));

    a = actionCollection()->addAction("map_go_home");
    a->setText(i18n("Go to Home Position"));
    a->setIconText(i18n("Go Home"));
    a->setIcon(QIcon::fromTheme("go-home"));
    a->setShortcut(Qt::CTRL+Qt::Key_Home);
    connect(a, SIGNAL(triggered()), mapController(), SLOT(slotGoHome()));

    mMapZoomInAction = actionCollection()->addAction(KStandardAction::ZoomIn, "map_zoom_in");
    connect(mMapZoomInAction, SIGNAL(triggered()), mapController()->view(), SLOT(zoomIn()));

    mMapZoomOutAction = actionCollection()->addAction(KStandardAction::ZoomOut, "map_zoom_out");
    connect(mMapZoomOutAction, SIGNAL(triggered()), mapController()->view(), SLOT(zoomOut()));

    a = actionCollection()->addAction("map_set_zoom");
    a->setText(i18n("Set Standard Zoom"));
    a->setIconText(i18n("Set Zoom"));
    a->setIcon(QIcon::fromTheme("bookmarks"));
    connect(a, SIGNAL(triggered()), mapController(), SLOT(slotSetZoom()));

    a = actionCollection()->addAction("map_zoom_standard");
    a->setText(i18n("Reset to Standard Zoom"));
    a->setIconText(i18n("Reset Zoom"));
    a->setIcon(QIcon::fromTheme("zoom-original"));
    a->setShortcut(Qt::CTRL+Qt::Key_1);
    connect(a, SIGNAL(triggered()), mapController(), SLOT(slotResetZoom()));

    mMapGoToAction = actionCollection()->addAction("map_go_selection");
    mMapGoToAction->setText(i18n("Show on Map"));
    mMapGoToAction->setIcon(QIcon::fromTheme("marble"));
    mMapGoToAction->setShortcut(Qt::CTRL+Qt::Key_G);
    connect(mMapGoToAction, SIGNAL(triggered()), SLOT(slotMapGotoSelection()));

    a = actionCollection()->addAction("map_select_theme");
    a->setText(i18n("Select Theme..."));
    a->setIcon(QIcon::fromTheme("image-loading"));
    connect(a, SIGNAL(triggered()), mapController(), SLOT(slotSelectTheme()));

    a = actionCollection()->addAction("map_find_address");
    a->setText(i18n("Find Address..."));
    a->setIcon(QIcon::fromTheme("view-pim-mail"));
    connect(a, SIGNAL(triggered()), mapController()->view(), SLOT(slotFindAddress()));

    KActionMenu *itemsMenu = new KActionMenu(this);
    QStringList notUseful = QString(notUsefulOverlays).split(',');
    QStringList items = mapController()->view()->overlays(false);
    for (QStringList::const_iterator it = items.constBegin();
         it!=items.constEnd(); ++it)
    {
        QString itemId = (*it);
        if (notUseful.contains(itemId)) continue;

        a = mapController()->view()->actionForOverlay(itemId);
        if (a==NULL) continue;

        connect(a, SIGNAL(triggered()), mapController()->view(), SLOT(slotShowOverlay()));
        itemsMenu->addAction(a);
    }
    a = actionCollection()->addAction("map_show_overlays", itemsMenu);
    a->setText(i18n("Show Overlays"));

    mMapDragAction = new KToggleAction(QIcon::fromTheme("transform-move"), i18n("Move Points"), this);
    mMapDragAction->setShortcut(Qt::CTRL+Qt::Key_M);
    connect(mMapDragAction, SIGNAL(triggered()), SLOT(slotMapMovePoints()));
    actionCollection()->addAction("map_move_points", mMapDragAction);

    a = KStandardAction::preferences(this, SLOT(slotPreferences()), actionCollection());

    a = actionCollection()->addAction("help_about_marble");
    a->setText(i18n("About Marble"));
    a->setIcon(QIcon::fromTheme("marble"));
    connect(a, SIGNAL(triggered()), mapController(), SLOT(slotAboutMarble()));

    setupGUI(KXmlGuiWindow::Default);
    setAutoSaveSettings();
}


void MainWindow::setupStatusBar()
{
    QStatusBar *sb = statusBar();

//    sb->insertPermanentFixedItem(" 1000% ",sbZoom);
//    sb->insertPermanentFixedItem(i18n(" X000,Y000 "),sbLocation);

    mModifiedIndicator = new QLabel(this);
    mModifiedIndicator->setPixmap(QIcon::fromTheme("document-save").pixmap(16));
    mModifiedIndicator->setFixedWidth(20);
    sb->insertPermanentWidget(sbModified, mModifiedIndicator);

    mStatusMessage = new KSqueezedTextLabel(i18n("Initialising..."), sb);
    sb->addWidget(mStatusMessage, 1);

    sb->setSizeGripEnabled(false);
}




void MainWindow::closeEvent(QCloseEvent *ev)
{
    KConfigGroup grp = Settings::self()->config()->group(CONFIG_GROUP);
    saveProperties(grp);
    grp.sync();

    KMainWindow::closeEvent(ev);
}




bool MainWindow::queryClose()
{
    if (!mProject->isModified()) return (true);		// not modified, OK to close

    QString query;
    if (mProject->hasFileName()) query = i18n("<qt>File <b>%1</b> has been modified. Save changes?", mProject->name());
    else query = i18n("File has been modified. Save changes?");

    switch (KMessageBox::warningYesNoCancel(this, query, QString::null,
                                            KStandardGuiItem::save(), KStandardGuiItem::discard()))
    {
case KMessageBox::Yes:
        slotSaveProject();
        return (!mProject->isModified());		// check that save worked

case KMessageBox::No:					// no need to save
        return true;

default:						// cancelled
        return false;
    }

}



// TODO: only when last window closed
// or to unique window/file ID
void MainWindow::saveProperties(KConfigGroup &grp)
{
    kDebug() << "to" << grp.name();
    KMainWindow::saveProperties(grp);

    mapController()->saveProperties();
    filesController()->saveProperties();

    Settings::setMainWindowSplitterState(mSplitter->saveState().toBase64());

    Settings::self()->writeConfig();
}



void MainWindow::readProperties(const KConfigGroup &grp)
{
    kDebug() << "from" << grp.name();
    KMainWindow::readProperties(grp);

    mapController()->readProperties();
    filesController()->readProperties();

    QString splitterState = Settings::mainWindowSplitterState();
    if (!splitterState.isEmpty()) mSplitter->restoreState(QByteArray::fromBase64(splitterState.toAscii()));
}



// Error reporting and status messages are done in FilesController::exportFile()
bool MainWindow::save(const KUrl &to)
{
    kDebug() << "to" << to;

    if (!to.isValid() || !to.hasPath()) return (false);	// should never happen
    QDir d(to.path());					// should be absolute already,
    QString savePath = d.absolutePath();		// but just make sure

    TrackDataFile *tdf = filesController()->model()->rootFileItem();
    if (tdf==NULL) return (false);			// should never happen

    // metadata from map controller
    tdf->setMetadata(DataIndexer::self()->index("position"), mapController()->view()->currentPosition());

    // metadata for save file
    tdf->setMetadata(DataIndexer::self()->index("creator"), QApplication::applicationDisplayName());
    tdf->setMetadata(DataIndexer::self()->index("time"), QDateTime::currentDateTimeUtc().toString(Qt::ISODate));

    if (filesController()->exportFile(savePath, tdf)!=FilesController::StatusOk) return (false);
    slotStatusMessage(i18n("<qt>Saved <filename>%1</filename>", savePath));
    return (true);					// more appropriate message
}



// Error reporting and status messages are done in FilesController::importFile()
FilesController::Status MainWindow::load(const KUrl &from)
{
    kDebug() << "from" << from;

    // TODO: allow non-local files
    if (!from.isValid() || !from.hasPath()) return (FilesController::StatusFailed);
    QDir d(from.path());				// should be absolute already,
    QString loadPath = d.absolutePath();		// but just make sure


    FilesController::Status status = filesController()->importFile(loadPath);
    if (status!=FilesController::StatusOk && status!=FilesController::StatusResave) return (status);

    TrackDataFile *tdf = filesController()->model()->rootFileItem();
    if (tdf!=NULL)
    {
        QString s = tdf->metadata("position");
        kDebug() << "pos metadata" << s;
        mapController()->view()->blockSignals(true);	// no status bar update from zooming
        if (!s.isEmpty())
        {
            mapController()->view()->setCurrentPosition(s);
        }
        else
        {
            mapController()->gotoSelection(QList<TrackDataItem *>() << tdf);
        }
        mapController()->view()->blockSignals(false);
    }

    filesController()->view()->expandToDepth(1);	// expand to show segments
    return (status);
}



void MainWindow::slotStatusMessage(const QString &text)
{
    mStatusMessage->setText(text);
}



void MainWindow::slotNewProject()
{
    MainWindow *w = new MainWindow(NULL);
    w->show();
}



void MainWindow::slotOpenProject()
{
    KFileDialog d(QUrl("kfiledialog:///project/"), FilesController::allProjectFilters(true), this);
    d.setWindowTitle(i18n("Open Tracks File"));
    d.setOperationMode(KFileDialog::Opening);
    d.setKeepLocation(true);
    d.setMode(KFile::File|KFile::LocalOnly);

    if (!d.exec()) return;

    if (filesController()->model()->isEmpty()) loadProject(d.selectedUrl());
    else
    {
        MainWindow *w = new MainWindow(NULL);
        const bool ok = w->loadProject(d.selectedUrl());
        if (ok) w->show();
        else w->deleteLater();
    }
}


bool MainWindow::loadProject(const KUrl &loadFrom)
{
    if (!loadFrom.isValid()) return (false);
    kDebug() << "from" << loadFrom;

    FilesController::Status status = load(loadFrom);	// load in data file
    if (status!=FilesController::StatusOk && status!=FilesController::StatusResave) return (false);

    mProject->setFileName(loadFrom);
    mUndoStack->clear();				// clear undo history
							// ensure window title updated
    slotSetModified(status==FilesController::StatusResave);
    return (true);
}



void MainWindow::slotSaveProject()
{
    if (!mProject->hasFileName())
    {
        slotSaveAs();
        return;
    }

    KUrl projectFile = mProject->fileName();
    kDebug() << "to" << projectFile;

    if (save(projectFile))
    {
        mUndoStack->setClean();				// undo history is now clean
        slotSetModified(false);				// ensure window title updated
    }
}



void MainWindow::slotSaveAs()
{
    KFileDialog d(QUrl("kfiledialog:///project/untitled"), FilesController::allProjectFilters(false), this);
    d.setWindowTitle(i18n("Save Tracks File As"));
    d.setOperationMode(KFileDialog::Saving);
    d.setKeepLocation(true);
    d.setMode(KFile::File|KFile::LocalOnly);
    d.setConfirmOverwrite(true);

    if (!d.exec()) return;
    KUrl saveTo = d.selectedUrl();
    if (!saveTo.isValid()) return;			// should never happen,
							// but check to avoid recursion
    mProject->setFileName(saveTo);
    slotSaveProject();
}


void MainWindow::slotImportFile()
{
    KFileDialog d(QUrl("kfiledialog:///import"), FilesController::allImportFilters(), this);
    d.setWindowTitle(i18n("Import File"));
    d.setOperationMode(KFileDialog::Opening);
    d.setKeepLocation(true);
    d.setMode(KFile::File|KFile::LocalOnly);

    if (!d.exec()) return;
    filesController()->importFile(d.selectedUrl());
}


void MainWindow::slotExportFile()
{
    KFileDialog d(QUrl("kfiledialog:///export/"+mProject->name(true)), FilesController::allExportFilters(), this);
    d.setWindowTitle(i18n("Export File"));
    d.setOperationMode(KFileDialog::Saving);
    d.setConfirmOverwrite(true);
    d.setKeepLocation(true);
    d.setMode(KFile::File|KFile::LocalOnly);

    if (!d.exec()) return;
//////// TODO: export selected item
//    filesController()->exportFile(d.selectedUrl());
}


void MainWindow::slotImportPhoto()
{
    const QStringList mimeTypes = KImageIO::mimeTypes(KImageIO::Reading);
    KFileDialog d(KUrl("kfiledialog:///importphoto"), mimeTypes.join(" "), this);
    d.setWindowTitle(i18n("Import Photo"));
    d.setOperationMode(KFileDialog::Opening);
    d.setKeepLocation(true);
    d.setMode(KFile::Files|KFile::LocalOnly);
    d.setInlinePreviewShown(true);

    if (!d.exec()) return;
    filesController()->importPhoto(d.selectedUrls());
}


void MainWindow::slotSetModified(bool mod)
{
    mProject->setModified(mod);
    mSaveProjectAction->setEnabled(mod && mProject->hasFileName());
    mModifiedIndicator->setEnabled(mod);
    setWindowTitle(mProject->name()+" [*]");
    setWindowModified(mod);

    mSaveProjectAsAction->setEnabled(!filesController()->model()->isEmpty());
}


void MainWindow::slotUpdateActionState()
{
    int selCount = filesController()->view()->selectedCount();
    TrackData::Type selType = filesController()->view()->selectedType();
    kDebug() << "selected" << selCount << "type" << selType;

    bool propsEnabled = false;
    bool profileEnabled = false;
    QString propsText = i18nc("@action:inmenu", "Properties...");
    bool delEnabled = true;
    QString delText = i18nc("@action:inmenu", "Delete");
    bool moveEnabled = false;
    QString moveText = i18nc("@action:inmenu", "Move Item...");
    bool playEnabled = false;
    QString playText = i18nc("@action:inmenu", "View Media");
    bool statusEnabled = false;
    int statusValue = TrackData::StatusInvalid;

    const TrackDataItem *selectedContainer = NULL;
    switch (selType)
    {
case TrackData::File:
        propsText = i18ncp("@action:inmenu", "File Properties...", "Files Properties...", selCount);
        propsEnabled = true;
        delEnabled = false;
        profileEnabled = true;
        break;

case TrackData::Track:
        propsText = i18ncp("@action:inmenu", "Track Properties...", "Tracks Properties...", selCount);
        propsEnabled = true;
        delText = i18ncp("@action:inmenu", "Delete Track", "Delete Tracks", selCount);
        profileEnabled = true;
        break;

case TrackData::Segment:
        propsText = i18ncp("@action:inmenu", "Segment Properties...", "Segments Properties...", selCount);
        propsEnabled = true;
        delText = i18ncp("@action:inmenu", "Delete Segment", "Delete Segments", selCount);
        moveEnabled = true;
        moveText = i18nc("@action:inmenu", "Move Segment...");
        selectedContainer = filesController()->view()->selectedItem();
        profileEnabled = true;
        break;

case TrackData::Point:
        propsText = i18ncp("@action:inmenu", "Point Properties...", "Points Properties...", selCount);
        propsEnabled = true;
        delText = i18ncp("@action:inmenu", "Delete Point", "Delete Points", selCount);
        selectedContainer = filesController()->view()->selectedItem()->parent();
        profileEnabled = (selCount>1);
        break;

case TrackData::Folder:
        propsText = i18ncp("@action:inmenu", "Folder Properties...", "Folders Properties...", selCount);
        propsEnabled = true;
        delText = i18ncp("@action:inmenu", "Delete Folder", "Delete Folders", selCount);
        moveEnabled = true;
        moveText = i18nc("@action:inmenu", "Move Folder...");
        selectedContainer = filesController()->view()->selectedItem();
        break;

case TrackData::Waypoint:
        propsText = i18ncp("@action:inmenu", "Waypoint Properties...", "Waypoints Properties...", selCount);
        propsEnabled = true;
        delText = i18ncp("@action:inmenu", "Delete Waypoint", "Delete Waypoints", selCount);
        moveEnabled = true;
        moveText = i18ncp("@action:inmenu", "Move Waypoint...", "Move Waypoints...", selCount);
        selectedContainer = filesController()->view()->selectedItem()->parent();
        statusEnabled = true;

        if (selCount==1)
        {
            const TrackDataWaypoint *tdw = dynamic_cast<const TrackDataWaypoint *>(filesController()->view()->selectedItem());
            if (tdw!=NULL)
            {
                switch (tdw->waypointType())
                {
case TrackData::WaypointAudioNote:	playEnabled = true;
					playText = i18nc("@action:inmenu", "Play Audio Note");
					break;

case TrackData::WaypointVideoNote:	playEnabled = true;
					playText = i18nc("@action:inmenu", "Play Video Note");
					break;

case TrackData::WaypointPhoto:		playEnabled = true;
					playText = i18nc("@action:inmenu", "View Photo");
					break;

default:				break;
                }

                statusValue = tdw->metadata("status").toInt();
            }
        }
        break;

case TrackData::Mixed:
        propsText = i18nc("@action:inmenu", "Selection Properties...");
        delText = i18nc("@action:inmenu", "Delete Selection");
        delEnabled = false;
        break;

default:
        delEnabled = false;
        break;
    }

    mPropertiesAction->setEnabled(propsEnabled);
    mPropertiesAction->setText(propsText);
    mDeleteItemsAction->setEnabled(delEnabled);
    mDeleteItemsAction->setText(delText);
    mProfileAction->setEnabled(profileEnabled);
    mStatisticsAction->setEnabled(profileEnabled);
    mStopDetectAction->setEnabled(profileEnabled);

    mPlayMediaAction->setEnabled(playEnabled);
    mPlayMediaAction->setText(playText);
    mOpenMediaAction->setEnabled(playEnabled);
    mSaveMediaAction->setEnabled(playEnabled);

    mSelectAllAction->setEnabled(selCount>0 && selType!=TrackData::Mixed);
    mClearSelectAction->setEnabled(selCount>0);
    mMapGoToAction->setEnabled(selCount>0 && selType!=TrackData::Mixed);

    mSplitTrackAction->setEnabled(selCount==1 && selType==TrackData::Point);
    mMoveItemAction->setEnabled(moveEnabled);
    mMoveItemAction->setText(moveText);
    mMergeTrackAction->setEnabled(selCount>1 && selType==TrackData::Segment);
    mAddTrackAction->setEnabled(selCount==1 && selType==TrackData::File);
    mAddFolderAction->setEnabled(selCount==1 && (selType==TrackData::File ||
                                                 selType==TrackData::Folder));
    mAddWaypointAction->setEnabled(selCount==1 && (selType==TrackData::Folder ||
                                                   selType==TrackData::Point ||
                                                   selType==TrackData::Waypoint));

    mWaypointStatusAction->setEnabled(statusEnabled);
    QList<QAction *> acts = mWaypointStatusAction->actions();
    for (QList<QAction *>::const_iterator it = acts.constBegin(); it!=acts.constEnd(); ++it)
    {
        QAction *act = (*it);
        act->setChecked(statusValue==act->data().toInt());
    }

    if (selCount==1 && selType==TrackData::Point)
    {
        const QModelIndex idx = filesController()->model()->indexForItem(filesController()->view()->selectedItem());
        mAddPointAction->setEnabled(idx.row()>0);	// not first point in segment
    }
    else mAddPointAction->setEnabled(false);

    // If there is a selected container or point(s), then move points mode
    // is allowed to be entered;  otherwise, it is disabled.
    //
    // If there is a selected container and it is the same as the currently
    // selected container, then move points mode can stay at the same state
    // as it currently is.  Otherwise, it is forced off.

    if (selectedContainer!=NULL)
    {
        if (selectedContainer!=mSelectedContainer)
        {
            mMapDragAction->setChecked(false);
            slotMapMovePoints();
        }
        mMapDragAction->setEnabled(true);
    }
    else
    {
        mMapDragAction->setChecked(false);
        slotMapMovePoints();
        mMapDragAction->setEnabled(false);
    }

    mSelectedContainer = selectedContainer;
}


void MainWindow::slotCanUndoChanged(bool can)
{
    kDebug() << can;
    mUndoAction->setEnabled(can);
}

void MainWindow::slotCanRedoChanged(bool can)
{
    kDebug() << can;
    mRedoAction->setEnabled(can);
}

void MainWindow::slotUndoTextChanged(const QString &text)
{
    kDebug() << text;
    mUndoAction->setText(text.isEmpty() ? mUndoText : i18n("%2: %1", text, mUndoText));
}

void MainWindow::slotRedoTextChanged(const QString &text)
{
    kDebug() << text;
    mRedoAction->setText(text.isEmpty() ? mRedoText : i18n("%2: %1", text, mRedoText));
}

void MainWindow::slotCleanUndoChanged(bool clean)
{
    kDebug() << "clean" << clean;
    slotSetModified(!clean);
}


void MainWindow::slotMapZoomChanged(bool canZoomIn, bool canZoomOut)
{
    mMapZoomInAction->setEnabled(canZoomIn);
    mMapZoomOutAction->setEnabled(canZoomOut);
}


void MainWindow::slotMapGotoSelection()
{
    mapController()->gotoSelection(filesController()->view()->selectedItems());
}


void MainWindow::slotPreferences()
{
    SettingsDialogue d(this);
    if (d.exec()) mapController()->view()->update();
}


void MainWindow::slotMapMovePoints()
{
    mapController()->view()->setMovePointsMode(mMapDragAction->isChecked());
}


void MainWindow::slotTrackProfile()
{
    ProfileWidget *w = new ProfileWidget(this);
    w->setAttribute(Qt::WA_DeleteOnClose);
    w->setModal(false);
    w->setWindowTitle(i18n("Track Profile"));
    w->show();
}


void MainWindow::slotTrackStatistics()
{
    StatisticsWidget *w = new StatisticsWidget(this);
    w->setAttribute(Qt::WA_DeleteOnClose);
    w->setModal(false);
    w->setWindowTitle(i18n("Track Statistics/Quality"));
    w->show();
}


// TODO: status messages from player
void MainWindow::slotPlayMedia()
{
    const TrackDataWaypoint *tdw = dynamic_cast<const TrackDataWaypoint *>(filesController()->view()->selectedItem());
    Q_ASSERT(tdw!=NULL);
    switch (tdw->waypointType())
    {
case TrackData::WaypointAudioNote:	MediaPlayer::playAudioNote(tdw);
					break;

case TrackData::WaypointVideoNote:	MediaPlayer::playVideoNote(tdw);
					break;

case TrackData::WaypointPhoto:		MediaPlayer::viewPhotoNote(tdw);
					break;

default:				break;
    }
}


void MainWindow::slotOpenMedia()
{
    const TrackDataWaypoint *tdw = dynamic_cast<const TrackDataWaypoint *>(filesController()->view()->selectedItem());
    Q_ASSERT(tdw!=NULL);
    if (tdw->isMediaType()) MediaPlayer::openMediaFile(tdw);
}


void MainWindow::slotSaveMedia()
{
    const TrackDataWaypoint *tdw = dynamic_cast<const TrackDataWaypoint *>(filesController()->view()->selectedItem());
    Q_ASSERT(tdw!=NULL);
    if (tdw->isMediaType()) MediaPlayer::saveMediaFile(tdw);
}


void MainWindow::executeCommand(QUndoCommand *cmd)
{
    if (mUndoStack!=NULL) mUndoStack->push(cmd);	// do via undo system
    else { cmd->redo(); delete cmd; }			// do directly (fallback)
}


void MainWindow::dragEnterEvent(QDragEnterEvent *ev)
{
    if (ev->dropAction()!=Qt::CopyAction) return;
    const QMimeData *mimeData = ev->mimeData();
    if (!mimeData->hasUrls()) return;

    ev->accept();
}


bool MainWindow::acceptMimeData(const QMimeData *mimeData)
{
    if (!mimeData->hasUrls()) return (false);
    QList<QUrl> urls = mimeData->urls();

    const QStringList imageTypes = KImageIO::mimeTypes(KImageIO::Reading);

    KUrl::List validUrls;
    for (QList<QUrl>::const_iterator it = urls.constBegin(); it!=urls.constEnd(); ++it)
    {
        const KUrl url = (*it);
        const KMimeType::Ptr mime = KMimeType::findByUrl(url);

        if (imageTypes.contains(mime->name()))
        {
            kDebug() << "accept image" << url << "mimetype" << mime->name();
            validUrls.append(url);
        }
        else kWarning() << "reject" << url << "mimetype" << mime->name();
    }

    if (validUrls.isEmpty())				// no usable URLs found
    {
        KMessageBox::sorry(this, i18n("Don't know what to do with any of the pasted or dropped URLs"));
        return (false);
    }

    return (filesController()->importPhoto(validUrls)!=FilesController::StatusCancelled);
}


void MainWindow::dropEvent(QDropEvent *ev)
{
    if (ev->dropAction()!=Qt::CopyAction) return;
    const QMimeData *mimeData = ev->mimeData();
    if (acceptMimeData(mimeData)) ev->accept();
}


void MainWindow::slotPaste()
{
    const QClipboard *clip = QApplication::clipboard();
    if (clip->ownsClipboard()) return;			// has our internal data

    const QMimeData *mimeData = clip->mimeData();
    acceptMimeData(mimeData);
}


void MainWindow::slotUpdatePasteState()
{
    const QClipboard *clip = QApplication::clipboard();
    bool enable = false;
    if (!clip->ownsClipboard())				// has data from someone else
    {
        const QMimeData *mimeData = clip->mimeData();
        if (mimeData->hasUrls()) enable = true;		// one or more URLs
    }

    mPasteAction->setEnabled(enable);
}


void MainWindow::slotTrackStopDetect()
{
    StopDetectDialogue *d = new StopDetectDialogue(this);
    d->show();
}
