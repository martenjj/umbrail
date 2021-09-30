
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
#include <qtimer.h>

#include <klocalizedstring.h>
#include <ktoggleaction.h>
#include <kselectaction.h>
#include <kactioncollection.h>
#include <kstandardaction.h>
#include <kmessagebox.h>
#include <ksqueezedtextlabel.h>
#include <kactionmenu.h>

#include <kfdialog/recentsaver.h>
#include <kfdialog/imagefilter.h>

#include "filescontroller.h"
#include "filesview.h"
#include "filesmodel.h"
#include "dataindexer.h"
#include "mapcontroller.h"
#include "mapview.h"
#include "settings.h"
#include "settingsdialogue.h"
#include "profilewidget.h"
#include "statisticswidget.h"
#include "mediaplayer.h"
#include "stopdetectdialogue.h"


static const char CONFIG_GROUP[] = "MainWindow";

static const int sbModified = 0;

static const char notUsefulOverlays[] = "elevationprofile,GpsInfo,routing,speedometer";


 
MainWindow::MainWindow(QWidget *pnt)
    : KXmlGuiWindow(pnt),
      ApplicationData()
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
    connect(mUndoStack, &QUndoStack::canUndoChanged, this, &MainWindow::slotCanUndoChanged);
    connect(mUndoStack, &QUndoStack::canRedoChanged, this, &MainWindow::slotCanRedoChanged);
    connect(mUndoStack, &QUndoStack::undoTextChanged, this, &MainWindow::slotUndoTextChanged);
    connect(mUndoStack, &QUndoStack::redoTextChanged, this, &MainWindow::slotRedoTextChanged);
    connect(mUndoStack, &QUndoStack::cleanChanged, this, &MainWindow::slotCleanUndoChanged);

    // Need to set this in ApplicationData before constructing
    // anything else that will use it.
    mMainWidget = this;

    mFilesController = new FilesController(this);
    connect(mFilesController, &FilesController::statusMessage, this, &MainWindow::slotStatusMessage);
    connect(mFilesController, &FilesController::modified, this, [this]() { slotSetModified(true); });
    connect(mFilesController, &FilesController::updateActionState, this, &MainWindow::slotUpdateActionState);

    mFilesView = filesController()->view();		// set in ApplicationData

    mMapController = new MapController(this);
    connect(mMapController, &MapController::statusMessage, this, &MainWindow::slotStatusMessage);
    connect(mMapController, &MapController::modified, this, [this]() { slotSetModified(true); });
    connect(mMapController, &MapController::mapZoomChanged, this, &MainWindow::slotMapZoomChanged);
    connect(mMapController, &MapController::mapDraggedPoints, mFilesController, &FilesController::slotMapDraggedPoints);

    connect(mFilesController, &FilesController::updateMap, mMapController->view(), QOverload<>::of(&QWidget::update));
    // TODO: temp, see FilesView::selectionChanged()
    connect(mFilesController, &FilesController::updateActionState, mMapController->view(), QOverload<>::of(&QWidget::update));

    connect(mMapController->view(), &MapView::createWaypoint, mFilesController, &FilesController::slotAddWaypoint);
    connect(mMapController->view(), &MapView::createRoutepoint, mFilesController, &FilesController::slotAddRoutepoint);

    mSplitter->addWidget(mFilesController->view());
    mSplitter->addWidget(mMapController->view());

    setupStatusBar();
    setupActions();

    readProperties(Settings::self()->config()->group(CONFIG_GROUP));

    mSelectedContainer = nullptr;

    slotSetModified(false);
    slotUpdateActionState();
    slotUpdatePasteState();
}


MainWindow::~MainWindow()
{
    qDebug() << "done";
}


