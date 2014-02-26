
#include "mainwindow.h"

#include <string.h>
#include <errno.h>

#include <qapplication.h>
#include <qgraphicsview.h>
#include <qlabel.h>
#include <qsplitter.h>
#include <qundostack.h>
#include <qdatetime.h>

#include <kdebug.h>
#include <klocale.h>

#include <kapplication.h>
#include <kaboutdata.h>
#include <kaction.h>
#include <kactioncollection.h>
#include <kstandardaction.h>
#include <kstatusbar.h>
#include <kfiledialog.h>
#include <kmessagebox.h>
#include <ksqueezedtextlabel.h>
#include <kmimetype.h>
#include <kurl.h>
#include <kactionmenu.h>

#include <marble/AbstractFloatItem.h>


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
    mSplitter = new QSplitter(this);
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
    mMapController->view()->setFilesModel(mFilesController->model());
    mMapController->view()->setFilesView(mFilesController->view());
    connect(mMapController, SIGNAL(statusMessage(const QString &)), SLOT(slotStatusMessage(const QString &)));
    connect(mMapController, SIGNAL(modified()), SLOT(slotSetModified()));
    connect(mMapController, SIGNAL(mapZoomChanged(bool,bool)), SLOT(slotMapZoomChanged(bool,bool)));

    connect(mFilesController, SIGNAL(updateMap()), mMapController->view(), SLOT(update()));
    // TODO: temp, see FilesView::selectionChanged()
    connect(mFilesController, SIGNAL(updateActionState()), mMapController->view(), SLOT(update()));

    mSplitter->addWidget(mFilesController->view());
    mSplitter->addWidget(mMapController->view());

    setupStatusBar();
    setupActions();

    readProperties(Settings::self()->config()->group(CONFIG_GROUP));

    slotSetModified(false);
    slotUpdateActionState();
}



MainWindow::~MainWindow()
{
    delete mProject;

    kDebug() << "done";
}



