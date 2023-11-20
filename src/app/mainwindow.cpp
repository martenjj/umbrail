//////////////////////////////////////////////////////////////////////////
//									//
//  Project:	Umbrail - GPX track viewer and editor			//
//									//
//////////////////////////////////////////////////////////////////////////
//									//
//  Copyright (c) 2014-2022 Jonathan Marten <jjm@keelhaul.me.uk>	//
//  Home and download page: <http://github.com/martenjj/umbrail>	//
//									//
//  This program is free software; you can redistribute it and/or	//
//  modify it under the terms of the GNU General Public License as	//
//  published by the Free Software Foundation, either version 3 of	//
//  the License or (at your option) any later version.			//
//									//
//  It is distributed in the hope that it will be useful, but		//
//  WITHOUT ANY WARRANTY;  without even the implied warranty of		//
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the	//
//  GNU General Public License for more details.			//
//									//
//  You should have received a copy of the GNU General Public License	//
//  along with this program;  see the file COPYING for further		//
//  details.  If not, see <http://gnu.org/licenses/gpl>.      		//
//									//
//////////////////////////////////////////////////////////////////////////

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
#include <qstackedwidget.h>
#include <qtabwidget.h>

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
#include "pointsview.h"
#include "dataindexer.h"
#include "mapcontroller.h"
#include "mapview.h"
#include "settings.h"
#include "settingsdialogue.h"
#include "profilewidget.h"
#include "statisticswidget.h"
#include "mediaplayer.h"
#include "stopdetectdialogue.h"
#include "pointiconprovider.h"
#include "importfiledialogue.h"
#include "exportfiledialogue.h"


static const char CONFIG_GROUP[] = "MainWindow";

static const int sbModified = 0;

static const char notUsefulOverlays[] = "elevationprofile,GpsInfo,routing,speedometer";

