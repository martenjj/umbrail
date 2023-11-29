//////////////////////////////////////////////////////////////////////////
//									//
//  Project:	Umbrail - GPX track viewer and editor			//
//									//
//////////////////////////////////////////////////////////////////////////
//									//
//  Copyright (c) 2014-2023 Jonathan Marten <jjm@keelhaul.me.uk>	//
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

#include "filescontroller.h"

#include <qundostack.h>
#include <qaction.h>
#include <qscopedpointer.h>
#include <qpushbutton.h>
#include <qapplication.h>
#include <qlayout.h>
#include <qdebug.h>
#include <qurl.h>
#include <qmimetype.h>
#include <qmimedatabase.h>
#include <qtimer.h>
#include <qsortfilterproxymodel.h>
#ifdef HAVE_KEXIV2
#include <qtimezone.h>
#endif

#include <klocalizedstring.h>
#include <kmessagebox.h>
#include <kactioncollection.h>
#include <klinkitemselectionmodel.h>

#include <kio/statjob.h>
#include <kio/filecopyjob.h>

#ifdef HAVE_KEXIV2
#include <kexiv2/kexiv2.h>
using namespace KExiv2Iface;
#endif

#include "filesmodel.h"
#include "filesview.h"
#include "pointsdatamodel.h"
#include "fileslistmodel.h"
#include "waypointsfiltermodel.h"
#include "homepointsfiltermodel.h"
#include "homepointsdatamodel.h"
#include "pointsview.h"
#include "commands.h"
#include "gpximporter.h"
#include "gpxexporter.h"
#include "marksimporter.h"
#include "mainwindow.h"
#include "trackpropertiesdialogue.h"
#include "moveitemdialogue.h"
#include "createpointdialogue.h"
#include "errorreporter.h"
#include "mapview.h"
#include "mapcontroller.h"
#include "settings.h"
#include "metadatamodel.h"
#include "dataindexer.h"
#include "categoriesmanagedialogue.h"
#include "mergepointsdialogue.h"
#include "timezonesettingdialogue.h"


#define GROUP_FILES		"Files"
#define PHOTO_FOLDER_NAME	"Photos"


bool DialogueConstraintFilter::eventFilter(QObject *obj, QEvent *ev)
{
    if (ev->type()==QEvent::Show)
    {
        QWidget *w = qobject_cast<QWidget *>(obj);	// auto resize when "Details" toggled
        if (w!=nullptr) w->layout()->setSizeConstraint(QLayout::SetFixedSize);
    }

    return (false);					// always pass the event on
}


FilesController::FilesController(QObject *pnt)
    : QObject(pnt),
      ApplicationDataInterface(pnt)
{
    qDebug();

    // The master data tree model
    mFilesModel = new FilesModel(this);

    // A QTreeView to provide the main tree view onto that
    mFilesView = new FilesView(mainWidget());
    mFilesView->setModel(mFilesModel);
    connect(mFilesView, &FilesView::updateActionState, this, &FilesController::slotUpdateActionState);

    // A KDescendantsProxyModel to flatten the tree into a linear list of points
    FilesListModel *model1 = new FilesListModel(this);
    model1->setDisplayAncestorData(false);
    model1->setSourceModel(mFilesModel);

    // A QSortFilterProxyModel to filter only waypoints out of the list
    mWaypointsFilterModel = new WaypointsFilterModel(this);
    mWaypointsFilterModel->setDynamicSortFilter(true);
    mWaypointsFilterModel->setSourceModel(model1);

    // A KExtraColumnsProxyModel to generate display data for the waypoints
    mPointsDataModel = new PointsDataModel(this);
    mPointsDataModel->setSourceModel(mWaypointsFilterModel);

    // A QSortFilterProxyModel to sort the points data for display
    QSortFilterProxyModel *model3 = new QSortFilterProxyModel(this);
    model3->setSortCaseSensitivity(Qt::CaseInsensitive);
    model3->setSortRole(Qt::UserRole);
    model3->setDynamicSortFilter(true);
    model3->setSourceModel(mPointsDataModel);

    // A QTreeView to provide the points list view onto that
    mPointsView = new PointsView(mainWidget());
    mPointsView->setModel(model3);

    // A KExtraColumnsProxyModel to generate display data for
    // the home/work waypoints in the "Export File" dialogue.
    // This is created on demand by homePointsModel() below.
    mHomePointsModel = nullptr;

    // Data or selection updates from the main tree view
    connect(mFilesModel, &FilesModel::dataChanged, this, &FilesController::slotUpdateActionState);
    connect(mFilesModel, &FilesModel::dragDropItems, this, &FilesController::slotDragDropItems);

    // Synchronising the selection between the points list and the tree view,
    // which is all handled automatically by this model.  This means that the
    // points list view does not need to do anything concerned with selection.
    KLinkItemSelectionModel *linkModel = new KLinkItemSelectionModel(model3, mFilesView->selectionModel());
    mPointsView->setSelectionModel(linkModel);

    mWarnedNoTimezone = false;
}


FilesController::~FilesController()
{
    qDebug() << "done";
}


HomePointsDataModel *FilesController::homePointsModel()
{
    if (mHomePointsModel==nullptr)
    {
        HomePointsFilterModel *homeModel = new HomePointsFilterModel(this);
        homeModel->setDynamicSortFilter(true);
        homeModel->setSourceModel(mWaypointsFilterModel);
        homeModel->sort(0);				// sort by name

        mHomePointsModel = new HomePointsDataModel(this);
        mHomePointsModel->setSourceModel(homeModel);
    }

    return (mHomePointsModel);
}


void FilesController::readProperties()
{
    filesView()->readProperties();
    pointsView()->readProperties();
}


void FilesController::saveProperties()
{
    filesView()->saveProperties();
    pointsView()->saveProperties();
}


void FilesController::initNew()
{
    Q_ASSERT(filesModel()->rootFileItem()==nullptr);

    TrackDataFile *fileItem = new TrackDataFile;
    filesModel()->setRootFileItem(fileItem);
}


bool FilesController::fileWarningIgnored(const QUrl &file, const QByteArray &type)
{
    QByteArray askKey = QUrl::toPercentEncoding(file.url());
    KConfigGroup grp = KSharedConfig::openConfig()->group("FileWarnings");
    QStringList list = grp.readEntry(askKey.constData(), QStringList());

    // If the list contains the type value, then the warning is ignored.
    // The old single value "false" in the list is ignored.
    return (list.contains(QString::fromLocal8Bit(type)));
}


void FilesController::setFileWarningIgnored(const QUrl &file, const QByteArray &type)
{
    qDebug() << type << "for" << file;
    QByteArray askKey = QUrl::toPercentEncoding(file.url());
    KConfigGroup grp = KSharedConfig::openConfig()->group("FileWarnings");

    QStringList list = grp.readEntry(askKey.constData(), QStringList());
    QString typeString = QString::fromLocal8Bit(type);
    if (!list.contains(typeString))			// if not set already
    {
        list.append(typeString);
        grp.writeEntry(askKey.constData(), list);
    }
}


void FilesController::resetAllFileWarnings()
{
    qDebug();
    // Our application's file-specific warnings
    KConfigGroup grp = KSharedConfig::openConfig()->group("FileWarnings");
    grp.deleteGroup();
    // Group name from API documentation of KMessageBox
    grp = KSharedConfig::openConfig()->group("Notification Messages");
    grp.deleteGroup();
    grp.sync();
}


