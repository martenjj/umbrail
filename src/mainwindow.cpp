
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
#include <qdebug.h>
#include <qurl.h>
#include <qfiledialog.h>
#include <qimagereader.h>
#include <qmimetype.h>
#include <qmimedatabase.h>

#include <klocalizedstring.h>
#include <kaction.h>
#include <ktoggleaction.h>
#include <kselectaction.h>
#include <kactioncollection.h>
#include <kstandardaction.h>
#include <kmessagebox.h>
#include <ksqueezedtextlabel.h>
#include <kactionmenu.h>

#include <recentsaver.h>
#include <imagefilter.h>

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
    qDebug();

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

    qDebug() << "done";
}






void MainWindow::setupActions()
{
    KActionCollection *ac = actionCollection();

    KStandardAction::quit(this, SLOT(close()), ac);

    QAction *a = KStandardAction::openNew(this, SLOT(slotNewProject()), ac);
    a = KStandardAction::open(this, SLOT(slotOpenProject()), ac);
    mSaveProjectAction = KStandardAction::save(this, SLOT(slotSaveProject()), ac);
    mSaveProjectAsAction = KStandardAction::saveAs(this, SLOT(slotSaveAs()), ac);

    mImportAction = ac->addAction("file_import");
    mImportAction->setText(i18n("Import..."));
    mImportAction->setIcon(QIcon::fromTheme("document-import"));
    ac->setDefaultShortcut(mImportAction, Qt::CTRL+Qt::Key_I);
    connect(mImportAction, SIGNAL(triggered()), SLOT(slotImportFile()));

    mExportAction = ac->addAction("file_export");
    mExportAction->setText(i18n("Export..."));
    mExportAction->setIcon(QIcon::fromTheme("document-export"));
    ac->setDefaultShortcut(mExportAction, Qt::CTRL+Qt::Key_E);
    connect(mExportAction, SIGNAL(triggered()), SLOT(slotExportFile()));
    mExportAction->setEnabled(false);

    a = ac->addAction("file_add_photo");
    a->setText("Import Photo...");
    a->setIcon(QIcon::fromTheme("image-loading"));
    connect(a, SIGNAL(triggered()), SLOT(slotImportPhoto()));

    mSelectAllAction = KStandardAction::selectAll(filesController()->view(), SLOT(slotSelectAllSiblings()), ac);
    mClearSelectAction = KStandardAction::deselect(filesController()->view(), SLOT(clearSelection()), ac);
    mClearSelectAction->setIcon(QIcon::fromTheme("edit-clear-list"));

    mUndoAction = KStandardAction::undo(mUndoStack, SLOT(undo()), ac);
    mUndoAction->setEnabled(false);
    mUndoText = mUndoAction->text();

    mRedoAction = KStandardAction::redo(mUndoStack, SLOT(redo()), ac);
    mRedoAction->setEnabled(false);
    mRedoText = mRedoAction->text();

    mPasteAction = KStandardAction::paste(this, SLOT(slotPaste()), ac);
    mPasteAction->setEnabled(false);
    connect(QApplication::clipboard(), SIGNAL(dataChanged()), SLOT(slotUpdatePasteState()));

    a = ac->addAction("track_expand_all");
    a->setText(i18n("Expand All"));
    a->setIcon(QIcon::fromTheme("application_side_tree"));
    ac->setDefaultShortcut(a, Qt::CTRL+Qt::Key_Period);
    connect(a, SIGNAL(triggered()), filesController()->view(), SLOT(expandAll()));

    a = ac->addAction("track_collapse_all");
    a->setText(i18n("Collapse All"));
    a->setIcon(QIcon::fromTheme("application_side_list"));
    ac->setDefaultShortcut(a, Qt::CTRL+Qt::Key_Comma);
    connect(a, SIGNAL(triggered()), filesController()->view(), SLOT(collapseAll()));

    mAddTrackAction = ac->addAction("edit_add_track");
    mAddTrackAction->setText(i18n("Add Track"));
    mAddTrackAction->setIcon(QIcon::fromTheme("list-add"));
    connect(mAddTrackAction, SIGNAL(triggered()), filesController(), SLOT(slotAddTrack()));

    mAddFolderAction = ac->addAction("edit_add_folder");
    mAddFolderAction->setText(i18n("Add Folder"));
    mAddFolderAction->setIcon(QIcon::fromTheme("bookmark-new-list"));
    connect(mAddFolderAction, SIGNAL(triggered()), filesController(), SLOT(slotAddFolder()));

    mAddPointAction = ac->addAction("edit_add_point");
    mAddPointAction->setText(i18n("Add Point"));
    mAddPointAction->setIcon(QIcon::fromTheme("list-add"));
    connect(mAddPointAction, SIGNAL(triggered()), filesController(), SLOT(slotAddPoint()));

    mAddWaypointAction = ac->addAction("edit_add_waypoint");
    mAddWaypointAction->setText(i18n("Add Waypoint..."));
    mAddWaypointAction->setIcon(QIcon::fromTheme("list-add"));
    connect(mAddWaypointAction, SIGNAL(triggered()), filesController(), SLOT(slotAddWaypoint()));

    a = ac->addAction("map_add_waypoint");
    a->setText(i18n("Create Waypoint..."));
    a->setIcon(QIcon::fromTheme("list-add"));
    connect(a, SIGNAL(triggered()), mapController()->view(), SLOT(slotAddWaypoint()));

    mDeleteItemsAction = ac->addAction("edit_delete_track");
    mDeleteItemsAction->setText(i18n("Delete"));
    mDeleteItemsAction->setIcon(QIcon::fromTheme("edit-delete"));
    connect(mDeleteItemsAction, SIGNAL(triggered()), filesController(), SLOT(slotDeleteItems()));

    mSplitTrackAction = ac->addAction("track_split");
    mSplitTrackAction->setText(i18n("Split Segment"));
    mSplitTrackAction->setIcon(QIcon::fromTheme("split"));
    connect(mSplitTrackAction, SIGNAL(triggered()), filesController(), SLOT(slotSplitSegment()));

    mMergeTrackAction = ac->addAction("track_merge");
    mMergeTrackAction->setText(i18n("Merge Segments"));
    mMergeTrackAction->setIcon(QIcon::fromTheme("merge"));
    connect(mMergeTrackAction, SIGNAL(triggered()), filesController(), SLOT(slotMergeSegments()));

    mMoveItemAction = ac->addAction("track_move_item");
    mMoveItemAction->setText(i18n("Move Item..."));
    mMoveItemAction->setIcon(QIcon::fromTheme("go-up"));
    connect(mMoveItemAction, SIGNAL(triggered()), filesController(), SLOT(slotMoveItem()));

    mStopDetectAction = ac->addAction("track_stop_detect");
    mStopDetectAction->setText(i18n("Locate Stops..."));
    mStopDetectAction->setIcon(QIcon::fromTheme("media-playback-stop"));
    connect(mStopDetectAction, SIGNAL(triggered()), SLOT(slotTrackStopDetect()));

    mPropertiesAction = ac->addAction("track_properties");
    // text set in slotUpdateActionState() below
    QList<QKeySequence> cuts;
    cuts.append(QKeySequence(Qt::CTRL+Qt::Key_Return));
    cuts.append(QKeySequence(Qt::CTRL+Qt::Key_Enter));
    ac->setDefaultShortcuts(mPropertiesAction, cuts);
    mPropertiesAction->setIcon(QIcon::fromTheme("document-properties"));
    connect(mPropertiesAction, SIGNAL(triggered()), filesController(), SLOT(slotTrackProperties()));

    mWaypointStatusAction = new KSelectAction(i18nc("@action:inmenu", "Waypoint Status"), this);
    mWaypointStatusAction->setToolBarMode(KSelectAction::MenuMode);
    ac->addAction("waypoint_status", mWaypointStatusAction);

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

    mProfileAction = ac->addAction("track_profile");
    mProfileAction->setText(i18n("Elevation/Speed Profile..."));
    mProfileAction->setIcon(QIcon::fromTheme("office-chart-line-stacked"));
    connect(mProfileAction, SIGNAL(triggered()), SLOT(slotTrackProfile()));

    mStatisticsAction = ac->addAction("track_statistics");
    mStatisticsAction->setText(i18n("Statistics/Quality..."));
    mStatisticsAction->setIcon(QIcon::fromTheme("kt-check-data"));
    connect(mStatisticsAction, SIGNAL(triggered()), SLOT(slotTrackStatistics()));

    a = ac->addAction("track_play_media");
    a->setText(i18nc("@action:inmenu", "View Media"));
    a->setIcon(QIcon::fromTheme("media-playback-start"));
    ac->setDefaultShortcut(a, Qt::CTRL+Qt::Key_P);
    connect(a, SIGNAL(triggered()), SLOT(slotPlayMedia()));
    mPlayMediaAction = a;

    a = ac->addAction("file_open_media");
    a->setText(i18nc("@action:inmenu", "Open Media With..."));
    a->setIcon(QIcon::fromTheme("document-open"));
    connect(a, SIGNAL(triggered()), SLOT(slotOpenMedia()));
    mOpenMediaAction = a;

    a = ac->addAction("file_save_media");
    a->setText(i18nc("@action:inmenu", "Save Media As..."));
    a->setIcon(QIcon::fromTheme("file-save-as"));
    connect(a, SIGNAL(triggered()), SLOT(slotSaveMedia()));
    mSaveMediaAction = a;

    a = ac->addAction("map_save");
    a->setText(i18n("Save As Image..."));
    a->setIcon(QIcon::fromTheme("document-save"));
    connect(a, SIGNAL(triggered()), mapController(), SLOT(slotSaveImage()));

    a = ac->addAction("map_set_home");
    a->setText(i18n("Set Home Position"));
    a->setIconText(i18n("Set Home"));
    a->setIcon(QIcon::fromTheme("bookmarks"));
    connect(a, SIGNAL(triggered()), mapController(), SLOT(slotSetHome()));

    a = ac->addAction("map_go_home");
    a->setText(i18n("Go to Home Position"));
    a->setIconText(i18n("Go Home"));
    a->setIcon(QIcon::fromTheme("go-home"));
    ac->setDefaultShortcut(a, Qt::CTRL+Qt::Key_Home);
    connect(a, SIGNAL(triggered()), mapController(), SLOT(slotGoHome()));

    mMapZoomInAction = ac->addAction(KStandardAction::ZoomIn, "map_zoom_in");
    connect(mMapZoomInAction, SIGNAL(triggered()), mapController()->view(), SLOT(zoomIn()));

    mMapZoomOutAction = ac->addAction(KStandardAction::ZoomOut, "map_zoom_out");
    connect(mMapZoomOutAction, SIGNAL(triggered()), mapController()->view(), SLOT(zoomOut()));

    a = ac->addAction("map_set_zoom");
    a->setText(i18n("Set Standard Zoom"));
    a->setIconText(i18n("Set Zoom"));
    a->setIcon(QIcon::fromTheme("bookmarks"));
    connect(a, SIGNAL(triggered()), mapController(), SLOT(slotSetZoom()));

    a = ac->addAction("map_zoom_standard");
    a->setText(i18n("Reset to Standard Zoom"));
    a->setIconText(i18n("Reset Zoom"));
    a->setIcon(QIcon::fromTheme("zoom-original"));
    ac->setDefaultShortcut(a, Qt::CTRL+Qt::Key_1);
    connect(a, SIGNAL(triggered()), mapController(), SLOT(slotResetZoom()));

    mMapGoToAction = ac->addAction("map_go_selection");
    mMapGoToAction->setText(i18n("Show on Map"));
    mMapGoToAction->setIcon(QIcon::fromTheme("marble"));
    ac->setDefaultShortcut(mMapGoToAction, Qt::CTRL+Qt::Key_G);
    connect(mMapGoToAction, SIGNAL(triggered()), SLOT(slotMapGotoSelection()));

    a = ac->addAction("map_select_theme");
    a->setText(i18n("Select Theme..."));
    a->setIcon(QIcon::fromTheme("image-loading"));
    connect(a, SIGNAL(triggered()), mapController(), SLOT(slotSelectTheme()));

    a = ac->addAction("map_find_address");
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
    a = ac->addAction("map_show_overlays", itemsMenu);
    a->setText(i18n("Show Overlays"));

    mMapDragAction = new KToggleAction(QIcon::fromTheme("transform-move"), i18n("Move Points"), this);
    ac->setDefaultShortcut(mMapDragAction, Qt::CTRL+Qt::Key_M);
    connect(mMapDragAction, SIGNAL(triggered()), SLOT(slotMapMovePoints()));
    ac->addAction("map_move_points", mMapDragAction);

    a = KStandardAction::preferences(this, SLOT(slotPreferences()), ac);

    a = ac->addAction("help_about_marble");
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
    qDebug() << "to" << grp.name();
    KMainWindow::saveProperties(grp);

    mapController()->saveProperties();
    filesController()->saveProperties();

    Settings::setMainWindowSplitterState(mSplitter->saveState().toBase64());

    Settings::self()->save();
}