//////////////////////////////////////////////////////////////////////////
//									//
//  Constructor, destructor and actions					//
//									//
//////////////////////////////////////////////////////////////////////////
 
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

    mWidgetStack = new QStackedWidget(this);
    setCentralWidget(mWidgetStack);

    mTreeModeSplitter = new QSplitter(Qt::Horizontal, this);
    mTreeModeSplitter->setChildrenCollapsible(false);
    mWidgetStack->addWidget(mTreeModeSplitter);		// index 0

    mMainTabs = new QTabWidget(this);
    mMainTabs->setTabsClosable(false);
    mWidgetStack->addWidget(mMainTabs);			// index 1

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

    mFilesView = filesController()->filesView();		// set in ApplicationData
    mPointsView = filesController()->pointsView();		// set in ApplicationData

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

    mMapTabPlaceholder = new QStackedWidget(this);
    mMapTreePlaceholder = new QStackedWidget(this);

    mMainTabs->addTab(mPointsView, QIcon::fromTheme("view-list-text"), i18n("Points"));	// index 0
    mMainTabs->addTab(mMapTabPlaceholder, QIcon::fromTheme("marble"), i18n("Map"));	// index 1

    mTreeModeSplitter->addWidget(mFilesController->filesView());
    mTreeModeSplitter->addWidget(mMapTreePlaceholder);

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
    mapController()->view()->setParent(nullptr);	// avoid double delete
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
    mSaveProjectCopyAction->setIcon(QIcon::fromTheme("document-save-all"));
    connect(mSaveProjectCopyAction, &QAction::triggered, this, &MainWindow::slotSaveCopy);

    mImportAction = ac->addAction("file_import");
    mImportAction->setText(i18n("Import File..."));
    mImportAction->setIcon(QIcon::fromTheme("document-import"));
    ac->setDefaultShortcut(mImportAction, Qt::CTRL+Qt::Key_I);
    connect(mImportAction, &QAction::triggered, this, &MainWindow::slotImportFile);

    mExportAction = ac->addAction("file_export");
    mExportAction->setText(i18n("Export File..."));
    mExportAction->setIcon(QIcon::fromTheme("document-export"));
    ac->setDefaultShortcut(mExportAction, Qt::CTRL+Qt::Key_E);
    connect(mExportAction, &QAction::triggered, this, &MainWindow::slotExportFile);

    mPhotoAction = ac->addAction("file_add_photo");
    mPhotoAction->setText("Import Photo...");
    mPhotoAction->setIcon(QIcon::fromTheme("image-loading"));
    connect(mPhotoAction, &QAction::triggered, this, &MainWindow::slotImportPhoto);

    mSelectAllAction = KStandardAction::selectAll(filesController()->filesView(), &FilesView::slotSelectAllSiblings, ac);
    mClearSelectAction = KStandardAction::deselect(filesController()->filesView(), &QTreeView::clearSelection, ac);
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

    mViewModeAction = new KToggleAction(i18n("Points List"), this);
    connect(mViewModeAction, &QAction::triggered, this, &MainWindow::slotViewPointsMode);
    ac->addAction("view_points_mode", mViewModeAction);
    ac->setDefaultShortcut(mViewModeAction, Qt::CTRL+Qt::SHIFT+Qt::Key_P);

    a = ac->addAction("track_expand_all");
    a->setText(i18n("Expand Tree"));
    a->setIcon(QIcon::fromTheme("application_side_tree"));
    ac->setDefaultShortcut(a, Qt::CTRL+Qt::Key_Period);
    connect(a, &QAction::triggered, filesController()->filesView(), &FilesView::slotExpandAll);

    a = ac->addAction("track_expand_complete");
    a->setText(i18n("Expand All"));
    ac->setDefaultShortcut(a, Qt::CTRL+Qt::ALT+Qt::SHIFT+Qt::Key_Period);
    connect(a, &QAction::triggered, filesController()->filesView(), &QTreeView::expandAll);

    a = ac->addAction("track_collapse_all");
    a->setText(i18n("Collapse Tree"));
    a->setIcon(QIcon::fromTheme("application_side_list"));
    ac->setDefaultShortcut(a, Qt::CTRL+Qt::Key_Comma);
    connect(a, &QAction::triggered, filesController()->filesView(), &FilesView::slotCollapseAll);

    a = ac->addAction("track_collapse_complete");
    a->setText(i18n("Collapse All"));
    ac->setDefaultShortcut(a, Qt::CTRL+Qt::ALT+Qt::SHIFT+Qt::Key_Comma);
    connect(a, &QAction::triggered, filesController()->filesView(), &QTreeView::collapseAll);

    mAddTrackAction = ac->addAction("edit_add_track");
    mAddTrackAction->setText(i18n("Add Track"));
    mAddTrackAction->setIcon(QIcon::fromTheme("list-add"));
    connect(mAddTrackAction, &QAction::triggered, filesController(), &FilesController::slotAddTrack);

    mAddFolderAction = ac->addAction("edit_add_folder");
    mAddFolderAction->setText(i18n("Add Folder"));
    mAddFolderAction->setIcon(QIcon::fromTheme("folder-new"));
    connect(mAddFolderAction, &QAction::triggered, filesController(), &FilesController::slotAddFolder);

    mAddTrackpointAction = ac->addAction("edit_add_trackpoint");
    mAddTrackpointAction->setText(i18n("Add Track Point"));
    mAddTrackpointAction->setIcon(QIcon::fromTheme("list-add"));
    connect(mAddTrackpointAction, &QAction::triggered, filesController(), &FilesController::slotAddTrackpoint);

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
    mSplitTrackAction->setText(i18n("Split"));
    mSplitTrackAction->setIcon(QIcon::fromTheme("split"));
    connect(mSplitTrackAction, &QAction::triggered, filesController(), &FilesController::slotSplitSegment);

    mMergeTrackAction = ac->addAction("track_merge");
    mMergeTrackAction->setText(i18n("Merge"));
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

    a = ac->addAction("track_manage_categories");
    a->setText(i18n("Manage Categories..."));
    a->setIcon(QIcon::fromTheme("folder-green"));
    connect(a, &QAction::triggered, filesController(), &FilesController::slotManageCategories);

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
    const QStringList layerIds = mapView->allLayers(false);
    for (const QString &id : layerIds)
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
    const QStringList notUseful = QString(notUsefulOverlays).split(',');
    const QStringList itemIds = mapView->allOverlays(false);
    for (const QString &itemId : itemIds)
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

//////////////////////////////////////////////////////////////////////////
//									//
//  Status bar								//
//									//
//////////////////////////////////////////////////////////////////////////

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


void MainWindow::slotStatusMessage(const QString &text)
{
    mStatusMessage->setText(text);
    mStatusMessage->repaint();				// show new message immediately
}

//////////////////////////////////////////////////////////////////////////
//									//
//  Creating and closing windows					//
//									//
//////////////////////////////////////////////////////////////////////////