bool FilesController::reportFileError(bool saving, const QUrl &file, const QString &msg)
{
    ErrorReporter rep;
    rep.setError(ErrorReporter::Fatal, msg);
    return (reportFileError(saving, file, &rep));
}


bool FilesController::reportFileError(bool saving, const QUrl &file, const ErrorReporter *rep)
{
    bool detailed = (rep->messageCount()>1);
    QStringList list = rep->messageList();

    QString message;
    QString caption;
    QString iconName;
    QString askText;

    bool result = true;

    switch (rep->severity())
    {
case ErrorReporter::NoError:
        return (result);

case ErrorReporter::Warning:
        if (fileWarningIgnored(file, "warnings")) return (result);

        detailed = true;
        message = (saving ?
                   xi18nc("@info with placeholder at end", "The file <filename>%1</filename><nl/>was saved but with warnings.###", file.toDisplayString()) :
                   xi18nc("@info with placeholder at end", "The file <filename>%1</filename><nl/>was loaded but with warnings.###", file.toDisplayString()));
        caption = (saving ? i18n("File Save Warning") : i18n("File Load Warning"));
        iconName = "dialog-information";
        askText = i18n("Do not show again for this file");
        break;

case ErrorReporter::Error:
        message = (saving ?
                   xi18nc("@info with placeholder at end", "The file <filename>%1</filename><nl/>was saved but had errors.###", file.toDisplayString()) :
                   xi18nc("@info with placeholder at end", "The file <filename>%1</filename><nl/>was loaded but had errors.###", file.toDisplayString()));
        caption = (saving ? i18n("File Save Error") : i18n("File Load Error"));
        iconName = "dialog-warning";
        break;

case ErrorReporter::Fatal:
        message = (saving ?
                   xi18nc("@info with placeholder at end", "The file <filename>%1</filename><nl/>could not be saved.###", file.toDisplayString()) :
                   xi18nc("@info with placeholder at end", "The file <filename>%1</filename><nl/>could not be loaded.###", file.toDisplayString()));
        caption = (saving ? i18n("File Save Failure") : i18n("File Load Failure"));
        iconName = "dialog-error";
        result = false;
        break;
    }

    // The "###" marker in the formatted messages above is a placeholder for the
    // error message, the last in the list if not showing full details.  It cannot
    // simply be appended to the I18N string, because of the closing "</html>"
    // markup from KUIT.
    //
    // The diagnostics in 'list' are already I18N'ed and formatted either as
    // plain text or HTML (from KUIT markup).  So just insert the string into
    // the appropriate place in the message.
    QString details;
    if (!detailed)
    {
        details = "<br/><br/>"+list.last();		// last or only error message
        list.clear();					// don't show in list
    }
    message.replace("###", details);			// insert or blank details

    // Using the message box indirectly here for versatility.

    bool notAgain = false;

    QDialog *dlg = new QDialog(mainWidget());
    dlg->setWindowTitle(caption);
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok, dlg);
    if (detailed)
    {
        // From tier1/kwidgetsaddons/src/kmessagebox.cpp KMessageBox::detailedErrorInternal()
        // The "Details" button is recognised by its object name.
        QPushButton *detailsButton = new QPushButton;
        detailsButton->setObjectName(QStringLiteral("detailsButton"));
        detailsButton->setText(QApplication::translate("KMessageBox", "&Details")+QStringLiteral(" >>"));
        detailsButton->setIcon(QIcon::fromTheme(QStringLiteral("help-about")));
        buttonBox->addButton(detailsButton, QDialogButtonBox::HelpRole);
    }

    // Setting the sizeConstraint of the layout is required so that the dialogue
    // will expand and contract when the "Details" button is used.  However, there
    // is a problem with KMessageBox:  if the NoExec option is used so that the
    // layout constraint can be set before the exec(), then there is no way to
    // access the state of the check box when the dialogue is finished.  Therefore,
    // we use an event filter to set the constraint when the dialogue is shown.
    dlg->installEventFilter(new DialogueConstraintFilter(this));

    KMessageBox::createKMessageBox(dlg,					// dialog
                                   buttonBox,				// buttons
                                   QIcon::fromTheme(iconName),		// icon
                                   message,				// text
                                   QStringList(),			// strlist
                                   askText,				// ask
                                   &notAgain,				// checkboxReturn
                                   KMessageBox::AllowLink,  		// options
                                   QString("<qt>")+list.join("<br>"));	// details

    if (notAgain) setFileWarningIgnored(file, "warnings");
    return (result);
}


FilesController::Status FilesController::importFile(const QUrl &importFrom, const ImporterExporterOptions &options)
{
    if (!importFrom.isValid()) return (FilesController::StatusFailed);

    QMimeDatabase db;
    QString importType = db.suffixForFileName(importFrom.path());
    if (importType.isEmpty())
    {
        QString fileName = importFrom.fileName();
        int i = fileName.lastIndexOf('.');
        if (i>0) importType = fileName.mid(i+1);
    }

    importType = importType.toUpper();
    qDebug() << "from" << importFrom << "type" << importType;

    QScopedPointer<ImporterBase> imp;			// importer for requested format
    if (importType=="GPX")				// import from GPX file
    {
        imp.reset(new GpxImporter);
    }
    else if (importType=="MARKS")			// import from POI marks file
    {
        imp.reset(new MarksImporter);
    }

    if (imp.isNull())					// could not create importer
    {
        qDebug() << "Unknown import format" << importType;
        reportFileError(false, importFrom, i18n("Unknown import format"));
        return (FilesController::StatusFailed);
    }

    emit statusMessage(i18n("Loading %1 from <filename>%2</filename>...", importType, importFrom.toDisplayString()));
    imp->setOptions(options);				// set the import options
    TrackDataFile *tdf = imp->load(importFrom);		// do the import

    const ErrorReporter *rep = imp->reporter();
    if (!reportFileError(false, importFrom, rep))
    {
        emit statusMessage(xi18nc("@info", "Loading <filename>%1</filename> failed", importFrom.toDisplayString()));
        return (FilesController::StatusFailed);
    }

    Q_ASSERT(tdf!=nullptr);				// must have tree if no error

    // If the imported file has categories defined, then the CategoriesList
    // will have been set on its root file item 'tdf'.  If this is an import,
    // the command executed here will merge those categories with the existing
    // ones.
    ImportFileCommand *cmd = new ImportFileCommand(this);
    cmd->setText(i18n("Import"));
    cmd->setData(tdf);					// takes ownership of tree
    cmd->setOptions(options);				// user options for import

    if (filesModel()->isEmpty())			// loading a new file?
    {
        cmd->redo();					// yes, just do the import
        delete cmd;					// no need for this now
        emit statusMessage(xi18nc("@info", "Loaded <filename>%1</filename>", importFrom.toDisplayString()));
    }
    else if (options.hasFlag(ImporterExporterOptions::MergeWaypoints))
    {							// import with waypoint merge
        // Need to execute the command immediately
        // (synchronously), so that the MainWindow can
        // then clear its undo stack.
        cmd->redo();					// do the import and merge
        delete cmd;					// no need for this now
        emit statusMessage(xi18nc("@info", "Merged <filename>%1</filename>", importFrom.toDisplayString()));
    }
    else						// an import without merge
    {
        executeCommand(cmd);				// make the operation undo'able
        emit statusMessage(xi18nc("@info", "Imported <filename>%1</filename>", importFrom.toDisplayString()));
    }

    emit modified();					// done, finished with importer

    if (imp->needsResave() && !fileWarningIgnored(importFrom, "warnings")) return (FilesController::StatusResave);
    return (FilesController::StatusOk);
}