void MainWindow::setupActions()
{
    KActionCollection *ac = actionCollection();

    KStandardAction::quit(this, &MainWindow::close, ac);

    QAction *a = KStandardAction::openNew(this, &MainWindow::slotNewProject, ac);
    a = KStandardAction::open(this, &MainWindow::slotOpenProject, ac);
    mSaveProjectAction = KStandardAction::save(this, &MainWindow::slotSaveProject, ac);
    mSaveProjectAsAction = KStandardAction::saveAs(this, &MainWindow::slotSaveAs, ac);

    mSaveProjectCopyAction = ac->addAction("file_save_copy");
    mSaveProjectCopyAction->setText(i18n("Save Copy As..."));
    mSaveProjectCopyAction->setIcon(QIcon::fromTheme("folder-new"));
    connect(mSaveProjectCopyAction, &QAction::triggered, this, &MainWindow::slotSaveCopy);

    mImportAction = ac->addAction("file_import");
    mImportAction->setText(i18n("Import File..."));
    mImportAction->setIcon(QIcon::fromTheme("document-import"));
    ac->setDefaultShortcut(mImportAction, Qt::CTRL+Qt::Key_I);
    connect(mImportAction, &QAction::triggered, this, &MainWindow::slotImportFile);

    mExportAction = ac->addAction("file_export");
    mExportAction->setText(i18n("Export..."));
    mExportAction->setIcon(QIcon::fromTheme("document-export"));
    ac->setDefaultShortcut(mExportAction, Qt::CTRL+Qt::Key_E);
    connect(mExportAction, &QAction::triggered, this, &MainWindow::slotExportFile);
    mExportAction->setEnabled(false);

    mPhotoAction = ac->addAction("file_add_photo");
    mPhotoAction->setText("Import Photo...");
    mPhotoAction->setIcon(QIcon::fromTheme("image-loading"));
    connect(mPhotoAction, &QAction::triggered, this, &MainWindow::slotImportPhoto);

    mSelectAllAction = KStandardAction::selectAll(filesController()->view(), &FilesView::slotSelectAllSiblings, ac);
    mClearSelectAction = KStandardAction::deselect(filesController()->view(), &QTreeView::clearSelection, ac);
    mClearSelectAction->setIcon(QIcon::fromTheme("edit-clear-list"));

    mUndoAction = KStandardAction::undo(mUndoStack, &QUndoStack::undo, ac);
    mUndoAction->setEnabled(false);
    mUndoText = mUndoAction->text();

    mRedoAction = KStandardAction::redo(mUndoStack, &QUndoStack::redo, ac);
    mRedoAction->setEnabled(false);
    mRedoText = mRedoAction->text();

    mCopyAction = KStandardAction::copy(this, &MainWindow::slotCopy, ac);
    mCopyAction->setEnabled(false);

    mPasteAction = KStandardAction::paste(this, &MainWindow::slotPaste, ac);
    mPasteAction->setEnabled(false);
    connect(QApplication::clipboard(), &QClipboard::dataChanged, this, &MainWindow::slotUpdatePasteState);

    a = ac->addAction("track_expand_all");
    a->setText(i18n("Expand View"));
    a->setIcon(QIcon::fromTheme("application_side_tree"));
    ac->setDefaultShortcut(a, Qt::CTRL+Qt::Key_Period);
    connect(a, &QAction::triggered, filesController()->view(), &FilesView::slotExpandAll);

    a = ac->addAction("track_expand_complete");
    a->setText(i18n("Expand All"));
    ac->setDefaultShortcut(a, Qt::CTRL+Qt::ALT+Qt::SHIFT+Qt::Key_Period);
    connect(a, &QAction::triggered, filesController()->view(), &QTreeView::expandAll);

    a = ac->addAction("track_collapse_all");
    a->setText(i18n("Collapse View"));
    a->setIcon(QIcon::fromTheme("application_side_list"));
    ac->setDefaultShortcut(a, Qt::CTRL+Qt::Key_Comma);
    connect(a, &QAction::triggered, filesController()->view(), &FilesView::slotCollapseAll);

    a = ac->addAction("track_collapse_complete");
    a->setText(i18n("Collapse All"));
    ac->setDefaultShortcut(a, Qt::CTRL+Qt::ALT+Qt::SHIFT+Qt::Key_Comma);
    connect(a, &QAction::triggered, filesController()->view(), &QTreeView::collapseAll);

    mAddTrackAction = ac->addAction("edit_add_track");
    mAddTrackAction->setText(i18n("Add Track"));
    mAddTrackAction->setIcon(QIcon::fromTheme("list-add"));
    connect(mAddTrackAction, &QAction::triggered, filesController(), &FilesController::slotAddTrack);

    mAddFolderAction = ac->addAction("edit_add_folder");
    mAddFolderAction->setText(i18n("Add Folder"));
    mAddFolderAction->setIcon(QIcon::fromTheme("bookmark-new-list"));
    connect(mAddFolderAction, &QAction::triggered, filesController(), &FilesController::slotAddFolder);

    mAddPointAction = ac->addAction("edit_add_point");
    mAddPointAction->setText(i18n("Add Point"));
    mAddPointAction->setIcon(QIcon::fromTheme("list-add"));
    connect(mAddPointAction, &QAction::triggered, filesController(), &FilesController::slotAddPoint);

    mAddWaypointAction = ac->addAction("edit_add_waypoint");
    mAddWaypointAction->setText(i18n("Add Waypoint..."));
    mAddWaypointAction->setIcon(QIcon::fromTheme("list-add"));
    connect(mAddWaypointAction, &QAction::triggered, this, [this]() { filesController()->slotAddWaypoint(); });

    mAddRouteAction = ac->addAction("edit_add_route");
    mAddRouteAction->setText(i18n("Add Route"));
    mAddRouteAction->setIcon(QIcon::fromTheme("list-add"));
    connect(mAddRouteAction, &QAction::triggered, filesController(), &FilesController::slotAddRoute);

    mAddRoutepointAction = ac->addAction("edit_add_routepoint");
    mAddRoutepointAction->setText(i18n("Add Route Point..."));
    mAddRoutepointAction->setIcon(QIcon::fromTheme("list-add"));
    connect(mAddRoutepointAction, &QAction::triggered, this, [this]() { filesController()->slotAddRoutepoint(); });

    // TODO: could these actions be combined with corresponding 2 above?
    a = ac->addAction("map_add_waypoint");
    a->setText(i18n("Create Waypoint..."));
    a->setIcon(QIcon::fromTheme("list-add"));
    connect(a, &QAction::triggered, mapController()->view(), &MapView::slotAddWaypoint);

    a = ac->addAction("map_add_routepoint");
    a->setText(i18n("Create Route Point..."));
    a->setIcon(QIcon::fromTheme("list-add"));
    connect(a, &QAction::triggered, mapController()->view(), &MapView::slotAddRoutepoint);

    mDeleteItemsAction = ac->addAction("edit_delete_track");
    mDeleteItemsAction->setText(i18n("Delete"));
    mDeleteItemsAction->setIcon(QIcon::fromTheme("edit-delete"));
    ac->setDefaultShortcut(mDeleteItemsAction, KStandardShortcut::deleteFile().value(0));
    connect(mDeleteItemsAction, &QAction::triggered, filesController(), &FilesController::slotDeleteItems);

    mSplitTrackAction = ac->addAction("track_split");
    mSplitTrackAction->setText(i18n("Split Segment"));
    mSplitTrackAction->setIcon(QIcon::fromTheme("split"));
    connect(mSplitTrackAction, &QAction::triggered, filesController(), &FilesController::slotSplitSegment);

    mMergeTrackAction = ac->addAction("track_merge");
    mMergeTrackAction->setText(i18n("Merge Segments"));
    mMergeTrackAction->setIcon(QIcon::fromTheme("merge"));
    connect(mMergeTrackAction, &QAction::triggered, filesController(), &FilesController::slotMergeSegments);

    mMoveItemAction = ac->addAction("track_move_item");
    mMoveItemAction->setText(i18n("Move Item..."));
    mMoveItemAction->setIcon(QIcon::fromTheme("go-up"));
    connect(mMoveItemAction, &QAction::triggered, filesController(), &FilesController::slotMoveItem);

    mStopDetectAction = ac->addAction("track_stop_detect");
    mStopDetectAction->setText(i18n("Locate Stops..."));
    mStopDetectAction->setIcon(QIcon::fromTheme("media-playback-stop"));
    connect(mStopDetectAction, &QAction::triggered, this, &MainWindow::slotTrackStopDetect);

    mPropertiesAction = ac->addAction("track_properties");
    // text set in slotUpdateActionState() below
    QList<QKeySequence> cuts;
    cuts.append(QKeySequence(Qt::CTRL+Qt::Key_Return));
    cuts.append(QKeySequence(Qt::CTRL+Qt::Key_Enter));
    ac->setDefaultShortcuts(mPropertiesAction, cuts);
    mPropertiesAction->setIcon(QIcon::fromTheme("document-properties"));
    connect(mPropertiesAction, &QAction::triggered, filesController(), &FilesController::slotTrackProperties);

    mWaypointStatusAction = new KSelectAction(QIcon::fromTheme("favorites"), i18nc("@action:inmenu", "Waypoint Status"), this);
    mWaypointStatusAction->setToolBarMode(KSelectAction::MenuMode);
    ac->addAction("waypoint_status", mWaypointStatusAction);

    a = mWaypointStatusAction->addAction(QIcon::fromTheme("unknown"), TrackData::formattedWaypointStatus(TrackData::StatusNone));
    a->setData(TrackData::StatusNone);
    connect(a, &QAction::triggered, filesController(), &FilesController::slotSetWaypointStatus);

    a = mWaypointStatusAction->addAction(QIcon::fromTheme("task-ongoing"), TrackData::formattedWaypointStatus(TrackData::StatusTodo));
    a->setData(TrackData::StatusTodo);
    connect(a, &QAction::triggered, filesController(), &FilesController::slotSetWaypointStatus);

    a = mWaypointStatusAction->addAction(QIcon::fromTheme("task-complete"), TrackData::formattedWaypointStatus(TrackData::StatusDone));
    a->setData(TrackData::StatusDone);
    connect(a, &QAction::triggered, filesController(), &FilesController::slotSetWaypointStatus);

    a = mWaypointStatusAction->addAction(QIcon::fromTheme("task-attempt"), TrackData::formattedWaypointStatus(TrackData::StatusQuestion));
    a->setData(TrackData::StatusQuestion);
    connect(a, &QAction::triggered, filesController(), &FilesController::slotSetWaypointStatus);

    a = mWaypointStatusAction->addAction(QIcon::fromTheme("task-reject"), TrackData::formattedWaypointStatus(TrackData::StatusUnwanted));
    a->setData(TrackData::StatusUnwanted);
    connect(a, &QAction::triggered, filesController(), &FilesController::slotSetWaypointStatus);

    mProfileAction = ac->addAction("track_profile");
    mProfileAction->setText(i18n("Elevation/Speed Profile..."));
    mProfileAction->setIcon(QIcon::fromTheme("office-chart-line-stacked"));
    connect(mProfileAction, &QAction::triggered, this, &MainWindow::slotTrackProfile);

    mStatisticsAction = ac->addAction("track_statistics");
    mStatisticsAction->setText(i18n("Statistics/Quality..."));
    mStatisticsAction->setIcon(QIcon::fromTheme("kt-check-data"));
    connect(mStatisticsAction, &QAction::triggered, this, &MainWindow::slotTrackStatistics);

    a = ac->addAction("track_play_media");
    a->setText(i18nc("@action:inmenu", "View Media"));
    a->setIcon(QIcon::fromTheme("media-playback-start"));
    ac->setDefaultShortcut(a, Qt::CTRL+Qt::Key_P);
    connect(a, &QAction::triggered, this, &MainWindow::slotPlayMedia);
    mPlayMediaAction = a;

    a = ac->addAction("file_open_media");
    a->setText(i18nc("@action:inmenu", "Open Media With..."));
    a->setIcon(QIcon::fromTheme("document-open"));
    connect(a, &QAction::triggered, this, &MainWindow::slotOpenMedia);
    mOpenMediaAction = a;

    a = ac->addAction("file_save_media");
    a->setText(i18nc("@action:inmenu", "Save Media As..."));
    a->setIcon(QIcon::fromTheme("folder-video"));
    connect(a, &QAction::triggered, this, &MainWindow::slotSaveMedia);
    mSaveMediaAction = a;

    a = ac->addAction("track_time_zone");
    a->setText(i18nc("@action:inmenu", "Set Time Zone..."));
    a->setIcon(QIcon::fromTheme("preferences-system-time"));
    connect(a, &QAction::triggered, filesController(), &FilesController::slotSetTimeZone);

    a = ac->addAction("map_save");
    a->setText(i18n("Save As Image..."));
    a->setIcon(QIcon::fromTheme("folder-picture"));
    connect(a, &QAction::triggered, mapController(), &MapController::slotSaveImage);

    a = ac->addAction("map_set_home");
    a->setText(i18n("Set Home Position"));
    a->setIconText(i18n("Set Home"));
    a->setIcon(QIcon::fromTheme("bookmarks"));
    connect(a, &QAction::triggered, mapController(), &MapController::slotSetHome);

    a = ac->addAction("map_go_home");
    a->setText(i18n("Go to Home Position"));
    a->setIconText(i18n("Go Home"));
    a->setIcon(QIcon::fromTheme("go-home"));
    ac->setDefaultShortcut(a, Qt::CTRL+Qt::Key_Home);
    connect(a, &QAction::triggered, mapController(), &MapController::slotGoHome);

    mMapZoomInAction = ac->addAction(KStandardAction::ZoomIn, "map_zoom_in");
    connect(mMapZoomInAction, &QAction::triggered, this, [this]() { mapController()->view()->zoomIn(); });

    mMapZoomOutAction = ac->addAction(KStandardAction::ZoomOut, "map_zoom_out");
    connect(mMapZoomOutAction, &QAction::triggered, this, [this]() { mapController()->view()->zoomOut(); });

    a = ac->addAction("map_set_zoom");
    a->setText(i18n("Set Standard Zoom"));
    a->setIconText(i18n("Set Zoom"));
    a->setIcon(QIcon::fromTheme("bookmarks"));
    connect(a, &QAction::triggered, mapController(), &MapController::slotSetZoom);

    a = ac->addAction("map_zoom_standard");
    a->setText(i18n("Reset to Standard Zoom"));
    a->setIconText(i18n("Reset Zoom"));
    a->setIcon(QIcon::fromTheme("zoom-original"));
    ac->setDefaultShortcut(a, Qt::CTRL+Qt::Key_1);
    connect(a, &QAction::triggered, mapController(), &MapController::slotResetZoom);

    mMapGoToAction = ac->addAction("map_go_selection");
    mMapGoToAction->setText(i18n("Show on Map"));
    mMapGoToAction->setIcon(QIcon::fromTheme("marble"));
    ac->setDefaultShortcut(mMapGoToAction, Qt::CTRL+Qt::Key_G);
    connect(mMapGoToAction, &QAction::triggered, this, &MainWindow::slotMapGotoSelection);

    a = ac->addAction("map_select_theme");
    a->setText(i18n("Select Theme..."));
    a->setIcon(QIcon::fromTheme("image-loading"));
    connect(a, &QAction::triggered, mapController(), &MapController::slotSelectTheme);

    a = ac->addAction("map_find_address");
    a->setText(i18n("Position Information..."));
    a->setIcon(QIcon::fromTheme("view-pim-mail"));
    connect(a, &QAction::triggered, mapController()->view(), &MapView::slotFindAddress);

    mReadOnlyAction = ac->addAction("settings_read_only");
    mReadOnlyAction->setText(i18n("Read Only"));
    mReadOnlyAction->setCheckable(true);
    connect(mReadOnlyAction, &QAction::toggled, this, &MainWindow::slotReadOnly);

    a = ac->addAction("reset_cancel");
    a->setText(i18n("Reset/Cancel"));			// only seen in "Configure Shortcuts"
    a->setIcon(QIcon::fromTheme("dialog-cancel"));
    ac->setDefaultShortcut(a, Qt::Key_Escape);
    connect(a, &QAction::triggered, this, &MainWindow::slotResetAndCancel);

    a = ac->addAction("map_open_osm");
    a->setText(i18n("View on OpenStreetMap..."));
    a->setIcon(QIcon::fromTheme("openstreetmap"));
    connect(a, &QAction::triggered, this, [this]() { openExternalMap(MapBrowser::OSM); });

#ifdef ENABLE_OPEN_WITH_GOOGLE
    a = ac->addAction("map_open_google");
    a->setText(i18n("View with Google Maps..."));
    a->setIcon(QIcon::fromTheme("googlemaps"));
    connect(a, &QAction::triggered, this, [this]() { openExternalMap(MapBrowser::Google); });
#endif // ENABLE_OPEN_WITH_GOOGLE

#ifdef ENABLE_OPEN_WITH_BING
    a = ac->addAction("map_open_bing");
    a->setText(i18n("View with Bing Maps..."));
    a->setIcon(QIcon::fromTheme("bingmaps"));
    connect(a, &QAction::triggered, this, [this]() { openExternalMap(MapBrowser::Bing); });
#endif // ENABLE_OPEN_WITH_BING

    const MapView *mapView = mapController()->view();

    KActionMenu *itemsMenu = new KActionMenu(this);
    // For consistency with "Waypoint Status" which is a KSelectAction
    itemsMenu->setPopupMode(QToolButton::InstantPopup);
    QStringList layerIds = mapView->allLayers(false);
    foreach (const QString &id, layerIds)
    {
        a = mapView->actionForLayer(id);
        if (a==nullptr) continue;

        connect(a, &QAction::triggered, mapView, &MapView::slotShowLayer);
        itemsMenu->addAction(a);
    }
    a = ac->addAction("map_show_layers", itemsMenu);
    a->setText(i18n("Show Layers"));
    a->setIcon(QIcon::fromTheme("layer-visible-on"));

    itemsMenu = new KActionMenu(this);
    itemsMenu->setPopupMode(QToolButton::InstantPopup);
    QStringList notUseful = QString(notUsefulOverlays).split(',');
    QStringList itemIds = mapView->allOverlays(false);
    foreach (const QString &itemId, itemIds)
    {
        if (notUseful.contains(itemId)) continue;

        a = mapView->actionForOverlay(itemId);
        if (a==nullptr) continue;

        connect(a, &QAction::triggered, mapController()->view(), &MapView::slotShowOverlay);
        itemsMenu->addAction(a);
    }
    a = ac->addAction("map_show_overlays", itemsMenu);
    a->setText(i18n("Show Overlays"));
    a->setIcon(QIcon::fromTheme("flag-black"));

    mMapDragAction = new KToggleAction(QIcon::fromTheme("transform-move"), i18n("Move Mode"), this);
    ac->setDefaultShortcut(mMapDragAction, Qt::CTRL+Qt::Key_M);
    connect(mMapDragAction, &QAction::triggered, this, &MainWindow::slotMapMovePoints);
    ac->addAction("map_move_points", mMapDragAction);

    a = KStandardAction::preferences(this, &MainWindow::slotPreferences, ac);

    a = ac->addAction("help_about_marble");
    a->setText(i18n("About Marble"));
    a->setIcon(QIcon::fromTheme("marble"));
    connect(a, &QAction::triggered, mapController(), &MapController::slotAboutMarble);

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
    if (!isModified()) return (true);			// not modified, OK to close

    QString query;
    if (hasFileName()) query = xi18nc("@info", "File <emphasis strong=\"1\"><filename>%1</filename></emphasis> has been modified. Save changes?", documentName());
    else query = i18n("File has been modified. Save changes?");

    switch (KMessageBox::warningYesNoCancel(this, query, QString(),
                                            KStandardGuiItem::save(), KStandardGuiItem::discard()))
    {
case KMessageBox::Yes:
        slotSaveProject();
        return (!isModified());				// check that save worked

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
    if (!splitterState.isEmpty()) mSplitter->restoreState(QByteArray::fromBase64(splitterState.toLocal8Bit()));
}



// Error reporting and status messages are done in FilesController::exportFile()
bool MainWindow::save(const QUrl &to, ImporterExporterBase::Options options)
{
    qDebug() << "to" << to;

    if (!to.isValid()) return (false);			// should never happen

    TrackDataFile *tdf = filesController()->model()->rootFileItem();
    if (tdf==nullptr) return (false);			// should never happen

    // metadata from map controller
    tdf->setMetadata("position", mapController()->view()->currentPosition());

    // metadata for save file
    tdf->setMetadata("creator", QApplication::applicationDisplayName());
    tdf->setMetadata("time", QDateTime::currentDateTimeUtc().toString(Qt::ISODate));

    return (filesController()->exportFile(to, tdf, options)==FilesController::StatusOk);
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
    if (tdf!=nullptr)
    {
        QVariant s = tdf->metadata("position");
        qDebug() << "pos metadata" << s;
        QSignalBlocker block(mapController()->view());	// no status bar update from zooming
        if (!s.isNull()) mapController()->view()->setCurrentPosition(s.toString());
        else mapController()->gotoSelection(QList<TrackDataItem *>() << tdf);
    }

    filesController()->view()->expandToDepth(1);	// expand to show segments
    if (Settings::fileCheckTimezone())			// check time zone is set
    {
        QTimer::singleShot(0, filesController(), &FilesController::slotCheckTimeZone);
    }
    return (status);
}



void MainWindow::slotStatusMessage(const QString &text)
{
    mStatusMessage->setText(text);
    mStatusMessage->repaint();				// show new message immediately
}



void MainWindow::slotNewProject()
{
    MainWindow *w = new MainWindow(nullptr);
    w->filesController()->initNew();
    w->show();
}



void MainWindow::slotOpenProject()
{
    RecentSaver saver("project");
    QUrl file = QFileDialog::getOpenFileUrl(this,					// parent
                                            i18n("Open Tracks File"),			// caption
                                            saver.recentUrl(),				// dir
                                            FilesController::allProjectFilters(true),	// filter
                                            nullptr,					// selectedFilter,
                                            QFileDialog::Options(),			// options
                                            QStringList("file"));			// supportedSchemes

    if (!file.isValid()) return;			// didn't get a file name
    saver.save(file);

    if (filesController()->model()->isEmpty()) loadProject(file);
    else
    {
        MainWindow *w = new MainWindow(nullptr);
        const bool ok = w->loadProject(file);
        if (ok) w->show();
        else w->deleteLater();
    }
}


bool MainWindow::loadProject(const QUrl &loadFrom, bool readOnly)
{
    if (!loadFrom.isValid()) return (false);
    qDebug() << "from" << loadFrom << "readonly?" << readOnly;

    FilesController::Status status = load(loadFrom);	// load in data file
    if (status!=FilesController::StatusOk && status!=FilesController::StatusResave) return (false);

    setFileName(loadFrom);				// record file name
    mUndoStack->clear();				// clear undo history
    slotSetModified(status==FilesController::StatusResave);
							// ensure window title updated
    setReadOnly(readOnly);				// record read-only state
    mReadOnlyAction->setChecked(isReadOnly());		// set state in GUI
    return (true);
}



void MainWindow::slotSaveProject()
{
    if (!hasFileName())
    {
        slotSaveAs();
        return;
    }

    QUrl projectFile = fileName();
    qDebug() << "to" << projectFile;

    if (save(projectFile, ImporterExporterBase::NoOption))
    {
        TrackDataFile *tdf = filesController()->model()->rootFileItem();
        if (tdf!=nullptr) tdf->setFileName(projectFile);
							// set file name in root item
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
                                            nullptr,					// selectedFilter,
                                            QFileDialog::Options(),			// options
                                            QStringList("file"));			// supportedSchemes

    if (!file.isValid()) return;			// didn't get a file name
    saver.save(file);

    setFileName(file);
    slotSaveProject();
}


void MainWindow::slotSaveCopy()
{
    RecentSaver saver("projectcopy");
    QUrl file = QFileDialog::getSaveFileUrl(this,					// parent
                                            i18n("Save Copy of Tracks File As"),	// caption
                                            saver.recentUrl("untitled"),		// dir
                                            FilesController::allProjectFilters(false),	// filter
                                            nullptr,					// selectedFilter,
                                            QFileDialog::Options(),			// options
                                            QStringList("file"));			// supportedSchemes

    if (!file.isValid()) return;			// didn't get a file name
    saver.save(file);

    qDebug() << "to" << file;
    save(file, ImporterExporterBase::NoOption);
}


void MainWindow::slotImportFile()
{
    RecentSaver saver("import");
    QUrl file = QFileDialog::getOpenFileUrl(this,					// parent
                                            i18n("Import File"),			// caption
                                            saver.recentUrl(),				// dir
                                            FilesController::allImportFilters(),	// filter
                                            nullptr,					// selectedFilter,
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
                                            saver.recentUrl(documentName(true)),	// dir
                                            FilesController::allExportFilters(),	// filter
                                            nullptr,					// selectedFilter,
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
                                                     nullptr,				// selectedFilter,
                                                     QFileDialog::Options(),		// options
                                                     QStringList("file"));		// supportedSchemes

    if (files.isEmpty()) return;			// didn't get a file name
    saver.save(files.first());
    filesController()->importPhoto(files);
}


void MainWindow::slotSetModified(bool mod)
{
    setModified(mod);
    mSaveProjectAction->setEnabled(mod && hasFileName());
    mModifiedIndicator->setEnabled(mod);
    setWindowTitle(documentName()+" [*]");
    setWindowModified(mod);

    mSaveProjectAsAction->setEnabled(!filesController()->model()->isEmpty());
    mSaveProjectCopyAction->setEnabled(!filesController()->model()->isEmpty());
}


void MainWindow::slotUpdateActionState()
{
    int selCount = filesController()->view()->selectedCount();
    TrackData::Type selType = filesController()->view()->selectedType();
    qDebug() << "selected" << selCount << "type" << selType;

    bool copyEnabled = false;
    bool propsEnabled = false;
    bool profileEnabled = false;
    bool stopsEnabled = false;
    QString propsText = i18nc("@action:inmenu", "Properties...");
    bool delEnabled = true;
    QString delText = i18nc("@action:inmenu", "Delete");
    bool moveEnabled = false;
    QString moveText = i18nc("@action:inmenu", "Move Item...");
    bool playEnabled = false;
    QString playText = i18nc("@action:inmenu", "View Media");
    bool statusEnabled = false;
    int statusValue = TrackData::StatusInvalid;
    bool splitEnabled = false;
    QString splitText =  i18nc("@action:inmenu", "Split");
    bool mergeEnabled = false;
    QString mergeText =  i18nc("@action:inmenu", "Merge");

    const TrackDataItem *selectedContainer = nullptr;
    switch (selType)
    {
case TrackData::File:
        propsText = i18ncp("@action:inmenu", "File Properties...", "Files Properties...", selCount);
        propsEnabled = true;
        delEnabled = false;
        stopsEnabled = profileEnabled = true;
        break;

case TrackData::Track:
        propsText = i18ncp("@action:inmenu", "Track Properties...", "Tracks Properties...", selCount);
        propsEnabled = true;
        delText = i18ncp("@action:inmenu", "Delete Track", "Delete Tracks", selCount);
        selectedContainer = filesController()->view()->selectedItem();
        stopsEnabled = profileEnabled = true;
        break;

case TrackData::Route:
        propsText = i18ncp("@action:inmenu", "Route Properties...", "Routes Properties...", selCount);
        propsEnabled = true;
        delText = i18ncp("@action:inmenu", "Delete Route", "Delete Routes", selCount);
        selectedContainer = filesController()->view()->selectedItem();
        profileEnabled = true;
        mergeEnabled = (selCount>1);
        mergeText = i18nc("@action:inmenu", "Merge Routes");
        break;

case TrackData::Segment:
        propsText = i18ncp("@action:inmenu", "Segment Properties...", "Segments Properties...", selCount);
        propsEnabled = true;
        delText = i18ncp("@action:inmenu", "Delete Segment", "Delete Segments", selCount);
        moveEnabled = true;
        moveText = i18nc("@action:inmenu", "Move Segment...");
        stopsEnabled = profileEnabled = true;
        mergeEnabled = (selCount>1);
        mergeText = i18nc("@action:inmenu", "Merge Segments");
        break;

case TrackData::Point:
        propsText = i18ncp("@action:inmenu", "Point Properties...", "Points Properties...", selCount);
        propsEnabled = true;
        delText = i18ncp("@action:inmenu", "Delete Point", "Delete Points", selCount);
        selectedContainer = filesController()->view()->selectedItem()->parent();
        stopsEnabled = profileEnabled = (selCount>1);
        copyEnabled = true;
        splitEnabled = (selCount==1);
        splitText = i18nc("@action:inmenu", "Split Segment");
        break;

case TrackData::Routepoint:
        propsText = i18ncp("@action:inmenu", "Route Point Properties...", "Route Points Properties...", selCount);
        propsEnabled = true;
        delText = i18ncp("@action:inmenu", "Delete Route Point", "Delete Route Points", selCount);
        selectedContainer = filesController()->view()->selectedItem()->parent();
        profileEnabled = (selCount>1);
        copyEnabled = true;
        splitEnabled = (selCount==1);
        splitText = i18nc("@action:inmenu", "Split Route");
        break;

case TrackData::Folder:
        propsText = i18ncp("@action:inmenu", "Folder Properties...", "Folders Properties...", selCount);
        propsEnabled = true;
        delText = i18ncp("@action:inmenu", "Delete Folder", "Delete Folders", selCount);
        selectedContainer = filesController()->view()->selectedItem();
        moveEnabled = true;
        moveText = i18nc("@action:inmenu", "Move Folder...");
        break;

case TrackData::Waypoint:
        propsText = i18ncp("@action:inmenu", "Waypoint Properties...", "Waypoints Properties...", selCount);
        propsEnabled = true;
        delText = i18ncp("@action:inmenu", "Delete Waypoint", "Delete Waypoints", selCount);
        moveEnabled = true;
        moveText = i18ncp("@action:inmenu", "Move Waypoint...", "Move Waypoints...", selCount);
        selectedContainer = filesController()->view()->selectedItem()->parent();
        statusEnabled = true;
        copyEnabled = true;

        if (selCount==1)
        {
            const TrackDataWaypoint *tdw = dynamic_cast<const TrackDataWaypoint *>(filesController()->view()->selectedItem());
            if (tdw!=nullptr)
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
    mDeleteItemsAction->setEnabled(delEnabled && !isReadOnly());
    mDeleteItemsAction->setText(delText);
    mProfileAction->setEnabled(profileEnabled);
    mStatisticsAction->setEnabled(profileEnabled);
    mStopDetectAction->setEnabled(stopsEnabled);

    mPlayMediaAction->setEnabled(playEnabled);
    mPlayMediaAction->setText(playText);
    mOpenMediaAction->setEnabled(playEnabled);
    mSaveMediaAction->setEnabled(playEnabled);

    mSelectAllAction->setEnabled(selCount>0 && selType!=TrackData::Mixed);
    mClearSelectAction->setEnabled(selCount>0);
    mMapGoToAction->setEnabled(selCount>0 && selType!=TrackData::Mixed);
    mCopyAction->setEnabled(copyEnabled);

    mSplitTrackAction->setEnabled(splitEnabled && !isReadOnly());
    if (splitEnabled) mSplitTrackAction->setText(splitText);
    mMergeTrackAction->setEnabled(mergeEnabled && !isReadOnly());
    if (mergeEnabled) mMergeTrackAction->setText(mergeText);

    mMoveItemAction->setEnabled(moveEnabled && !isReadOnly());
    mMoveItemAction->setText(moveText);
    mAddTrackAction->setEnabled(selCount==1 && selType==TrackData::File && !isReadOnly());
    mAddRouteAction->setEnabled(selCount==1 && selType==TrackData::File && !isReadOnly());
    mAddFolderAction->setEnabled(selCount==1 && (selType==TrackData::File ||
                                                 selType==TrackData::Folder) && !isReadOnly());
    mAddWaypointAction->setEnabled(selCount==1 && (selType==TrackData::Folder ||
                                                   selType==TrackData::Point ||
                                                   selType==TrackData::Waypoint) && !isReadOnly());
    mAddRoutepointAction->setEnabled(selCount==1 && (selType==TrackData::Route ||
                                                     selType==TrackData::Point ||
                                                     selType==TrackData::Waypoint) && !isReadOnly());

    mWaypointStatusAction->setEnabled(statusEnabled && !isReadOnly());
    QList<QAction *> acts = mWaypointStatusAction->actions();
    for (QList<QAction *>::const_iterator it = acts.constBegin(); it!=acts.constEnd(); ++it)
    {
        QAction *act = (*it);
        act->setChecked(statusValue==act->data().toInt());
    }

    if (selCount==1 && selType==TrackData::Point)
    {							// not first point in segment
        const QModelIndex idx = filesController()->model()->indexForItem(filesController()->view()->selectedItem());
        mAddPointAction->setEnabled(idx.row()>0 && !isReadOnly());
    }
    else mAddPointAction->setEnabled(false);

    // If there is a selected container or point(s), then move points mode
    // is allowed to be entered;  otherwise, it is disabled.
    //
    // If there is a selected container and it is the same as the currently
    // selected container, then move points mode can stay at the same state
    // as it currently is.  Otherwise, it is forced off.

    if (selectedContainer!=nullptr)
    {
        if (selectedContainer!=mSelectedContainer)
        {
            mMapDragAction->setChecked(false);
            slotMapMovePoints();
        }
        mMapDragAction->setEnabled(true && !isReadOnly());
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
    mUndoAction->setEnabled(can && !isReadOnly());
}

void MainWindow::slotCanRedoChanged(bool can)
{
    qDebug() << can;
    mRedoAction->setEnabled(can && !isReadOnly());
}

void MainWindow::slotUndoTextChanged(const QString &text)
{
    mUndoAction->setText(text.isEmpty() ? mUndoText : i18n("%2: %1", text, mUndoText));
}

void MainWindow::slotRedoTextChanged(const QString &text)
{
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
    const bool on = mMapDragAction->isChecked();
    mapController()->view()->setMovePointsMode(on);
    filesController()->view()->setMovePointsMode(on);
}


void MainWindow::slotTrackProfile()
{
    ProfileWidget *w = new ProfileWidget(this);
    w->setAttribute(Qt::WA_DeleteOnClose);
    w->setModal(false);
    w->show();
}


void MainWindow::slotTrackStatistics()
{
    StatisticsWidget *w = new StatisticsWidget(this);
    w->setAttribute(Qt::WA_DeleteOnClose);
    w->setModal(false);
    w->setWindowTitle(i18n("Data Statistics/Quality"));
    w->show();
}


// TODO: status messages from player
void MainWindow::slotPlayMedia()
{
    const TrackDataWaypoint *tdw = dynamic_cast<const TrackDataWaypoint *>(filesController()->view()->selectedItem());
    Q_ASSERT(tdw!=nullptr);
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
    Q_ASSERT(tdw!=nullptr);
    if (tdw->isMediaType()) MediaPlayer::openMediaFile(tdw);
}


void MainWindow::slotSaveMedia()
{
    const TrackDataWaypoint *tdw = dynamic_cast<const TrackDataWaypoint *>(filesController()->view()->selectedItem());
    Q_ASSERT(tdw!=nullptr);
    if (tdw->isMediaType()) MediaPlayer::saveMediaFile(tdw);
}


void MainWindow::slotExecuteCommand(QUndoCommand *cmd)
{
    if (mUndoStack!=nullptr) mUndoStack->push(cmd);	// do via undo system
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
    if (isReadOnly())
    {
        ev->ignore();
        return;
    }

    if (ev->dropAction()!=Qt::CopyAction) return;
    const QMimeData *mimeData = ev->mimeData();
    if (acceptMimeData(mimeData)) ev->accept();
}


void MainWindow::slotCopy()
{
    qDebug();

    QUrl projectFile("clipboard:/a.gpx");		// selects clipboard, sets format
    bool ok = save(projectFile, ImporterExporterBase::ToClipboard|ImporterExporterBase::SelectionOnly);
    if (!ok) qWarning() << "Save to clipboard failed";
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

    mPasteAction->setEnabled(enable && !isReadOnly());
}


void MainWindow::slotTrackStopDetect()
{
    StopDetectDialogue *d = new StopDetectDialogue(this);
    d->show();
}


void MainWindow::slotResetAndCancel()
{
    mapController()->view()->cancelDrag();
    filesController()->view()->clearSelection();
}


void MainWindow::slotReadOnly(bool on)
{
    qDebug() << on;
    setReadOnly(on);

    slotUpdateActionState();
    mPhotoAction->setEnabled(!on);
    mImportAction->setEnabled(!on);

    // Update these to reflect the current state,
    // overriden if the file is read only.
    slotCanUndoChanged(mUndoStack->canUndo());
    slotCanRedoChanged(mUndoStack->canRedo());
}


void MainWindow::openExternalMap(MapBrowser::MapProvider map)
{
    mapController()->openExternalMap(map, filesController()->view()->selectedItems());
}