void MainWindow::setupActions()
{
//    KStandardAction::quit(this, SLOT(close()), actionCollection());
    KStandardAction::close(this, SLOT(close()), actionCollection());

    KAction *a = KStandardAction::openNew(this, SLOT(slotNewProject()), actionCollection());
    a = KStandardAction::open(this, SLOT(slotOpenProject()), actionCollection());
    mSaveProjectAction = KStandardAction::save(this, SLOT(slotSaveProject()), actionCollection());
    mSaveProjectAsAction = KStandardAction::saveAs(this, SLOT(slotSaveAs()), actionCollection());

    mImportAction = actionCollection()->addAction("file_import");
    mImportAction->setText(i18n("Import..."));
    mImportAction->setIcon(KIcon("document-import"));
    mImportAction->setShortcut(Qt::CTRL+Qt::Key_I);
    connect(mImportAction, SIGNAL(triggered()), SLOT(slotImportFile()));

    mExportAction = actionCollection()->addAction("file_export");
    mExportAction->setText(i18n("Export..."));
    mExportAction->setIcon(KIcon("document-export"));
    mExportAction->setShortcut(Qt::CTRL+Qt::Key_E);
    connect(mExportAction, SIGNAL(triggered()), SLOT(slotExportFile()));
    mExportAction->setEnabled(false);

    mSelectAllAction = KStandardAction::selectAll(filesController()->view(), SLOT(slotSelectAllSiblings()), actionCollection());
    mClearSelectAction = KStandardAction::deselect(filesController()->view(), SLOT(clearSelection()), actionCollection());
    mClearSelectAction->setIcon(KIcon("edit-clear-list"));

    mUndoAction = KStandardAction::undo(mUndoStack, SLOT(undo()), actionCollection());
    mUndoAction->setEnabled(false);
    mUndoText = mUndoAction->text();

    mRedoAction = KStandardAction::redo(mUndoStack, SLOT(redo()), actionCollection());
    mRedoAction->setEnabled(false);
    mRedoText = mRedoAction->text();

    a = actionCollection()->addAction("track_expand_all");
    a->setText(i18n("Expand All"));
    a->setIcon(KIcon("application_side_tree"));
    a->setShortcut(Qt::CTRL+Qt::Key_Period);
    connect(a, SIGNAL(triggered()), filesController()->view(), SLOT(expandAll()));

    a = actionCollection()->addAction("track_collapse_all");
    a->setText(i18n("Collapse All"));
    a->setIcon(KIcon("application_side_list"));
    a->setShortcut(Qt::CTRL+Qt::Key_Comma);
    connect(a, SIGNAL(triggered()), filesController()->view(), SLOT(collapseAll()));

    mPropertiesAction = actionCollection()->addAction("track_properties");
    // text set in slotUpdateActionState() below
    mPropertiesAction->setShortcut(KShortcut(Qt::CTRL+Qt::Key_Return, Qt::CTRL+Qt::Key_Enter));
    mPropertiesAction->setIcon(KIcon("document-properties"));
    connect(mPropertiesAction, SIGNAL(triggered()), filesController(), SLOT(slotTrackProperties()));

    a = actionCollection()->addAction("map_save");
    a->setText(i18n("Save As Image..."));
    a->setIcon(KIcon("document-save"));
    connect(a, SIGNAL(triggered()), mapController(), SLOT(slotSaveImage()));

    a = actionCollection()->addAction("map_set_home");
    a->setText(i18n("Set Home Position"));
    a->setIconText(i18n("Set Home"));
    a->setIcon(KIcon("bookmarks"));
    connect(a, SIGNAL(triggered()), mapController(), SLOT(slotSetHome()));

    a = actionCollection()->addAction("map_go_home");
    a->setText(i18n("Go to Home Position"));
    a->setIconText(i18n("Go Home"));
    a->setIcon(KIcon("go-home"));
    a->setShortcut(Qt::CTRL+Qt::Key_Home);
    connect(a, SIGNAL(triggered()), mapController(), SLOT(slotGoHome()));

    mMapZoomInAction = actionCollection()->addAction(KStandardAction::ZoomIn, "map_zoom_in");
    connect(mMapZoomInAction, SIGNAL(triggered()), mapController()->view(), SLOT(zoomIn()));

    mMapZoomOutAction = actionCollection()->addAction(KStandardAction::ZoomOut, "map_zoom_out");
    connect(mMapZoomOutAction, SIGNAL(triggered()), mapController()->view(), SLOT(zoomOut()));

    a = actionCollection()->addAction("map_set_zoom");
    a->setText(i18n("Set Standard Zoom"));
    a->setIconText(i18n("Set Zoom"));
    a->setIcon(KIcon("bookmarks"));
    connect(a, SIGNAL(triggered()), mapController(), SLOT(slotSetZoom()));

    a = actionCollection()->addAction("map_zoom_standard");
    a->setText(i18n("Reset to Standard Zoom"));
    a->setIconText(i18n("Reset Zoom"));
    a->setIcon(KIcon("zoom-original"));
    a->setShortcut(Qt::CTRL+Qt::Key_1);
    connect(a, SIGNAL(triggered()), mapController(), SLOT(slotResetZoom()));

    mMapGoToAction = actionCollection()->addAction("map_go_selection");
    mMapGoToAction->setText(i18n("Show on Map"));
    mMapGoToAction->setIcon(KIcon("marble"));
    mMapGoToAction->setShortcut(Qt::CTRL+Qt::Key_G);
    connect(mMapGoToAction, SIGNAL(triggered()), this, SLOT(slotMapGotoSelection()));

    a = actionCollection()->addAction("map_select_theme");
    a->setText(i18n("Select Theme..."));
    a->setIcon(KIcon("image-loading"));
    connect(a, SIGNAL(triggered()), mapController(), SLOT(slotSelectTheme()));

    a = actionCollection()->addAction("map_find_address");
    a->setText(i18n("Find Address..."));
    a->setIcon(KIcon("view-pim-mail"));
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

    a = KStandardAction::preferences(this, SLOT(slotPreferences()), actionCollection());

    a = actionCollection()->addAction("help_about_marble");
    a->setText(i18n("About Marble"));
    a->setIcon(KIcon("marble"));
    connect(a, SIGNAL(triggered()), mapController(), SLOT(slotAboutMarble()));

    setupGUI(KXmlGuiWindow::Default);
    setAutoSaveSettings();
}