// The result may be:	 1 - file definitely exists
//			 0 - file does not exist
//			-1 - unable to determine
static int fileExists(const QUrl &file)
{
    int exists = -1;
    if (file.isLocalFile())				// to a local file?
    {							// see if exists already
        exists = (QFile::exists(file.toLocalFile())) ? 1 : 0;
    }
    else						// to a remote file
    {
        qDebug() << "stat remote" << file;

        KIO::StatJob *job = KIO::statDetails(file, KIO::StatJob::DestinationSide,
                                             KIO::StatBasic|KIO::StatResolveSymlink);
        bool ok = job->exec();
        qDebug() << "job result" << ok << "error" << job->error();

        if (ok) exists = 1;
        else if (job->error()==KIO::ERR_DOES_NOT_EXIST) exists = 0;
    }

    return (exists);
}


FilesController::Status FilesController::exportFile(const QUrl &exportTo, const ImporterExporterOptions &options)
{
    if (!exportTo.isValid()) return (FilesController::StatusFailed);

    QMimeDatabase db;
    QString exportType = db.suffixForFileName(exportTo.path());
    if (exportType.isEmpty())
    {
        QString fileName = exportTo.fileName();
        int i = fileName.lastIndexOf('.');
        if (i>0) exportType = fileName.mid(i+1);
    }

    exportType = exportType.toUpper();
    qDebug() << "to" << exportTo;
    qDebug() << "type" << exportType << "options" << options.flags();

    QScopedPointer<ExporterBase> exp;			// exporter for requested format
    if (exportType=="GPX")				// export to GPX file
    {
        exp.reset(new GpxExporter);
    }

    if (exp.isNull())					// could not create exporter
    {
        qDebug() << "Unknown export format" << exportType;
        reportFileError(true, exportTo, i18n("Unknown export format"));
        return (FilesController::StatusFailed);
    }

    if (exportTo.scheme()!="clipboard")			// no backup for copy/paste!
    {
        QUrl backupFile = exportTo;			// make path for backup file
        backupFile.setPath(backupFile.path()+".orig");

        int exportExists = fileExists(exportTo);	// see whether file and backup exist
        int backupExists = fileExists(backupFile);
        qDebug() << "exportexists" << exportExists << "backupexists" << backupExists;

        if (exportExists==-1 || backupExists==-1)	// unable to determine either
        {
            if (!fileWarningIgnored(exportTo, "remotebackup"))
            {
                const QString q = xi18nc("@info", "Cannot determine whether the remote file<nl/><filename>%1</filename><nl/> has a backup.<nl/><nl/>Save the file anyway (the original may be overwritten)?",
                                         exportTo.toDisplayString());
                KMessageBox::ButtonCode but = KMessageBox::warningContinueCancel(mainWidget(), q,
                                                                                 i18n("Cannot Check Backup"),
                                                                                 KStandardGuiItem::save());
                if (but==KMessageBox::Cancel) return (FilesController::StatusCancelled);
							// ignore from now on for this file
                setFileWarningIgnored(exportTo, "remotebackup");
            }

            exportExists = 0;				// pretend not overwriting
        }

        if (exportExists==1 && backupExists==0)		// no backup taken yet
        {
            bool ok;
            if (exportTo.isLocalFile())			// to a local file?
            {
                ok = QFile::copy(exportTo.path(), backupFile.path());
            }
            else					// to a remote file
            {
                KIO::FileCopyJob *job = KIO::file_copy(exportTo, backupFile, -1, KIO::Overwrite);
                ok = job->exec();
                qDebug() << "job result" << ok << "error" << job->error();
            }

            if (!ok)
            {
                reportFileError(true, backupFile, i18n("Cannot save backup file"));
                emit statusMessage(i18n("Backup failed"));
                return (FilesController::StatusFailed);
            }

            KMessageBox::information(mainWidget(),
                                     xi18nc("@info", "Original of<nl/><filename>%1</filename><nl/>has been backed up as<nl/><filename>%2</filename>",
                                            exportTo.toDisplayString(), backupFile.toDisplayString()),
                                     i18n("Original file backed up"),
                                     "fileBackupInfo");
        }
    }

    if (options.hasFlag(ImporterExporterOptions::SelectionOnly)) exp->setSelectionId(filesView()->selectionId());

    emit statusMessage(i18n("Saving %1 to <filename>%2</filename>...", exportType, exportTo.toDisplayString()));
    exp->setOptions(options);				// set the export options
    exp->save(exportTo, filesModel()->rootFileItem());

    const ErrorReporter *rep = exp->reporter();
    if (!reportFileError(true, exportTo, rep))
    {
        emit statusMessage(xi18nc("@info", "Saving <filename>%1</filename> failed", exportTo.toDisplayString()));
        return (FilesController::StatusFailed);
    }

    if (options.hasFlag(ImporterExporterOptions::ToClipboard))
    {
        emit statusMessage(xi18nc("@info", "Copied selection to clipboard"));
    }
    else
    {
        if (options.hasFlag(ImporterExporterOptions::SelectionOnly))
        {
            emit statusMessage(xi18nc("@info", "Saved selection to <filename>%1</filename>", exportTo.toDisplayString()));
        }
        else
        {
            emit statusMessage(xi18nc("@info", "Saved to <filename>%1</filename>", exportTo.toDisplayString()));
        }
    }

    return (FilesController::StatusOk);			// done, finished with exporter
}


static int closestDiff;
static const TrackDataTrackpoint *closestPoint;


static void findChildWithTime(const TrackDataItem *pnt, const QDateTime &dt)
{
    const TrackDataTrackpoint *p = dynamic_cast<const TrackDataTrackpoint *>(pnt);
    if (p!=nullptr)
    {
        QDateTime pt = p->time();
        const int diff = abs(dt.secsTo(pt));
        //qDebug() << "  " << p->name() << pt << "diff" << diff;
        if (diff<closestDiff)
        {
            //qDebug() << "  closest" << p->name() << "diff" << diff;
            closestPoint = p;
            closestDiff = diff;
        }
    }
    else
    {
        const int cnt = pnt->childCount();
        for (int i = 0; i<cnt; ++i) findChildWithTime(pnt->childAt(i), dt);
    }
}