void MainWindow::readProperties(const KConfigGroup &grp)
{
    qDebug() << "from" << grp.name();
    KMainWindow::readProperties(grp);

    mapController()->readProperties();
    filesController()->readProperties();

    QString splitterState = Settings::mainWindowSplitterState();
    if (!splitterState.isEmpty()) mSplitter->restoreState(QByteArray::fromBase64(splitterState.toAscii()));
}



// Error reporting and status messages are done in FilesController::exportFile()
bool MainWindow::save(const QUrl &to)
{
    qDebug() << "to" << to;

    if (!to.isValid()) return (false);			// should never happen

    TrackDataFile *tdf = filesController()->model()->rootFileItem();
    if (tdf==NULL) return (false);			// should never happen

    // metadata from map controller
    tdf->setMetadata(DataIndexer::self()->index("position"), mapController()->view()->currentPosition());

    // metadata for save file
    tdf->setMetadata(DataIndexer::self()->index("creator"), QApplication::applicationDisplayName());
    tdf->setMetadata(DataIndexer::self()->index("time"), QDateTime::currentDateTimeUtc().toString(Qt::ISODate));

    if (filesController()->exportFile(to, tdf)!=FilesController::StatusOk) return (false);
    slotStatusMessage(i18n("<qt>Saved <filename>%1</filename>", to.toDisplayString()));
    return (true);					// more appropriate message
}