void MainWindow::setupStatusBar()
{
    KStatusBar *sb = statusBar();

//    sb->insertPermanentFixedItem(" 1000% ",sbZoom);
//    sb->insertPermanentFixedItem(i18n(" X000,Y000 "),sbLocation);

    mModifiedIndicator = new QLabel(this);
    mModifiedIndicator->setPixmap(KIcon("document-save").pixmap(16));
    mModifiedIndicator->setFixedWidth(20);
    sb->insertPermanentWidget(sbModified, mModifiedIndicator);

    mStatusMessage = new KSqueezedTextLabel(i18n("Initialising..."), sb);
    sb->addWidget(mStatusMessage, 1);
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
    tdf->setMetadata(DataIndexer::self()->index("creator"), KGlobal::mainComponent().aboutData()->appName());
    tdf->setMetadata(DataIndexer::self()->index("time"), QDateTime::currentDateTimeUtc().toString(Qt::ISODate));

    return (filesController()->exportFile(savePath, tdf));
}



// Error reporting and status messages are done in FilesController::importFile()
bool MainWindow::load(const KUrl &from)
{
    kDebug() << "from" << from;

    // TODO: allow non-local files
    if (!from.isValid() || !from.hasPath()) return (false);
    QDir d(from.path());				// should be absolute already,
    QString loadPath = d.absolutePath();		// but just make sure

    if (!filesController()->importFile(loadPath)) return (false);

    TrackDataFile *tdf = filesController()->model()->rootFileItem();
    if (tdf!=NULL)
    {
        QString s = tdf->metadata(DataIndexer::self()->index("position"));
        kDebug() << "pos metadata" << s;
        if (!s.isEmpty())
        {						// no status bar update from zooming
            mapController()->view()->blockSignals(true);
            mapController()->view()->setCurrentPosition(s);
            mapController()->view()->blockSignals(false);
        }
    }

    filesController()->view()->expandToDepth(1);	// expand to show segments
    return (true);
}



void MainWindow::slotStatusMessage(const QString &text)
{
    mStatusMessage->setText(text);
}



void MainWindow::clear()
{
    mUndoStack->clear();				// clear undo information
    mapController()->clear();				// clear old map
    filesController()->clear();				// clear old data
    mProject->clear();					// clear project data
}



void MainWindow::slotNewProject()
{
    MainWindow *w = new MainWindow(NULL);
    w->show();
}



void MainWindow::slotOpenProject()
{
    if (!queryClose()) return;

    KFileDialog d(QString("kfiledialog:///project/"), FilesController::allProjectFilters(true), this);
    d.setCaption(i18n("Open Tracks File"));
    d.setOperationMode(KFileDialog::Opening);
    d.setKeepLocation(true);
    d.setMode(KFile::File|KFile::LocalOnly);

    if (!d.exec()) return;

    if (filesController()->model()->isEmpty()) loadProject(d.selectedUrl());
    else
    {
        MainWindow *w = new MainWindow(NULL);
        w->loadProject(d.selectedUrl());
        w->show();
    }
}


void MainWindow::loadProject(const KUrl &loadFrom)
{
    if (!loadFrom.isValid()) return;
    kDebug() << "from" << loadFrom;

    clear();						// clear storage
    if (load(loadFrom))					// load in data file
    {
        mProject->setFileName(loadFrom);
        mUndoStack->clear();				// clear undo history
        slotSetModified(false);				// ensure window title updated
    }
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
    KFileDialog d(QString("kfiledialog:///project/%1").arg("untitled"), FilesController::allProjectFilters(false), this);
    d.setCaption(i18n("Save Tracks File As"));
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
    KFileDialog d(KUrl("kfiledialog:///import"), FilesController::allImportFilters(), this);
    d.setCaption(i18n("Import File"));
    d.setOperationMode(KFileDialog::Opening);
    d.setKeepLocation(true);
    d.setMode(KFile::File|KFile::LocalOnly);

    if (!d.exec()) return;
    filesController()->importFile(d.selectedUrl());
}


void MainWindow::slotExportFile()
{
    KFileDialog d(KUrl("kfiledialog:///export/"+mProject->name(true)), FilesController::allExportFilters(), this);
    d.setCaption(i18n("Export File"));
    d.setOperationMode(KFileDialog::Saving);
    d.setConfirmOverwrite(true);
    d.setKeepLocation(true);
    d.setMode(KFile::File|KFile::LocalOnly);

    if (!d.exec()) return;
//////// TODO: export selected item
//    filesController()->exportFile(d.selectedUrl());
}