// true => time OK, false => needs conversion but can't
bool FilesController::adjustTimeSpec(QDateTime &dt)
{
    if (!dt.isValid()) return (true);			// nothing can be done
    const Qt::TimeSpec ts = dt.timeSpec();
    if (ts==Qt::UTC) return (true);			// nothing to do
    if (ts==Qt::OffsetFromUTC)				// not sure what to do
    {
        qWarning() << "Don't know what to do with OffsetFromUTC time";
        return (false);
    }

    Q_ASSERT(ts==Qt::LocalTime);			// the only possibility left

    // Local time needs to be converted to UTC (using the time zone of the file)
    // in order to correspond with the recording times.  This means that a
    // time zone needs to be set for meaningful results.
    QByteArray zone = filesModel()->rootFileItem()->timeZone().toLocal8Bit();
    if (zone.isEmpty()) return (false);			// if none, can't convert

    QTimeZone tz(zone);
    qDebug() << "file time zone" << tz.displayName(QTimeZone::GenericTime) << "offset" << tz.offsetFromUtc(QDateTime::currentDateTime());

    // The local time is already in the time zone of the camera (which is
    // assumed to be the same as the GPS recording and hence the file).
    // So all that needs to be done is to set the time zone, which will
    // change the time spec to Qt::TimeZone.  We do not want to use
    // QDateTime::toTimeZone() here, that will also adjust the time!
    dt.setTimeZone(tz);

    qDebug() << "  new datetime" << dt << "spec" << dt.timeSpec();
    return (true);
}


FilesController::Status FilesController::importPhoto(const QList<QUrl> &urls)
{
    int q;						// status for questions
							// get the file time zone set
    const QString zone = filesModel()->rootFileItem()->timeZone();
    if (zone.isEmpty() && !mWarnedNoTimezone)		// message only once per file
    {
        q = KMessageBox::warningContinueCancel(mainWidget(),
                                               xi18nc("@info", "No time zone has been set for this file.<nl/>Locating by time and/or the time of<nl/>created waypoints may be incorrect."),
                                               i18n("No Time Zone"));

        if (q==KMessageBox::Cancel) return (FilesController::StatusCancelled);
        mWarnedNoTimezone = true;
    }

    const int total = urls.count();
    const bool multiple = (total>1);
    FilesController::Status result = FilesController::StatusOk;

    QUndoCommand *cmd = new QUndoCommand();		// parent command
    cmd->setText(i18np("Import Photo", "Import %1 Photos", total));

    // Find a folder to place the resulting waypoint in, but do not
    // try to create it at this stage if it does not already exist.
    TrackDataFolder *destFolder = TrackData::findFolderByPath(PHOTO_FOLDER_NAME, filesModel()->rootFileItem());

    for (int i = 0; i<total; ++i)
    {
        const QUrl &importFrom = urls[i];
        qDebug() << importFrom;

        if (!importFrom.isValid()) continue;
        if (!importFrom.isLocalFile())
        {
            const QString messageText = xi18nc("@info", "<filename>%1</filename> is not a local file", importFrom.toDisplayString());
            if (!multiple) KMessageBox::error(mainWidget(), messageText, i18n("Cannot Import"));
            else
            {
                q = KMessageBox::warningContinueCancel(mainWidget(),
                                                       messageText,
                                                       i18n("Cannot Import"),
                                                       KStandardGuiItem::cont(),
                                                       KGuiItem(i18nc("@action:button", "Cancel All"), KStandardGuiItem::cancel().icon()));
                if (q==KMessageBox::Continue) continue;
            }
            return (FilesController::StatusCancelled);
        }

        double alt = 0;
        double lat, lon;
        const TrackDataAbstractPoint *sourcePoint = nullptr;

#ifdef HAVE_KEXIV2
        KExiv2 exi(importFrom.toLocalFile());
        qDebug() << "Exiv2 data:";
        qDebug() << "  dimensions" << exi.getImageDimensions();
        qDebug() << "  orientation" << exi.getImageOrientation();

        // This appears to return the date/time in Qt::LocalTime specification.
        QDateTime dt = exi.getImageDateTime();
        qDebug() << "  datetime" << dt << "spec" << dt.timeSpec();
        bool gpsValid = exi.getGPSInfo(alt,lat,lon);
        qDebug() << "  gps valid?" << gpsValid << "alt" << alt << "lat" << lat << "lon" << lon;

        QString messageText;
        QString statusText;
        bool matched = false;

        adjustTimeSpec(dt);				// check time, even if using GPS

        if (gpsValid && Settings::photoUseGps())
        {
            messageText = xi18nc("@info", "The image file <filename>%1</filename> contained a valid GPS position.<nl/>The waypoint will be created at that position.", importFrom.fileName());
            statusText = xi18nc("@info", "Imported <filename>%1</filename> at GPS position", importFrom.toDisplayString());
            matched = true;
        }
        else
        {
            if (dt.isValid() && Settings::photoUseTime())
            {
                closestDiff = INT_MAX;
                closestPoint = nullptr;
                findChildWithTime(filesModel()->rootFileItem(), dt);

                if (closestPoint!=nullptr && closestDiff<=Settings::photoTimeThreshold())
                {
                    messageText = xi18ncp("@info", "The image <filename>%3</filename> date/time matched point '%2' within %1 second.<nl/>The waypoint will be created at that point position.",
                                          "The image <filename>%3</filename> date/time matched point '%2' within %1 seconds.<nl/>The waypoint will be created at that point position.",
                                        closestDiff, closestPoint->name(), importFrom.fileName());
                    statusText = xi18nc("@info", "Imported <filename>%1</filename> at date/time position", importFrom.toDisplayString());

                    sourcePoint = closestPoint;
                    lat = closestPoint->latitude();
                    lon = closestPoint->longitude();
                    alt = closestPoint->elevation();
                    matched = true;
                }
                else
                {
                    messageText = xi18nc("@info", "The image file <filename>%1</filename> had no GPS position,<nl/>and its date/time did not match any points.<nl/><nl/>The waypoint will be created at the current map centre.", importFrom.fileName());
                }
            }
        }

        if (!matched)
#endif
        {
            if (messageText.isEmpty()) messageText = xi18nc("@info", "The image file <filename>%1</filename> had no GPS position or date/time,<nl/>or the application is not set to use them.<nl/><nl/>The waypoint will be created at the current map centre.", importFrom.fileName());
            statusText = xi18nc("@info", "Imported <filename>%1</filename> at map centre", importFrom.toDisplayString());
            lat = mapController()->view()->centerLatitude();
            lon = mapController()->view()->centerLongitude();
        }

        if (messageText.isEmpty()) continue;		// nothing to do

        if (multiple)
        {
            q = KMessageBox::questionTwoActionsCancel(mainWidget(),
                                                      messageText,
                                                      i18n("Create Waypoint?"),
                                                      KGuiItem(i18nc("@action:button", "Accept"), KStandardGuiItem::ok().icon()),
                                                      KGuiItem(i18nc("@action:button", "Reject"), KStandardGuiItem::cancel().icon()),
                                                      KGuiItem(i18nc("@action:button", "Cancel All"), KStandardGuiItem::stop().icon()));
        }
        else
        {
            q = KMessageBox::questionTwoActions(mainWidget(),
                                                messageText,
                                                i18n("Create Waypoint?"),
                                                KGuiItem(i18nc("@action:button", "Accept"), KStandardGuiItem::ok().icon()),
                                                // KStandardGuiItem::no() used same icon as cancel()
                                                KGuiItem(i18nc("@action:button", "Reject"), KStandardGuiItem::cancel().icon()));
        }
        if (q==KMessageBox::Cancel) return (FilesController::StatusCancelled);
        if (q==KMessageBox::SecondaryAction)		// "Reject"
        {
            if (result==FilesController::StatusOk) result = FilesController::StatusFailed;
            continue;
        }

        // The destination folder now needs to be immediately created if it does
        // not already exist, see slotAddWaypoint().  Delayed until now so that we
        // know that there is a valid and accepted waypoint to create.
        if (destFolder==nullptr)
        {
            qDebug() << "need to add new folder";

            AddContainerCommand *cmd1 = new AddContainerCommand(this);
            cmd1->setText(i18n("Create Photo Folder"));
            cmd1->setName(PHOTO_FOLDER_NAME);
            cmd1->setData(TrackData::Folder, filesModel()->rootFileItem());
            executeCommand(cmd1);
            destFolder = dynamic_cast<TrackDataFolder *>(cmd1->addedItem());
        }
        Q_ASSERT(destFolder!=nullptr);

        // Create the waypoint
        AddPhotoCommand *cmd2 = new AddPhotoCommand(this, cmd);
        cmd2->setData(importFrom.fileName(), lat, lon, destFolder, sourcePoint);
        cmd2->setLink(importFrom);
        cmd2->setTime(dt);

        emit statusMessage(statusText);
    }

    if (cmd->childCount()==0)				// anything to actually do?
    {							// nothing added above, so
        delete cmd;					// don't need this after all
        return (result);
    }

    executeCommand(cmd);
    emit modified();
    return (result);
}