void MainWindow::slotNewProject()
{
    MainWindow *w = new MainWindow(nullptr);
    w->filesController()->initNew();
    w->show();
}


void MainWindow::closeEvent(QCloseEvent *ev)
{
    PointIconProvider::self()->aboutToQuit();		// dump cache statistics

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

    switch (KMessageBox::warningTwoActionsCancel(this, query, QString(),
                                                 KStandardGuiItem::save(), KStandardGuiItem::discard()))
    {
case KMessageBox::PrimaryAction:			// "Save"
        slotSaveProject();
        return (!isModified());				// check that save worked

case KMessageBox::SecondaryAction:			// "Discard"
        return true;

default:						// "Cancel"
        return false;
    }

}

//////////////////////////////////////////////////////////////////////////
//									//
//  Window properties							//
//									//
//////////////////////////////////////////////////////////////////////////

// TODO: only when last window closed
// or to unique window/file ID
void MainWindow::saveProperties(KConfigGroup &grp)
{
    qDebug() << "to" << grp.name();
    KMainWindow::saveProperties(grp);

    mapController()->saveProperties();
    filesController()->saveProperties();

    Settings::setMainWindowSplitterState(mTreeModeSplitter->saveState().toBase64());
    Settings::setMainWindowViewMode(mWidgetStack->currentIndex());

    Settings::self()->save();
}


void MainWindow::readProperties(const KConfigGroup &grp)
{
    qDebug() << "from" << grp.name();
    KMainWindow::readProperties(grp);

    mapController()->readProperties();
    filesController()->readProperties();

    QString splitterState = Settings::mainWindowSplitterState();
    if (!splitterState.isEmpty()) mTreeModeSplitter->restoreState(QByteArray::fromBase64(splitterState.toLocal8Bit()));

    int viewMode = Settings::mainWindowViewMode();
    if (viewMode==-1) viewMode = 0;			// apply the default
    setViewMode(static_cast<MainWindow::ViewMode>(viewMode));
}

//////////////////////////////////////////////////////////////////////////
//									//
//  Saving and export							//
//									//
//  Error reporting and status messages are done in			//
//  FilesController::exportFile()					//
//									//
//////////////////////////////////////////////////////////////////////////

//  "Save"			"Save As"	"Save Copy As"		"Export"
//    |				   |		      |			   |
//  slotSaveProject() --------> slotSaveAs()	slotSaveCopy()		slotExportFile()
//    |                             |		      |			   |
//  save() <------------------------+ <---------------+                    |
//    |                                                                    |
//    + <------------------------------------------------------------------+
//    |
//  FilesController::exportFile()

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

    // metadata for view mode
    tdf->setMetadata("viewmode", mViewModeAction->isChecked() ? "list" : "tree");

    return (filesController()->exportFile(to, tdf, options)==FilesController::StatusOk);
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
                                            QStringList());				// supportedSchemes

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
                                            QStringList());				// supportedSchemes

    if (!file.isValid()) return;			// didn't get a file name
    saver.save(file);

    qDebug() << "to" << file;
    save(file, ImporterExporterBase::NoOption);
}


void MainWindow::slotExportFile()
{
// TODO: option to export selected items
    ExportFileDialogue d(FilesController::allExportFilters(), this);
    FilesModel *mod = filesController()->model();
    d.setSourceModel(mod);

    if (!d.exec()) return;

    ImporterExporterBase::Options opts = ImporterExporterBase::ImportExport;
//    filesController()->exportFile(file, opts, d.homePoint(), d.workPoint());
}

//////////////////////////////////////////////////////////////////////////
//									//
//  Loading and import							//
//									//
//  Error reporting and status messages are done in			//
//  FilesController::importFile()					//
//									//
//////////////////////////////////////////////////////////////////////////

// "Open"			"Import"
//   |				   |
// slotOpenProject()		slotImportFile()
//   |                             |
// loadProject()                   |
//   |                             |
// load()                          |
//   |				   |
//   + <---------------------------+
//   |
// FilesController::importFile()