//void MainWindow::slotExport()
//{
//    QStringList filters;
//    filters << HtmlExporter::filter();
//    filters << ImageExporter::filter();
//
//    KFileDialog d(QString("kfiledialog:///export/%1").arg("map.html"),filters.join("\n"),this);
//    d.setCaption(i18n("Export"));
//    d.setOperationMode(KFileDialog::Saving);
//    d.setKeepLocation(true);
//    d.setMode(KFile::File|KFile::LocalOnly);
//    d.setConfirmOverwrite(true);
//
//    if (!d.exec()) return;
//    QString saveTo = d.selectedFile();			// can only export to local files
//    if (saveTo.isEmpty()) return;
//    QString saveType = d.currentFilter().split(' ').at(0);
//
//
//
//
//    kDebug() << "to" << saveTo << "filter" << saveType;
//
//    QString wantExt = saveType.mid(2);			// extension to save with
//    QString haveExt = KMimeType::extractKnownExtension(saveTo);
//    if (wantExt!=haveExt) saveTo += "."+wantExt;	// force add if not there
//
//    ExporterBase *exp = NULL;				// exporter for requested format
//
//    if (wantExt=="html")				// save map as HTML page
//    {
//        exp = mMap->createHtmlExporter();
//        exp->setTitle(mProject->title());
//    }
//    else if (wantExt=="png")				// save map as image
//    {
//        exp = mMap->createImageExporter();
//    }
//    else
//    {
//        kDebug() << "Unknown save format" << wantExt;
//        return;
//    }
//
//    if (!exp->save(saveTo))
//    {
//        KMessageBox::sorry(this,i18n("<qt>Cannot save %1 to<br><filename>%3</filename><br><br>%2",
//                                     wantExt.toUpper(),exp->lastError(),saveTo));
//    }
//
//    delete exp;						// finished with exporter
//}



void MainWindow::slotSetModified(bool mod)
{
    mProject->setModified(mod);
    mSaveProjectAction->setEnabled(mod && mProject->hasFileName());
    mModifiedIndicator->setEnabled(mod);
    setCaption(mProject->name(), mod);

    mSaveProjectAsAction->setEnabled(!filesController()->model()->isEmpty());
}




void MainWindow::slotUpdateActionState()
{
    int selCount = filesController()->view()->selectedCount();
    TrackData::Type selType = filesController()->view()->selectedType();
    kDebug() << "selected" << selCount << "type" << selType;

    bool propsEnabled = false;
    QString propsText = i18nc("@action:inmenu", "Properties...");
    switch (selType)
    {
case TrackData::File:
        propsText = i18ncp("@action:inmenu", "File Properties...", "Files Properties...", selCount);
        propsEnabled = true;
        break;

case TrackData::Track:
        propsText = i18ncp("@action:inmenu", "Track Properties...", "Tracks Properties...", selCount);
        propsEnabled = true;
        break;

case TrackData::Segment:
        propsText = i18ncp("@action:inmenu", "Segment Properties...", "Segments Properties...", selCount);
        propsEnabled = true;
        break;

case TrackData::Point:
        propsText = i18ncp("@action:inmenu", "Point Properties...", "Points Properties...", selCount);
        propsEnabled = true;
        break;

case TrackData::Mixed:
        propsText = i18nc("@action:inmenu", "Selection Properties...");
        break;

default:
        break;
    }
    mPropertiesAction->setEnabled(propsEnabled);
    mPropertiesAction->setText(propsText);
    // used in FilesController::slotTrackProperties()
    // the "..." is I18N'ed so that translations can change it to something that
    // will never match, if the target language does not use "..."
    QString dotdotdot = i18nc("as added to strings above", "...");
    if (propsText.endsWith(dotdotdot)) propsText.chop(dotdotdot.length());
    mPropertiesAction->setData(propsText);

    mSelectAllAction->setEnabled(selCount>0 && selType!=TrackData::Mixed);
    mClearSelectAction->setEnabled(selCount>0);
    mMapGoToAction->setEnabled(selCount>0 && selType!=TrackData::Mixed);
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



void MainWindow::executeCommand(QUndoCommand *cmd)
{
    if (mUndoStack!=NULL) mUndoStack->push(cmd);	// do via undo system
    else { cmd->redo(); delete cmd; }			// do directly (fallback)
}