void FilesController::slotUpdateActionState()
{
    TrackData::Type selType = filesView()->selectedType();
    int selCount = filesView()->selectedCount();

    if (selType==TrackData::None)
    {
        emit statusMessage(i18n("Nothing selected"));
    }
    else if (selType==TrackData::Mixed)
    {
        emit statusMessage(i18np("Selected %1 item", "Selected %1 items", selCount));
    }
    else
    {
        const TrackDataItem *tdi = filesView()->selectedItem();
        const QString name = tdi->name();
        QString msg = "";

        switch (selType)
        {
case TrackData::File:       if (selCount==1) msg = i18n("Selected file '%1'", name);
                            else msg = i18np("Selected %1 file", "Selected %1 files", selCount);
                            break;

case TrackData::Track:      if (selCount==1) msg = i18n("Selected track '%1'", name);
                            else msg = i18np("Selected %1 track", "Selected %1 tracks", selCount);
                            break;

case TrackData::Segment:    if (selCount==1) msg = i18n("Selected segment '%1'", name);
                            else msg = i18np("Selected %1 segment", "Selected %1 segments", selCount);
                            break;

case TrackData::Route:      if (selCount==1) msg = i18n("Selected route '%1'", name);
                            else msg = i18np("Selected %1 route", "Selected %1 routes", selCount);
                            break;

case TrackData::Trackpoint: if (selCount==1) msg = i18n("Selected point '%1'", name);
                            else msg = i18np("Selected %1 point", "Selected %1 points", selCount);
                            break;

case TrackData::Waypoint:   if (selCount==1)
                            {
                                const QString wptStatus = TrackData::formattedWaypointStatus(
                                    static_cast<TrackData::WaypointStatus>(tdi->metadata("status").toInt()), true);
                                if (!wptStatus.isEmpty()) msg = i18n("Selected waypoint '%1' (%2)", name, wptStatus);
                                else msg = i18n("Selected waypoint '%1'", name);
                            }
                            else msg = i18np("Selected %1 waypoint", "Selected %1 waypoints", selCount);
                            break;

case TrackData::Routepoint: if (selCount==1) msg = i18n("Selected routepoint '%1'", name);
                            else msg = i18np("Selected %1 routepoint", "Selected %1 routepoints", selCount);
                            break;

case TrackData::Folder:     if (selCount==1) msg = i18n("Selected folder '%1'", name);
                            else msg = i18np("Selected %1 folder", "Selected %1 folders", selCount);
                            break;

default:                    break;
        }

        if (!msg.isEmpty()) emit statusMessage(msg);
    }

    emit updateActionState();
}


void FilesController::slotCheckTimeZone()
{
    TrackDataFile *tdf = filesModel()->rootFileItem();
    if (tdf==nullptr) return;				// check model not empty

    QString zone = tdf->timeZone();			// get current file time zone
    if (!zone.isEmpty()) return;			// time zone is already set

    QUrl file = tdf->fileName();			// URL of loaded file
    if (!file.isValid()) return;			// file must be loaded

    bool notAgain = fileWarningIgnored(file, "timezone");
    if (notAgain) return;				// see if ignored for this file

    KMessageBox::ButtonCode but = KMessageBox::questionTwoActionsCancel(mainWidget(),
        i18n("The file does not have a time zone set.\nDo you want to set or look up one?"),
									// text
        i18n("No Time Zone"),					 	// title
        KGuiItem(i18n("Yes"), KStandardGuiItem::ok().icon()),		// primaryAction
        KGuiItem(i18n("Never"), QIcon::fromTheme("edit-clear-list")), 	// secondaryAction
        KGuiItem(i18n("No"), KStandardGuiItem::cancel().icon()));	// cancelAction

    if (but==KMessageBox::SecondaryAction) setFileWarningIgnored(file, "timezone");
    if (but!=KMessageBox::PrimaryAction) return;

    slotSetTimeZone();					// set via file properties
}


void FilesController::slotTrackProperties()
{
    qDebug();

    QList<TrackDataItem *> items = filesView()->selectedItems();
    if (items.count()==0) return;

    TrackPropertiesDialogue d(&items, filesView());
    QString actText = CommandBase::senderText(sender());
    d.setWindowTitle(actText);

    if (!d.exec()) return;				// execute the dialogue

    TrackDataItem *item = items.first();		// the item to change
    QUndoCommand *cmd = new QUndoCommand();		// parent command
    const MetadataModel *model = d.dataModel();		// model for new data

    // Item name
    const QString newItemName = model->data(DataIndexer::index("name")).toString();
    if (!newItemName.isEmpty() && newItemName!=item->name())
    {							// changing the name
        qDebug() << "name change" << item->name() << "->" << newItemName;
        ChangeItemNameCommand *cmd1 = new ChangeItemNameCommand(this, cmd);
        cmd1->setDataItem(item);
        cmd1->setData(newItemName);
    }

    // Point position
    const QVariant &latData = model->data(DataIndexer::index("latitude"));
    const QVariant &lonData = model->data(DataIndexer::index("longitude"));
    if (!latData.isNull() && !lonData.isNull())		// if applies to this point
    {
        TrackDataAbstractPoint *tdp = dynamic_cast<TrackDataAbstractPoint *>(item);
        Q_ASSERT(tdp!=nullptr);

        const double newLat = latData.toDouble();
        const double newLon = lonData.toDouble();

        if (newLat!=tdp->latitude() || newLon!=tdp->longitude())
        {
            qDebug() << "change position" << newLat << newLon;
            MovePointsCommand *cmd2 = new MovePointsCommand(this, cmd);
            cmd2->setDataItems((QList<TrackDataItem *>() << tdp));
            cmd2->setData(newLat-tdp->latitude(), newLon-tdp->longitude());
        }
    }

    // The remaining metadata
    const int num = model->rowCount();			// same as DataIndexer::count()
    for (int idx = 0; idx<num; ++idx)
    {
        const QByteArray name = DataIndexer::name(idx);
        if (MetadataModel::isInternalTag(name)) continue;
							// these handled specially above
        if (!model->isChanged(idx)) continue;		// data not changed in dialogue
        QVariant newData = model->data(idx);		// the new changed data

        if (name=="status")				// changing waypoint status
        {						// ignore if "No change"
            const TrackData::WaypointStatus wptstatus = static_cast<TrackData::WaypointStatus>(newData.toInt());
            if (wptstatus==TrackData::StatusInvalid) continue;
        }
        //else if (name=="linecolor" || name=="pointcolor")
        //{						// changing item colour
        //    // Alpha value encodes the inherit flag, see TrackItemStylePage
        //    QColor col = newData.value<QColor>();
        //    if (col.alpha()!=255) newData = QVariant();	// here null colour means inherit
        //}

        qDebug() << "index" << idx << name << "->" << newData;
        ChangeItemDataCommand *cmd3 = new ChangeItemDataCommand(this, cmd);
        cmd3->setDataItems(items);
        // TODO: overload setData() to take an index
        cmd3->setData(DataIndexer::name(idx), newData);
    }

    if (cmd->childCount()==0)				// anything to actually do?
    {							// no changes above, so
        delete cmd;					// don't need this after all
        return;
    }

    cmd->setText(actText);
    executeCommand(cmd);
}