FilesController::Status MainWindow::load(const QUrl &from)
{
    qDebug() << "from" << from;
    if (!from.isValid()) return (FilesController::StatusFailed);

    FilesController::Status status = filesController()->importFile(from, ImporterExporterBase::NoOption);
    if (status!=FilesController::StatusOk && status!=FilesController::StatusResave) return (status);

    TrackDataFile *tdf = filesController()->model()->rootFileItem();
    if (tdf!=nullptr)
    {
        QVariant s = tdf->metadata("position");
        qDebug() << "pos metadata" << s;
        QSignalBlocker block(mapController()->view());	// no status bar update from zooming
        if (!s.isNull()) mapController()->view()->setCurrentPosition(s.toString());
        else mapController()->gotoSelection(QList<TrackDataItem *>() << tdf);

        s = tdf->metadata("viewmode");
        qDebug() << "view mode metadata" << s;
        if (!s.isNull()) setViewMode(s.toString()=="list" ? MainWindow::ViewTabs : MainWindow::ViewTree);
    }

    filesController()->filesView()->expandToDepth(1);	// expand to show segments
    if (Settings::fileCheckTimezone())			// check time zone is set
    {
        QTimer::singleShot(0, filesController(), &FilesController::slotCheckTimeZone);
    }
    return (status);
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
                                            QStringList());				// supportedSchemes

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