// Error reporting and status messages are done in FilesController::importFile()
FilesController::Status MainWindow::load(const QUrl &from)
{
    qDebug() << "from" << from;

    // TODO: allow non-local files
    if (!from.isValid()) return (FilesController::StatusFailed);

    FilesController::Status status = filesController()->importFile(from);
    if (status!=FilesController::StatusOk && status!=FilesController::StatusResave) return (status);

    TrackDataFile *tdf = filesController()->model()->rootFileItem();
    if (tdf!=NULL)
    {
        QString s = tdf->metadata("position");
        qDebug() << "pos metadata" << s;
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
    RecentSaver saver("project");
    QUrl file = QFileDialog::getOpenFileUrl(this,					// parent
                                            i18n("Open Tracks File"),			// caption
                                            saver.recentUrl(),				// dir
                                            FilesController::allProjectFilters(true),	// filter
                                            NULL,					// selectedFilter,
                                            QFileDialog::Options(),			// options
                                            QStringList("file"));			// supportedSchemes

    if (!file.isValid()) return;			// didn't get a file name
    saver.save(file);

    if (filesController()->model()->isEmpty()) loadProject(file);
    else
    {
        MainWindow *w = new MainWindow(NULL);
        const bool ok = w->loadProject(file);
        if (ok) w->show();
        else w->deleteLater();
    }
}


bool MainWindow::loadProject(const QUrl &loadFrom)
{
    if (!loadFrom.isValid()) return (false);
    qDebug() << "from" << loadFrom;

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

    QUrl projectFile = mProject->fileName();
    qDebug() << "to" << projectFile;

    if (save(projectFile))
    {
        mUndoStack->setClean();				// undo history is now clean
        slotSetModified(false);				// ensure window title updated
    }
}



void MainWindow::slotSaveAs()
{
    RecentSaver saver("project");
    QUrl file = QFileDialog::getSaveFileUrl(this,					// parent
                                            i18n("Save Tracks File As"),		// caption
                                            saver.recentUrl("untitled"),		// dir
                                            FilesController::allProjectFilters(false),	// filter
                                            NULL,					// selectedFilter,
                                            QFileDialog::Options(),			// options
                                            QStringList("file"));			// supportedSchemes

    if (!file.isValid()) return;			// didn't get a file name
    saver.save(file);

    mProject->setFileName(file);
    slotSaveProject();
}


void MainWindow::slotImportFile()
{
    RecentSaver saver("import");
    QUrl file = QFileDialog::getOpenFileUrl(this,					// parent
                                            i18n("Import File"),			// caption
                                            saver.recentUrl(),				// dir
                                            FilesController::allImportFilters(),	// filter
                                            NULL,					// selectedFilter,
                                            QFileDialog::Options(),			// options
                                            QStringList("file"));			// supportedSchemes

    if (!file.isValid()) return;			// didn't get a file name
    saver.save(file);
    filesController()->importFile(file);
}


void MainWindow::slotExportFile()
{
    RecentSaver saver("project");
    QUrl file = QFileDialog::getSaveFileUrl(this,					// parent
                                            i18n("Export File As"),			// caption
                                            saver.recentUrl(mProject->name(true)),	// dir
                                            FilesController::allExportFilters(),	// filter
                                            NULL,					// selectedFilter,
                                            QFileDialog::Options(),			// options
                                            QStringList("file"));			// supportedSchemes

    if (!file.isValid()) return;			// didn't get a file name
    saver.save(file);
//////// TODO: export selected item
//    filesController()->exportFile(file);
}


void MainWindow::slotImportPhoto()
{
    RecentSaver saver("importphoto");
    QList<QUrl> files = QFileDialog::getOpenFileUrls(this,				// parent
                                                     i18n("Import Photo"),		// caption
                                                     saver.recentUrl(),			// dir
                                                     ImageFilter::qtFilterString(ImageFilter::Reading, ImageFilter::AllImages),
                                                     NULL,				// selectedFilter,
                                                     QFileDialog::Options(),		// options
                                                     QStringList("file"));		// supportedSchemes

    if (files.isEmpty()) return;			// didn't get a file name
    saver.save(files.first());
    filesController()->importPhoto(files);
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
    qDebug() << "selected" << selCount << "type" << selType;

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
    qDebug() << can;
    mUndoAction->setEnabled(can);
}

void MainWindow::slotCanRedoChanged(bool can)
{
    qDebug() << can;
    mRedoAction->setEnabled(can);
}

void MainWindow::slotUndoTextChanged(const QString &text)
{
    qDebug() << text;
    mUndoAction->setText(text.isEmpty() ? mUndoText : i18n("%2: %1", text, mUndoText));
}

void MainWindow::slotRedoTextChanged(const QString &text)
{
    qDebug() << text;
    mRedoAction->setText(text.isEmpty() ? mRedoText : i18n("%2: %1", text, mRedoText));
}

void MainWindow::slotCleanUndoChanged(bool clean)
{
    qDebug() << "clean" << clean;
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

    QList<QByteArray> imageTypes = QImageReader::supportedMimeTypes();

    QMimeDatabase db;
    QList<QUrl> validUrls;
    foreach (const QUrl &url, urls)
    {
        const QMimeType mime = db.mimeTypeForUrl(url);
        const QByteArray name = mime.name().toLatin1();
        if (imageTypes.contains(name))
        {
            qDebug() << "accept image" << url << "mimetype" << name;
            validUrls.append(url);
        }
        else qWarning() << "reject" << url << "mimetype" << name;
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