void FilesController::slotSetWaypointStatus()
{
    QAction *act = qobject_cast<QAction *>(sender());
    if (act==nullptr) return;
    int newStatus = act->data().toInt();

    ChangeItemDataCommand *cmd = new ChangeItemDataCommand(this);
    cmd->setText(i18n("Waypoint Status %1", CommandBase::senderText(sender())));
    cmd->setDataItems(filesView()->selectedItems());
    cmd->setData("status", (newStatus==0) ? "" : QString::number(newStatus));
    executeCommand(cmd);
}


void FilesController::slotSplitSegment()
{
    QList<TrackDataItem *> items = filesView()->selectedItems();
    if (items.count()!=1) return;
    const TrackDataItem *item = items.first();

    TrackDataItem *pnt = item->parent();
    if (pnt==nullptr) return;
    qDebug() << "split" << pnt->name() << "at" << item->name();

    int idx = pnt->childIndex(item);
    if (idx==0 || idx>=(pnt->childCount()-1))
    {
        KMessageBox::error(mainWidget(),
                           xi18nc("@info", "Cannot split the segment or route here<nl/>(at its start or end point)"),
                           i18n("Cannot split segment"));
        return;
    }

    SplitSegmentCommand *cmd = new SplitSegmentCommand(this);
    cmd->setSenderText(sender());
    cmd->setData(pnt, idx);
    executeCommand(cmd);
}



static bool compareSegmentTimes(const TrackDataItem *item1, const TrackDataItem *item2)
{
    if (item1->childCount()==0) return (true);		// empty always sorts first
    if (item2->childCount()==0) return (false);

    const TrackDataAbstractPoint *pnt1 = dynamic_cast<TrackDataAbstractPoint *>(item1->childAt(0));
    Q_ASSERT(pnt1!=nullptr);
    const TrackDataAbstractPoint *pnt2 = dynamic_cast<TrackDataAbstractPoint *>(item2->childAt(0));
    Q_ASSERT(pnt2!=nullptr);

    return (pnt1->time()<pnt2->time());
}


void FilesController::slotMergeSegments()
{
    // There may be more than two segments selected.  They will all be merged
    // in time order, which must still result in a strict monotonic ordering
    // of the segment points.  In other words, the master segment will be the
    // one with the earliest start time and each segment merged into it in turn
    // must have a start time after the current end of the master.
    //
    // Routes and route points do not have associated times.  In this case,
    // they are simply merged in file order.
    //
    // This can also be enabled for waypoints, in which case the manual
    // merge dialogue is started.

    QList<TrackDataItem *> items = filesView()->selectedItems();
    const int num = items.count();
    if (num<2) return;

    if (dynamic_cast<const TrackDataSegment *>(items.first())!=nullptr)
    {							// operating on segments
        std::sort(items.begin(), items.end(), &compareSegmentTimes);

        qDebug() << "sorted segments:";
        QDateTime prevEnd;
        for (int i = 0; i<num; ++i)
        {
            const TrackDataSegment *tds = dynamic_cast<const TrackDataSegment *>(items[i]);
            const TrackDataTrackpoint *pnt1 = dynamic_cast<TrackDataTrackpoint *>(tds->childAt(0));
            const TrackDataTrackpoint *pnt2 = dynamic_cast<TrackDataTrackpoint *>(tds->childAt(tds->childCount()-1));
            qDebug() << "  " << tds->name() << "start" << pnt1->formattedTime() << "end" << pnt2->formattedTime();

            if (i>0 && pnt1->time()<prevEnd)		// check no time overlap
            {						// all apart from first
                KMessageBox::error(mainWidget(), xi18nc("@info", "Cannot merge these segments<nl/><nl/>Start time of segment \"%1\"<nl/>overlaps the previous \"%2\"",
                                                        tds->name(), items[i-1]->name()),
                                   i18n("Cannot merge segments"));
                return;
            }

            prevEnd = pnt2->time();				// note end for next time
        }
    }
    else if (dynamic_cast<const TrackDataWaypoint *>(items.first())!=nullptr)
    {							// operating on waypoints
        // from NavTracks PointsController::slotMergeSelection()
        emit statusMessage(i18n("Checking positions"));

        const TrackDataWaypoint *refPoint = nullptr;	// first reference point
        bool mergeOk = true;				// positions close enough?
        QList<const TrackDataWaypoint *> pointsToMerge;	// waypoint list to merge

        for (const TrackDataItem *item : qAsConst(items))
        {						// check reference against others
            const TrackDataWaypoint *tdw = dynamic_cast<const TrackDataWaypoint *>(item);
            if (tdw==nullptr)				// should never happen, enforced by GUI
            {
                qWarning() << "trying to merge non-waypoint" << item->name();
                return;
            }

            if (refPoint==nullptr) refPoint = tdw;	// note the first reference point
            else					// compare others against it
            {
                if (!refPoint->canMerge(tdw, true))	// position check only
                {
                    mergeOk = false;			// point is too far away
                }
            }

            pointsToMerge.append(tdw);			// cast to a waypoint
        }

        if (!mergeOk)					// points were too far apart
        {
            // TODO: show the difference distance
            // TODO: should really use i18np() here, because some languages have
            // more plural forms than just "1" or "more".
            QString query = xi18nc("@info", "Some of the selected points are not close together.<nl/>Really merge the %1 points?", num);
            if (KMessageBox::warningContinueCancel(mainWidget(), query,
                                                   i18n("Merge Points"),
                                                   KGuiItem(i18n("Merge"), QIcon::fromTheme("merge")))!=KMessageBox::Continue)
            {
                emit statusMessage(i18n("Merge cancelled"));
                return;
            }
        }

        emit statusMessage(i18n("Manual merge"));

        MergePointsDialogue d(mainWidget());
        d.setPoints(&pointsToMerge);
        if (!d.exec())
        {
            emit statusMessage(i18n("Merge cancelled"));
            return;
        }

        ReplaceItemsCommand *cmd = new ReplaceItemsCommand(this);
        cmd->setSenderText(sender());
        cmd->setData(items, d.resultPoint());
        executeCommand(cmd);

        slotUpdateActionState();
        emit statusMessage(i18n("Merged %1 points", num));
        return;
    }

    // operating on a segment or route here
    TrackDataItem *masterSeg = items.takeFirst();

    MergeSegmentsCommand *cmd = new MergeSegmentsCommand(this);
    cmd->setSenderText(sender());
    cmd->setData(masterSeg, items);
    executeCommand(cmd);
}