void MainWindow::slotImportFile()
{
#if 1
    // TODO: maybe only use the dialogue if in points list mode?
    ImportFileDialogue d(FilesController::allImportFilters(), this);

    ImporterExporterBase::Options opts = ImporterExporterBase::IgnoreHome;
    if (isPointsListMode()) opts |= ImporterExporterBase::MergeWaypoints;
    const FilesModel *mod = filesController()->model();
    if (mod->isEmpty() || mod->rootFileItem()->childCount()==0) opts |= ImporterExporterBase::MergeNotAllowed;
    d.setOptions(opts);					// default options for dialogue

    if (!d.exec()) return;
    opts = d.options();					// actual options from dialogue
    opts |= ImporterExporterBase::ImportExport;		// add options for import operation
    if (isPointsListMode()) opts |= ImporterExporterBase::MarkNewWaypoints;

    // Importing with merged waypoints cannot be undone and may cause data
    // loss if the file is modified but not saved.  Warn the user and give
    // them a change to cancel the operation.
    if (isModified() && (opts & ImporterExporterBase::MergeWaypoints))
    {
        if (KMessageBox::warningContinueCancel(this,
                                               xi18nc("@info", "File <emphasis strong=\"1\"><filename>%1</filename></emphasis> has been modified but not saved.<nl/>The import operation with merged waypoints cannot be undone.<nl/><nl/>Continue with the import?", documentName()),
                                               i18n("Confirm Import"),
                                               KGuiItem(i18n("Import"), mImportAction->icon()))!=KMessageBox::Continue) return;
    }
							// do the import or merge
    if (filesController()->importFile(d.selectedUrl(), opts)!=FilesController::StatusOk) return;

    if (opts & ImporterExporterBase::MergeWaypoints)	// did import with merge,
    {							// cannot undo after that
        qDebug() << "clearing undo stack after import with merge";
        mUndoStack->clear();
    }
#else
    RecentSaver saver("import");
    QUrl file = QFileDialog::getOpenFileUrl(this,					// parent
                                            i18n("Import File"),			// caption
                                            saver.recentUrl(),				// dir
                                            FilesController::allImportFilters(),	// filter
                                            nullptr,					// selectedFilter,
                                            QFileDialog::Options(),			// options
                                            QStringList());				// supportedSchemes

    if (!file.isValid()) return;			// didn't get a file name
    saver.save(file);
#endif
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

//////////////////////////////////////////////////////////////////////////
//									//
//  Action states							//
//									//
//////////////////////////////////////////////////////////////////////////

void MainWindow::slotUpdateActionState()
{
    int selCount = filesController()->filesView()->selectedCount();
    TrackData::Type selType = filesController()->filesView()->selectedType();
    qDebug() << "selected" << selCount << "type" << selType;

    bool copyEnabled = false;
    bool propsEnabled = false;
    bool profileEnabled = false;
    bool stopsEnabled = false;
    QString propsText = i18nc("@action:inmenu", "Item Properties...");
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
    // This may be NULL; check that there is a selection before using it.
    const TrackDataItem *selectedItem = filesController()->filesView()->selectedItem();

    switch (selType)
    {
case TrackData::File:
        propsText = i18ncp("@action:inmenu", "File Properties...", "Files Properties...", selCount);
        propsEnabled = true;
        delEnabled = false;
        // Enabling these actions assumes that a track or route container
        // with valid points is present within the file.  That assumption
        // may not be true, but calling FilesView::selectedPoints() here
        // to really verify whether points are selected is is a bit too
        // expensive.  These operations must either work with no selected
        // points, or tell the user if there are no points to work on.
        stopsEnabled = profileEnabled = true;
        break;

case TrackData::Track:
        propsText = i18ncp("@action:inmenu", "Track Properties...", "Tracks Properties...", selCount);
        propsEnabled = true;
        delText = i18ncp("@action:inmenu", "Delete Track", "Delete Tracks", selCount);
        selectedContainer = selectedItem;
        stopsEnabled = profileEnabled = true;
        break;

case TrackData::Route:
        propsText = i18ncp("@action:inmenu", "Route Properties...", "Routes Properties...", selCount);
        propsEnabled = true;
        delText = i18ncp("@action:inmenu", "Delete Route", "Delete Routes", selCount);
        selectedContainer = selectedItem;
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

case TrackData::Trackpoint:
        propsText = i18ncp("@action:inmenu", "Point Properties...", "Points Properties...", selCount);
        propsEnabled = true;
        delText = i18ncp("@action:inmenu", "Delete Point", "Delete Points", selCount);
        selectedContainer = selectedItem->parent();
        stopsEnabled = profileEnabled = (selCount>1);
        copyEnabled = true;
        splitEnabled = (selCount==1);
        splitText = i18nc("@action:inmenu", "Split Segment");
        break;

case TrackData::Routepoint:
        propsText = i18ncp("@action:inmenu", "Route Point Properties...", "Route Points Properties...", selCount);
        propsEnabled = true;
        delText = i18ncp("@action:inmenu", "Delete Route Point", "Delete Route Points", selCount);
        selectedContainer = selectedItem->parent();
        profileEnabled = (selCount>1);
        copyEnabled = true;
        splitEnabled = (selCount==1);
        splitText = i18nc("@action:inmenu", "Split Route");
        break;

case TrackData::Folder:
        propsText = i18ncp("@action:inmenu", "Folder Properties...", "Folders Properties...", selCount);
        propsEnabled = true;
        delText = i18ncp("@action:inmenu", "Delete Folder", "Delete Folders", selCount);
        selectedContainer = selectedItem;
        moveEnabled = true;
        moveText = i18nc("@action:inmenu", "Move Folder...");
        break;

case TrackData::Waypoint:
        propsText = i18ncp("@action:inmenu", "Waypoint Properties...", "Waypoints Properties...", selCount);
        propsEnabled = true;
        delText = i18ncp("@action:inmenu", "Delete Waypoint", "Delete Waypoints", selCount);
        moveEnabled = true;
        moveText = i18ncp("@action:inmenu", "Move Waypoint...", "Move Waypoints...", selCount);
        selectedContainer = selectedItem->parent();
        statusEnabled = true;
        copyEnabled = true;
        mergeEnabled = (selCount>1);
        mergeText = i18nc("@action:inmenu", "Merge Waypoints...");

        if (selCount==1)
        {
            const TrackDataWaypoint *tdw = dynamic_cast<const TrackDataWaypoint *>(selectedItem);
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

    // If there is a selected container or point(s), then move points mode
    // is allowed to be entered;  otherwise, it is disabled.
    if (selectedContainer!=nullptr)
    {
        // If there is a selected container and it is the same as the currently
        // selected container, then move points mode can stay at the same state
        // as it currently is.  Otherwise, it is forced off.
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

    // Record the currently selected container, for checking as above
    // the next time we are called.
    mSelectedContainer = selectedContainer;

    // Update the waypoint status actions.  Their parent action will
    // be disabled below if read-only.
    const QList<QAction *> acts = mWaypointStatusAction->actions();
    for (QAction *act : acts) act->setChecked(statusValue==act->data().toInt());

    // This is allowed (but no modifications can be made)
    // even in read-only mode.
    mPropertiesAction->setEnabled(propsEnabled);
    mPropertiesAction->setText(propsText);

    // Viewing actions allowed in read-only mode
    mPlayMediaAction->setEnabled(playEnabled);
    mPlayMediaAction->setText(playText);
    mOpenMediaAction->setEnabled(playEnabled);
    mSaveMediaAction->setEnabled(playEnabled);

    mSelectAllAction->setEnabled(selCount>0 && selType!=TrackData::Mixed);
    mClearSelectAction->setEnabled(selCount>0);
    mMapGoToAction->setEnabled(selCount>0 && selType!=TrackData::Mixed);
    mCopyAction->setEnabled(copyEnabled);

    // No modifying actions are allowed in read-only mode, disable them
    // and then there is no more to do.
    if (isReadOnly())
    {
        mDeleteItemsAction->setEnabled(false);
        mSplitTrackAction->setEnabled(false);
        mMergeTrackAction->setEnabled(false);
        mMoveItemAction->setEnabled(false);
        mAddTrackAction->setEnabled(false);
        mAddRouteAction->setEnabled(false);
        mAddFolderAction->setEnabled(false);
        mAddWaypointAction->setEnabled(false);
        mAddRoutepointAction->setEnabled(false);
        mWaypointStatusAction->setEnabled(false);
        mMapDragAction->setEnabled(false);
        return;
    }

    mDeleteItemsAction->setEnabled(delEnabled);
    mDeleteItemsAction->setText(delText);
    mProfileAction->setEnabled(profileEnabled);
    mStatisticsAction->setEnabled(profileEnabled);
    mStopDetectAction->setEnabled(stopsEnabled);

    mSplitTrackAction->setEnabled(splitEnabled);
    if (splitEnabled) mSplitTrackAction->setText(splitText);
    mMergeTrackAction->setEnabled(mergeEnabled);
    if (mergeEnabled) mMergeTrackAction->setText(mergeText);

    mMoveItemAction->setEnabled(moveEnabled);
    mMoveItemAction->setText(moveText);

    mAddTrackAction->setEnabled(selCount==1 && selType==TrackData::File);
    mAddRouteAction->setEnabled(selCount==1 && selType==TrackData::File);
    mAddFolderAction->setEnabled(selCount==1 && (selType==TrackData::File || selType==TrackData::Folder));

    if (isPointsListMode())				// in points list view mode?
    {
        mAddWaypointAction->setEnabled(true);		// always allowed in this mode
        mAddRoutepointAction->setEnabled(false);	// never allowed in this mode
    }
    else						// tree view mode
    {
        // This will always be possible if an appropriate container is
        // selected.  If a point or waypoint is selected (to create at
        // that position), then it may not be possible to actually create
        // the point if no container exists to contain it.
        mAddWaypointAction->setEnabled(selCount==1 && (selType==TrackData::Folder ||
                                                       selType==TrackData::Trackpoint ||
                                                       selType==TrackData::Waypoint));
        mAddRoutepointAction->setEnabled(selCount==1 && (selType==TrackData::Route ||
                                                         selType==TrackData::Trackpoint ||
                                                         selType==TrackData::Waypoint));
    }

    mWaypointStatusAction->setEnabled(statusEnabled);

    if (selCount==1 && selType==TrackData::Trackpoint)
    {							// not first point in segment
        const QModelIndex idx = filesController()->model()->indexForItem(selectedItem);
        mAddTrackpointAction->setEnabled(idx.row()>0);
    }
    else mAddTrackpointAction->setEnabled(false);

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

//////////////////////////////////////////////////////////////////////////
//									//
//  Modification and undo						//
//									//
//////////////////////////////////////////////////////////////////////////

void MainWindow::slotSetModified(bool mod)
{
    setModified(mod);
    mSaveProjectAction->setEnabled(mod && hasFileName());
    mModifiedIndicator->setEnabled(mod);
    setWindowTitle(documentName()+" [*]");
    setWindowModified(mod);

    const bool hasContent = !filesController()->model()->isEmpty();
    mSaveProjectAsAction->setEnabled(hasContent);
    mExportAction->setEnabled(hasContent);
    mSaveProjectCopyAction->setEnabled(hasContent);
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


void MainWindow::slotExecuteCommand(QUndoCommand *cmd)
{
    if (mUndoStack!=nullptr) mUndoStack->push(cmd);	// do via undo system
    else { cmd->redo(); delete cmd; }			// do directly (fallback)
}

//////////////////////////////////////////////////////////////////////////
//									//
//  Miscellaneous slots and actions					//
//									//
//////////////////////////////////////////////////////////////////////////

void MainWindow::slotMapZoomChanged(bool canZoomIn, bool canZoomOut)
{
    mMapZoomInAction->setEnabled(canZoomIn);
    mMapZoomOutAction->setEnabled(canZoomOut);
}


void MainWindow::slotMapGotoSelection()
{
    mapController()->gotoSelection(filesController()->filesView()->selectedItems());

    // If the points view is active, ensure that the map tab is shown.
    if (mViewModeAction->isChecked()) mMainTabs->setCurrentIndex(1);
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
    filesController()->filesView()->setMovePointsMode(on);
}


void MainWindow::slotTrackProfile()
{
#ifdef HAVE_QCUSTOMPLOT
    ProfileWidget *w = new ProfileWidget(this);
    w->setAttribute(Qt::WA_DeleteOnClose);
    w->setModal(false);
    w->show();
#else // HAVE_QCUSTOMPLOT
    KMessageBox::error(this, i18n("The application is not built with QCustomPlot, profile is not available."));
#endif // HAVE_QCUSTOMPLOT
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
    const TrackDataWaypoint *tdw = dynamic_cast<const TrackDataWaypoint *>(filesController()->filesView()->selectedItem());
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
    const TrackDataWaypoint *tdw = dynamic_cast<const TrackDataWaypoint *>(filesController()->filesView()->selectedItem());
    Q_ASSERT(tdw!=nullptr);
    if (tdw->isMediaType()) MediaPlayer::openMediaFile(tdw);
}


void MainWindow::slotSaveMedia()
{
    const TrackDataWaypoint *tdw = dynamic_cast<const TrackDataWaypoint *>(filesController()->filesView()->selectedItem());
    Q_ASSERT(tdw!=nullptr);
    if (tdw->isMediaType()) MediaPlayer::saveMediaFile(tdw);
}


void MainWindow::slotTrackStopDetect()
{
    StopDetectDialogue *d = new StopDetectDialogue(this);
    d->show();
}


void MainWindow::slotResetAndCancel()
{
    mapController()->view()->cancelDrag();
    filesController()->filesView()->clearSelection();
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
    mapController()->openExternalMap(map, filesController()->filesView()->selectedItems());
}

//////////////////////////////////////////////////////////////////////////
//									//
//  Drag and drop							//
//									//
//////////////////////////////////////////////////////////////////////////

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
    const QList<QUrl> urls = mimeData->urls();

    QList<QByteArray> imageTypes = QImageReader::supportedMimeTypes();

    QMimeDatabase db;
    QList<QUrl> validUrls;
    for (const QUrl &url : urls)
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
        KMessageBox::error(this, i18n("Don't know what to do with any of the pasted or dropped URLs"));
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

//////////////////////////////////////////////////////////////////////////
//									//
//  Copy/paste								//
//									//
//////////////////////////////////////////////////////////////////////////

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

//////////////////////////////////////////////////////////////////////////
//									//
//  Display mode							//
//									//
//////////////////////////////////////////////////////////////////////////

void MainWindow::slotViewPointsMode()
{
    const bool on = mViewModeAction->isChecked();
    setViewMode(on ? MainWindow::ViewTabs : MainWindow::ViewTree);
}


void MainWindow::setViewMode(MainWindow::ViewMode mode)
{
    qDebug() << mode;

    QWidget *oldWidget = mMapTreePlaceholder->widget(0);
    if (oldWidget!=nullptr)
    {
        mMapTreePlaceholder->removeWidget(oldWidget);
        oldWidget->setParent(nullptr);			// QStackedWidget retains ownership
    }
    Q_ASSERT(mMapTreePlaceholder->count()==0);		// placeholder should now be empty

    oldWidget = mMapTabPlaceholder->widget(0);
    if (oldWidget!=nullptr)
    {
        mMapTabPlaceholder->removeWidget(oldWidget);
        oldWidget->setParent(nullptr);
    }
    Q_ASSERT(mMapTabPlaceholder->count()==0);

    if (mode==MainWindow::ViewTree)
    {
        mMapTreePlaceholder->addWidget(mapController()->view());
        mWidgetStack->setCurrentIndex(0);
        mViewModeAction->setChecked(false);
    }
    else
    {
        mMapTabPlaceholder->addWidget(mapController()->view());
        mWidgetStack->setCurrentIndex(1);
        mViewModeAction->setChecked(true);
    }

    slotUpdateActionState();				// action states may be mode-dependent
}


bool MainWindow::isPointsListMode() const
{
    return (mViewModeAction->isChecked());
}