void FilesController::slotMoveItem()
{
    QList<TrackDataItem *> items = filesView()->selectedItems();
    //if (items.count()!=1) return;
    const TrackDataItem *item = items.first();

    MoveItemDialogue d(filesView());
    d.setSource(&items);

    QString capt;
    if (dynamic_cast<const TrackDataSegment *>(item)!=nullptr) capt = i18nc("@title:window", "Move Segment");
    else if (dynamic_cast<const TrackDataFolder *>(item)!=nullptr) capt = i18nc("@title:window", "Move Folder");
    else if (dynamic_cast<const TrackDataWaypoint *>(item)!=nullptr) capt = i18nc("@title:window", "Move Waypoint");
    if (!capt.isEmpty()) d.setWindowTitle(capt);

    if (!d.exec()) return;

    TrackDataItem *dest = d.selectedItem();
    MoveItemCommand *cmd = new MoveItemCommand(this);
    cmd->setSenderText(sender());
    cmd->setData(items, dest);
    executeCommand(cmd);
}


void FilesController::slotAddTrack()
{
    QList<TrackDataItem *> items = filesView()->selectedItems();
    if (items.count()!=1) return;
    TrackDataItem *pnt = items.first();			// parent item (must be file)
    Q_ASSERT(dynamic_cast<TrackDataFile *>(pnt)!=nullptr);

    AddContainerCommand *cmd = new AddContainerCommand(this);
    cmd->setSenderText(sender());
    cmd->setData(TrackData::Track);
    executeCommand(cmd);
}


void FilesController::slotAddRoute()
{
    QList<TrackDataItem *> items = filesView()->selectedItems();
    if (items.count()!=1) return;
    TrackDataItem *pnt = items.first();			// parent item (must be file)
    Q_ASSERT(dynamic_cast<TrackDataFile *>(pnt)!=nullptr);

    AddContainerCommand *cmd = new AddContainerCommand(this);
    cmd->setSenderText(sender());
    cmd->setData(TrackData::Route);
    executeCommand(cmd);
}


void FilesController::slotAddFolder()
{
    QList<TrackDataItem *> items = filesView()->selectedItems();
    if (items.count()!=1) return;
    TrackDataItem *pnt = items.first();			// parent item (file or folder)
    Q_ASSERT(dynamic_cast<TrackDataFile *>(pnt)!=nullptr || dynamic_cast<TrackDataFolder *>(pnt)!=nullptr);

    AddContainerCommand *cmd = new AddContainerCommand(this);
    cmd->setSenderText(sender());
    cmd->setData(TrackData::Folder, pnt);
    executeCommand(cmd);
}


void FilesController::slotAddTrackpoint()
{
    QList<TrackDataItem *> items = filesView()->selectedItems();
    if (items.count()!=1) return;

    AddTrackpointCommand *cmd = new AddTrackpointCommand(this);
    cmd->setSenderText(sender());
    cmd->setData(items.first());
    executeCommand(cmd);
}


void FilesController::slotDeleteItems()
{
    QList<TrackDataItem *> items = filesView()->selectedItems();
    int num = items.count();
    if (num==0) return;

    QString query;
    if (num==1)
    {
        const TrackDataItem *tdi = items.first();
        if (tdi->childCount()==0)
        {
            query = xi18nc("@info", "Delete the selected item \"<emphasis strong=\"1\">%1</emphasis>\"?", tdi->name());
        }
        else
        {
            query = xi18nc("@info", "Delete the selected item \"<emphasis strong=\"1\">%1</emphasis>\"<nl/>and everything under it?", tdi->name());
        }
    }
    else
    {
        query = i18np("Delete the selected item?", "Delete the %1 selected items?", num);
    }

    int result = KMessageBox::warningContinueCancel(mainWidget(), query,
                                                    i18n("Confirm Delete"),
                                                    KStandardGuiItem::del());
    if (result!=KMessageBox::Continue) return;

    DeleteItemsCommand *cmd = new DeleteItemsCommand(this);
    cmd->setSenderText(sender());
    cmd->setData(items);
    executeCommand(cmd);
}


void FilesController::slotAddWaypoint(qreal lat, qreal lon)
{
    // There are 3 options for specifying the position of the waypoint:
    //
    //   1.	If the click was over the map, then the coordinates passed in
    //		will be valid and the waypoint can be created there.
    //
    //   2.	If there is a selected track point or waypoint, then the
    //		waypoint can be created as a copy at the same position.
    //
    //   3.	The user may enter the coordinates.

    CreatePointDialogue d(false, mainWidget());		// waypoint mode
    if (!d.canCreate())					// no folders available to create
    {
        // For creating a waypoint (but not a routepoint below), if there
        // is no existing folder where the waypoint can be created then
        // one is created automatically.  This is to handle the case of
        // starting from an empty file in points list mode where this may
        // not be obvious, but for simplicity it also works in tree view mode.

        // Create a default named folder to contain the created waypoint.
        AddContainerCommand *cmd1 = new AddContainerCommand(this);
        // This assumes that the folder to be created is at the top level.
        // Safe to assume this, see StopDetectDialogue::slotCommitResults()
        // for why.
        cmd1->setData(TrackData::Folder, filesModel()->rootFileItem());
        cmd1->setText(i18n("Create Waypoint Folder"));
        executeCommand(cmd1);

        TrackDataFolder *destFolder = dynamic_cast<TrackDataFolder *>(cmd1->addedItem());
        Q_ASSERT(destFolder!=nullptr);			// should now have been created
        d.setDestinationContainer(destFolder);
    }
    Q_ASSERT(d.canCreate());				// must be possible now

    const QList<TrackDataItem *> items = filesView()->selectedItems();

    const TrackDataItem *sel = (items.count()==1 ? items.first() : nullptr);
    const TrackDataAbstractPoint *selPoint = dynamic_cast<const TrackDataAbstractPoint *>(sel);
    const TrackDataFolder *selFolder = dynamic_cast<const TrackDataFolder *>(sel);
    const TrackDataWaypoint *selWaypoint = dynamic_cast<const TrackDataWaypoint *>(sel);

    // Select where the new waypoint is to be placed.  If a single
    // folder is selected, then add it to that folder.  Otherwise, if a
    // single waypoint is selected, then add it to its parent folder.
    // Otherwise, allow the user to select where it is to go.
    if (selWaypoint!=nullptr)
    {
        selFolder = dynamic_cast<const TrackDataFolder *>(sel->parent());
        selPoint = nullptr;			// don't want this as source
    }
    if (selFolder!=nullptr) d.setDestinationContainer(selFolder);

    // Set the point coordinates.  Either from the parameters, if
    // explicitly provided, or from the source point if it is not
    // already a waypoint.
    if (!ISNAN(lat)) d.setSourceLatLong(lat, lon);
    else if (selPoint!=nullptr) d.setSourcePoint(selPoint);

    if (!d.exec()) return;

    d.pointPosition(&lat, &lon);
    const QString name = d.pointName();
    TrackDataFolder *destFolder = dynamic_cast<TrackDataFolder *>(d.selectedContainer());
    Q_ASSERT(destFolder!=nullptr);

    qDebug() << "create" << name << "in" << destFolder->name() << "at" << lat << lon;

    AddWaypointCommand *cmd2 = new AddWaypointCommand(this);

    QObject *sdr = sender();				// may be called directly
    if (qobject_cast<MapView *>(sdr)!=nullptr) cmd2->setText(i18n("Create Waypoint on Map"));
    else cmd2->setSenderText(sdr);

    cmd2->setData(name, lat, lon, destFolder, selPoint);
    executeCommand(cmd2);
}


void FilesController::slotAddRoutepoint(qreal lat, qreal lon)
{
    CreatePointDialogue d(true, mainWidget());		// route point mode
    if (!d.canCreate())
    {
        KMessageBox::error(mainWidget(),
                           i18n("There are no routes where a point can be created."),
                           i18n("Cannot Create Routepoint"));
        return;
    }

    const QList<TrackDataItem *> items = filesView()->selectedItems();

    const TrackDataItem *sel = (items.count()==1 ? items.first() : nullptr);
    const TrackDataAbstractPoint *selPoint = dynamic_cast<const TrackDataAbstractPoint *>(sel);
    const TrackDataRoute *selRoute = dynamic_cast<const TrackDataRoute *>(sel);
    const TrackDataRoutepoint *selRoutepoint = dynamic_cast<const TrackDataRoutepoint *>(sel);

    // Select where the new route point is to be placed.  If a single
    // route is selected, then add it to that route.  Otherwise, if a
    // single route point is selected, then add it to its parent route.
    // Otherwise, allow the user to select where it is to go.
    if (selRoutepoint!=nullptr)
    {
        selRoute = dynamic_cast<const TrackDataRoute *>(sel->parent());
        selPoint = nullptr;			// don't want this as source
    }
    if (selRoute!=nullptr) d.setDestinationContainer(selRoute);

    // Set the point coordinates.  Either from the parameters, if
    // explicitly provided, or from the source point if it is not
    // already a route point.
    if (!ISNAN(lat)) d.setSourceLatLong(lat, lon);
    else if (selPoint!=nullptr) d.setSourcePoint(selPoint);

    if (!d.exec()) return;

    d.pointPosition(&lat, &lon);
    const QString name = d.pointName();
    TrackDataRoute *destRoute = dynamic_cast<TrackDataRoute *>(d.selectedContainer());
    Q_ASSERT(destRoute!=nullptr);

    qDebug() << "create" << name << "in" << destRoute->name() << "at" << lat << lon;

    AddRoutepointCommand *cmd = new AddRoutepointCommand(this);

    QObject *sdr = sender();				// may be called directly
    if (qobject_cast<MapView *>(sdr)!=nullptr) cmd->setText(i18n("Create Route Point on Map"));
    else cmd->setSenderText(sdr);

    cmd->setData(name, lat, lon, destRoute, selPoint);
    executeCommand(cmd);
}

//////////////////////////////////////////////////////////////////////////
//									//
//  Drag and Drop							//
//									//
//////////////////////////////////////////////////////////////////////////

void FilesController::slotDragDropItems(const QList<TrackDataItem *> &sourceItems, TrackDataItem *ontoParent, int row)
{
    qDebug() << "items" << sourceItems.count() << "parent" << ontoParent->name() << "row" << row;

    MoveItemCommand *cmd = new MoveItemCommand(this);
    cmd->setText(i18n("Drag/Drop"));
    cmd->setData(sourceItems, ontoParent, row);
    executeCommand(cmd);
}


void FilesController::slotMapDraggedPoints(qreal latOff, qreal lonOff)
{
    qDebug() << latOff << lonOff;

    MovePointsCommand *cmd = new MovePointsCommand(this);
    cmd->setText(i18n("Move Points"));
    cmd->setDataItems(filesView()->selectedItems());
    cmd->setData(latOff, lonOff);
    executeCommand(cmd);
}

//////////////////////////////////////////////////////////////////////////
//									//
//  File type filters							//
//									//
//////////////////////////////////////////////////////////////////////////

static const char allFilter[] = "All Files (*)";


QString FilesController::allExportFilters()
{
    QStringList filters;
    filters << GpxExporter::filter();
    return (filters.join(";;"));
}


QString FilesController::allImportFilters()
{
    QStringList filters;
    filters << GpxImporter::filter();
    filters << MarksImporter::filter();
    filters << allFilter;
    return (filters.join(";;"));
}


QString FilesController::allProjectFilters(bool includeAllFiles)
{
    QStringList filters;
    filters << GpxImporter::filter();
    if (includeAllFiles) filters << allFilter;
    return (filters.join(";;"));
}

//////////////////////////////////////////////////////////////////////////
//									//
//  File time zone							//
//									//
//  Setting the time zone this way has the same effect as setting the	//
//  "timezone" data on the root file item, except that it is allowed	//
//  to be done on a read-only file and does not affect the modified	//
//  state of the file.							//
//									//
//////////////////////////////////////////////////////////////////////////

void FilesController::slotSetTimeZone()
{
    TrackDataItem *root = filesModel()->rootFileItem();
    if (root==nullptr) return;

// TODO: simplify, extract the time zone and keep it in a member variable
// search for it again on model reset

    TimeZoneSettingDialogue d(mainWidget());
    const QString &oldZone = root->timeZone();
    d.setTimeZone(oldZone);
    QList<TrackDataItem *> items;
    items.append(root);
    d.setItems(&items);

    if (!d.exec()) return;
    const QString &newZone = d.timeZone();
    if (newZone==oldZone) return;
    qDebug() << "time zone" << oldZone << "->" << newZone;

    if (isReadOnly())
    {
        // If the file is read only, then change the time zone metadata of
        // the root file item directly.  This will not take account of or
        // affect the file modification state and will not show up in the
        // undo history.
        root->setMetadata("timezone", newZone);
    }
    else
    {
        // Otherwise, change the root file item data in the normal way.
        ChangeItemDataCommand *cmd = new ChangeItemDataCommand(this);
        cmd->setText(i18n("Set Time Zone"));
        cmd->setDataItems(items);
        cmd->setData("timezone", newZone);
        executeCommand(cmd);
    }
}


void FilesController::slotManageCategories()
{
    CategoryList *cats = filesModel()->rootFileItem()->categories();
    CategoriesManageDialogue d(cats, mainWidget());	// existing categories, may be none
    if (!d.exec()) return;

    // TODO: maybe should be undo'able

    const CategoryList *newCats = d.categories();	// updated categories from dialogue
    Q_ASSERT(newCats!=nullptr);

    if (cats==nullptr)					// no categories previously set
    {
        if (newCats->count()==0) return;		// none to add, nothing to do
        cats = new CategoryList;			// allocate new and set on root
        filesModel()->rootFileItem()->setCategories(cats);
    }

    cats->clear();
    cats->addCategories(newCats);
    emit modified();
}
