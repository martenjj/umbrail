
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
#ifdef SORTABLE_VIEW
#include <qsortfilterproxymodel.h>
#endif
#ifdef HAVE_KEXIV2
#include <qtimezone.h>
#endif

#include <klocalizedstring.h>
#include <kmessagebox.h>
#include <kactioncollection.h>

#ifdef HAVE_KEXIV2
#include <kexiv2/kexiv2.h>
using namespace KExiv2Iface;
#endif

#include "filesmodel.h"
#include "filesview.h"
#include "commands.h"
#include "gpximporter.h"
#include "gpxexporter.h"
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

#define GROUP_FILES		"Files"

#define PHOTO_FOLDER_NAME	"Photos"


bool DialogueConstraintFilter::eventFilter(QObject *obj, QEvent *ev)
{
    if (ev->type()==QEvent::Show)
    {
        QWidget*w = qobject_cast<QWidget *>(obj);	// auto resize when "Details" toggled
        if (w!=nullptr) w->layout()->setSizeConstraint(QLayout::SetFixedSize);
    }

    return (false);					// always pass the event on
}


FilesController::FilesController(QObject *pnt)
    : QObject(pnt),
      MainWindowInterface(pnt)
{
    qDebug();

    mDataModel = new FilesModel(this);

#ifdef SORTABLE_VIEW
    mProxyModel = new QSortFilterProxyModel(this);
    mProxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
    mProxyModel->setSortRole(Qt::UserRole);
    mProxyModel->setSourceModel(mDataModel);
    mProxyModel->setDynamicSortFilter(true);
#endif

    mView = new FilesView(mainWindow());
#ifdef SORTABLE_VIEW
    mView->setModel(mProxyModel);
#else
    mView->setModel(mDataModel);
#endif
    connect(mView, SIGNAL(updateActionState()), SLOT(slotUpdateActionState()));
    connect(mDataModel, SIGNAL(clickedItem(const QModelIndex &,unsigned int)),
            mView, SLOT(slotClickedItem(const QModelIndex &,unsigned int)));

    mWarnedNoTimezone = false;
}


FilesController::~FilesController()
{
    qDebug() << "done";
}


void FilesController::readProperties()
{
    view()->readProperties();
}

void FilesController::saveProperties()
{
    view()->saveProperties();
}


void FilesController::initNew()
{
    Q_ASSERT(model()->rootFileItem()==NULL);

    TrackDataFile *fileItem = new TrackDataFile;
    model()->setRootFileItem(fileItem);
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

    KConfigGroup grp = KSharedConfig::openConfig()->group("FileWarnings");
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
                   xi18nc("@info", "The file <filename>%1</filename> was saved but with warnings.", file.toDisplayString()) :
                   xi18nc("@info", "The file <filename>%1</filename> was loaded but with warnings.", file.toDisplayString()));
        caption = (saving ? i18n("File Save Warning") : i18n("File Load Warning"));
        iconName = "dialog-information";
        askText = i18n("Do not show again for this file");
        break;

case ErrorReporter::Error:
        message = (saving ?
                   xi18nc("@info", "The file <filename>%1</filename> was saved but had errors.", file.toDisplayString()) :
                   xi18nc("@info", "The file <filename>%1</filename> was loaded but had errors.", file.toDisplayString()));
        caption = (saving ? i18n("File Save Error") : i18n("File Load Error"));
        iconName = "dialog-warning";
        break;

case ErrorReporter::Fatal:
        message = (saving ?
                   xi18nc("@info", "The file <filename>%1</filename> could not be saved.<nl/>%2", file.toDisplayString(), list.last()) :
                   xi18nc("@info", "The file <filename>%1</filename> could not be loaded.<nl/>%2", file.toDisplayString(), list.last()));
        caption = (saving ? i18n("File Save Failure") : i18n("File Load Failure"));
        iconName = "dialog-error";
        result = false;
        break;
    }

    if (!detailed && rep->severity()!=ErrorReporter::Fatal)
    {							// don't want full details?
        message += i18n("<br><br>%1", list.last());	// just show the last message
							// if not shown already
        list.clear();					// don't show in list
    }

    // Using the message box indirectly here for versatility.

    bool notAgain = false;

    QDialog *dlg = new QDialog(mainWindow());
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
    // access the state of the check box when the dialogue finished.  Therefore,
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


FilesController::Status FilesController::importFile(const QUrl &importFrom)
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

    if (imp.isNull())					// could not create importer
    {
        qDebug() << "Unknown import format" << importType;
        reportFileError(false, importFrom, i18n("Unknown import format"));
        return (FilesController::StatusFailed);
    }

    emit statusMessage(i18n("Loading %1 from <filename>%2</filename>...", importType, importFrom.toDisplayString()));
    TrackDataFile *tdf = imp->load(importFrom);		// do the import

    const ErrorReporter *rep = imp->reporter();
    if (!reportFileError(false, importFrom, rep))
    {
        emit statusMessage(xi18nc("@info", "Loading <filename>%1</filename> failed", importFrom.toDisplayString()));
        return (FilesController::StatusFailed);
    }

    Q_ASSERT(tdf!=NULL);
    ImportFileCommand *cmd = new ImportFileCommand(this);
    cmd->setText(i18n("Import"));
    cmd->setData(tdf);					// takes ownership of tree

    if (model()->isEmpty())				// any data held so far?
    {
        cmd->redo();					// no, just do the import
        delete cmd;					// no need for this now
        emit statusMessage(xi18nc("@info", "Loaded <filename>%1</filename>", importFrom.toDisplayString()));
    }
    else
    {
        mainWindow()->executeCommand(cmd);		// make the operation undo'able
        emit statusMessage(xi18nc("@info", "Imported <filename>%1</filename>", importFrom.toDisplayString()));
    }

    emit modified();					// done, finished with importer

    if (imp->needsResave() && !fileWarningIgnored(importFrom, "warnings")) return (FilesController::StatusResave);
    return (FilesController::StatusOk);
}


FilesController::Status FilesController::exportFile(const QUrl &exportTo, const TrackDataFile *tdf, ImporterExporterBase::Options options)
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
    qDebug() << "to" << exportTo << "type" << exportType << "options" << options;

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

    if (exportTo.isLocalFile() && QFile::exists(exportTo.path()))
    {							// to a local file?
        QUrl backupFile = exportTo;			// make path for backup file
        backupFile.setPath(backupFile.path()+".orig");

// TODO: use KIO
// ask whether to backup if a remote file (if cannot test for exists)
        if (!QFile::exists(backupFile.path()))		// no backup already?
        {
            if (!QFile::copy(exportTo.path(), backupFile.path()))
            {
                reportFileError(true, backupFile, i18n("Cannot save backup file"));
                emit statusMessage(i18n("Backup failed"));
                return (FilesController::StatusFailed);
            }

            KMessageBox::information(mainWindow(),
                                     xi18nc("@info", "Original of<nl/><filename>%1</filename><nl/>has been backed up as<nl/><filename>%2</filename>",
                                          exportTo.toDisplayString(), backupFile.toDisplayString()),
                                     i18n("Original file backed up"),
                                     "fileBackupInfo");
        }
    }

    if (options & ImporterExporterBase::SelectionOnly) exp->setSelectionId(view()->selectionId());

    emit statusMessage(i18n("Saving %1 to <filename>%2</filename>...", exportType, exportTo.toDisplayString()));
    exp->save(exportTo, tdf, options);

    const ErrorReporter *rep = exp->reporter();
    if (!reportFileError(true, exportTo, rep))
    {
        emit statusMessage(xi18nc("@info", "Saving <filename>%1</filename> failed", exportTo.toDisplayString()));
        return (FilesController::StatusFailed);
    }

    if (options & ImporterExporterBase::ToClipboard)
    {
        emit statusMessage(xi18nc("@info", "Copied selection to clipboard"));
    }
    else
    {
        if (options & ImporterExporterBase::SelectionOnly)
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
    if (p!=NULL)
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

    QByteArray zone = model()->rootFileItem()->timeZone().toLocal8Bit();
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

    QString zone = model()->rootFileItem()->timeZone();	// get the file time zone set
    if (zone.isEmpty() && !mWarnedNoTimezone)		// message only once per file
    {
        q = KMessageBox::warningContinueCancel(mainWindow(),
                                               xi18nc("@info", "No time zone has been set for this file.<nl/>Locating by time and/or the time of<nl/>created waypoints may be incorrect."),
                                               i18n("No Time Zone"));

        if (q==KMessageBox::Cancel) return (FilesController::StatusCancelled);
        mWarnedNoTimezone = true;
    }

    const int total = urls.count();
    const bool multiple = (total>1);
    FilesController::Status result = FilesController::StatusOk;
    bool containerCreated = false;

    QUndoCommand *cmd = new QUndoCommand();		// parent command
    cmd->setText(i18np("Import Photo", "Import %1 Photos", total));

    for (int i = 0; i<total; ++i)
    {
        const QUrl &importFrom = urls[i];
        qDebug() << importFrom;

        if (!importFrom.isValid()) continue;
        if (!importFrom.isLocalFile())
        {
            const QString messageText = xi18nc("@info", "<filename>%1</filename> is not a local file", importFrom.toDisplayString());
            if (!multiple) KMessageBox::sorry(mainWindow(), messageText, i18n("Cannot Import"));
            else
            {
                q = KMessageBox::warningContinueCancel(mainWindow(),
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
        const TrackDataAbstractPoint *sourcePoint = NULL;

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
                closestPoint = NULL;
                findChildWithTime(model()->rootFileItem(), dt);

                if (closestPoint!=NULL && closestDiff<=Settings::photoTimeThreshold())
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
                    messageText = xi18nc("@info", "The image file <filename>%1</filename> had no GPS position, and its date/time did not match any points.<nl/>The waypoint will be created at the current map centre.", importFrom.fileName());
                }
            }
        }

        if (!matched)
#endif
        {
            if (messageText.isEmpty()) messageText = xi18nc("@info", "The image file <filename>%1</filename> had no GPS position or date/time, or the application is not set to use them.<nl/>The waypoint will be created at the current map centre.", importFrom.fileName());
            statusText = xi18nc("@info", "Imported <filename>%1</filename> at map centre", importFrom.toDisplayString());
            lat = mapController()->view()->centerLatitude();
            lon = mapController()->view()->centerLongitude();
        }

        if (messageText.isEmpty()) continue;		// nothing to do

        if (multiple)
        {
            q = KMessageBox::questionYesNoCancel(mainWindow(),
                                                 messageText,
                                                 i18n("Create Waypoint?"),
                                                 KGuiItem(i18nc("@action:button", "Accept"), KStandardGuiItem::yes().icon()),
                                                 KGuiItem(i18nc("@action:button", "Reject"), KStandardGuiItem::no().icon()),
                                                 KGuiItem(i18nc("@action:button", "Cancel All"), KStandardGuiItem::cancel().icon()));
        }
        else
        {
            q = KMessageBox::questionYesNo(mainWindow(),
                                           messageText,
                                           i18n("Create Waypoint?"),
                                           KGuiItem(i18nc("@action:button", "Accept"), KStandardGuiItem::yes().icon()),
                                           KGuiItem(i18nc("@action:button", "Reject"), KStandardGuiItem::no().icon()));
        }
        if (q==KMessageBox::Cancel) return (FilesController::StatusCancelled);
        if (q==KMessageBox::No)
        {
            if (result==FilesController::StatusOk) result = FilesController::StatusFailed;
            continue;
        }

        // Find or create a folder to place the resulting waypoint in
        TrackDataFolder *foundFolder = TrackData::findFolderByPath(PHOTO_FOLDER_NAME, model()->rootFileItem());
        if (foundFolder==NULL)				// find where to store point
        {
            if (containerCreated) qDebug() << "new folder already added";
            else
            {
                qDebug() << "need to add new folder";
                AddContainerCommand *cmd1 = new AddContainerCommand(this, cmd);
                cmd1->setData(TrackData::Folder, model()->rootFileItem());
                cmd1->setName(PHOTO_FOLDER_NAME);
                containerCreated = true;
            }
        }

        // Create the waypoint
        AddPhotoCommand *cmd2 = new AddPhotoCommand(this, cmd);
        cmd2->setData(importFrom.fileName(), lat, lon, foundFolder, sourcePoint);
        cmd2->setLink(importFrom);
        cmd2->setTime(dt);

        emit statusMessage(statusText);
    }

    if (cmd->childCount()==0)				// anything to actually do?
    {							// nothing added above, so
        delete cmd;					// don't need this after all
        return (result);
    }

    mainWindow()->executeCommand(cmd);
    emit modified();
    return (result);
}


void FilesController::slotUpdateActionState()
{
    TrackData::Type selType = view()->selectedType();
    int selCount = view()->selectedCount();

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
        const TrackDataItem *tdi = view()->selectedItem();
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

case TrackData::Point:      if (selCount==1) msg = i18n("Selected point '%1'", name);
                            else msg = i18np("Selected %1 point", "Selected %1 points", selCount);
                            break;

case TrackData::Waypoint:   if (selCount==1) msg = i18n("Selected waypoint '%1'", name);
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
    TrackDataFile *tdf = model()->rootFileItem();
    if (tdf==NULL) return;				// check model not empty

    QString zone = tdf->timeZone();			// get current file time zone
    if (!zone.isEmpty()) return;			// time zone is already set

    QUrl file = tdf->fileName();			// URL of loaded file
    if (!file.isValid()) return;			// file must be loaded

    bool notAgain = fileWarningIgnored(file, "timezone");
    if (notAgain) return;				// see if ignored for this file

    KMessageBox::ButtonCode but = KMessageBox::questionYesNoCancel(mainWindow(),
        i18n("The file does not have a time zone set.\nDo you want to set or look up one?"),
									// text
        i18n("No Time Zone"),					 	// caption
        KStandardGuiItem::yes(),				 	// buttonYes
        KGuiItem(i18n("Never"), QIcon::fromTheme("edit-clear-list")), 	// buttonNo
        KStandardGuiItem::no());					// buttonCancel

    if (but==KMessageBox::No) setFileWarningIgnored(file, "timezone");
    if (but!=KMessageBox::Yes) return;

    // Select the top-level file item.
    view()->slotClickedItem(static_cast<FilesModel *>(model())->indexForItem(model()->rootFileItem()),
                            QItemSelectionModel::ClearAndSelect);

    TrackPropertiesDialogue::setNextPageIndex(0);	// open at "General" page
    // Trigger the action, so that the dialogue can get its text for the window caption.
    QAction *act = mainWindow()->actionCollection()->action("track_properties");
    Q_ASSERT(act!=NULL);
    QTimer::singleShot(0, act, SLOT(trigger()));
}


void FilesController::slotTrackProperties()
{
    qDebug();

    QList<TrackDataItem *> items = view()->selectedItems();
    if (items.count()==0) return;

    TrackPropertiesDialogue d(&items, view());
    QString actText = CommandBase::senderText(sender());
    d.setWindowTitle(actText);

    if (!d.exec()) return;

    TrackDataItem *item = items.first();		// the item to change
    QUndoCommand *cmd = new QUndoCommand();		// parent command
    const MetadataModel *model = d.dataModel();		// model for new data

    // Item name
    const QString newItemName = model->data(DataIndexer::self()->index("name")).toString();
    if (!newItemName.isEmpty() && newItemName!=item->name())
    {							// changing the name
        qDebug() << "name change" << item->name() << "->" << newItemName;
        ChangeItemNameCommand *cmd1 = new ChangeItemNameCommand(this, cmd);
        cmd1->setDataItem(item);
        cmd1->setData(newItemName);
    }

//     QString newTimeZone = d.newTimeZone();
//     //qDebug() << "new timezone" << newTimeZone;
//     if (newTimeZone!=item->metadata("timezone").toString())
//     {
//         qDebug() << "timezone change" << item->metadata("timezone") << "->" << newTimeZone;
//         ChangeItemDataCommand *cmd2 = new ChangeItemDataCommand(this, cmd);
//         cmd2->setDataItem(item);
//         cmd2->setData("timezone", newTimeZone);
//     }

//     QColor newColour = d.newColour();
//     //qDebug() << "new col" << newColour;
//     if (newColour!=item->metadata("color").value<QColor>())
//     {							// new colour is applicable
//         qDebug() << "change colour" << item->metadata("color") << "->" << newColour;
//         ChangeItemDataCommand *cmd4 = new ChangeItemDataCommand(this, cmd);
//         cmd4->setDataItem(item);
//         cmd4->setData("color", newColour);
//     }

    // QString newType = d.newTrackType();
    // //qDebug() << "new type" << newType;
    // if (newType!="-" && newType!=item->metadata("type").toString())
    // {							// new type is applicable
        // qDebug() << "change type" << item->metadata("type") << "->" << newType;
        // ChangeItemDataCommand *cmd4 = new ChangeItemDataCommand(this, cmd);
        // cmd4->setDataItem(item);
        // cmd4->setData("type", newType);
    // }

    // QString newDesc = d.newItemDesc();
    // //qDebug() << "new description" << newDesc;
    // if (newDesc!="-" && newDesc!=item->metadata("desc").toString())
    // {							// new description is applicable
        // qDebug() << "change desc" << item->metadata("desc") << "->" << newDesc;
        // ChangeItemDataCommand *cmd5 = new ChangeItemDataCommand(this, cmd);
        // cmd5->setDataItem(item);
        // cmd5->setData("desc", newDesc);
    // }

    QString newBearingData = d.newBearingData();
    if (newBearingData!="-" && newBearingData!=item->metadata("bearingline").toString())
    {							// new data is applicable
        qDebug() << "change brg line" << item->metadata("bearingline") << "->" << newBearingData;
        ChangeItemDataCommand *cmd8 = new ChangeItemDataCommand(this, cmd);
        cmd8->setDataItem(item);
        cmd8->setData("bearingline", newBearingData);
    }

    QString newRangeData = d.newRangeData();
    if (newRangeData!="-" && newRangeData!=item->metadata("rangering").toString())
    {							// new data is applicable
        qDebug() << "change range ring" << item->metadata("rangering") << "->" << newRangeData;
        ChangeItemDataCommand *cmd9 = new ChangeItemDataCommand(this, cmd);
        cmd9->setDataItem(item);
        cmd9->setData("rangering", newRangeData);
    }

    // Point position
    const QVariant &latData = model->data(DataIndexer::self()->index("latitude"));
    const QVariant &lonData = model->data(DataIndexer::self()->index("longitude"));
    if (!latData.isNull() && !lonData.isNull())		// if applies to this point
    {
        TrackDataAbstractPoint *tdp = dynamic_cast<TrackDataAbstractPoint *>(item);
        Q_ASSERT(tdp!=nullptr);

        const double newLat = latData.toDouble();
        const double newLon = lonData.toDouble();

        if (newLat!=tdp->latitude() || newLon!=tdp->longitude())
        {
            qDebug() << "change position" << newLat << newLon;
            MovePointsCommand *cmd6 = new MovePointsCommand(this, cmd);
            cmd6->setDataItems((QList<TrackDataItem *>() << tdp));
            cmd6->setData(newLat-tdp->latitude(), newLon-tdp->longitude());
        }
    }

    // The remaining metadata
    const int num = model->rowCount();			// same as DataIndexer::self()->count()
    for (int idx = 0; idx<num; ++idx)
    {
        const QString name = DataIndexer::self()->name(idx);
        if (name=="name") continue;			// these handled specially above
        if (name=="latitude" || name=="longitude") continue;

        const QVariant oldData = item->metadata(idx);
        QVariant newData = model->data(idx);
        if (newData==oldData) continue;			// data has not changed

        qDebug() << "index" << idx << name << oldData << "->" << newData;

        if (name=="status")				// changing waypoint status
        {						// ignore if "No change"
            const TrackData::WaypointStatus status = static_cast<TrackData::WaypointStatus>(newData.toInt());
            if (status==TrackData::StatusInvalid) continue;
        }
        else if (name=="color")				// changing item colour
        {
            // Alpha value encodes the inherit flag, see TrackItemStylePage
            QColor col = newData.value<QColor>();
            if (col.alpha()==0) newData = QVariant();	// here null colour means inherit
            qDebug() << "index" << idx << name << oldData << "->" << newData;
        }

        ChangeItemDataCommand *cmd7 = new ChangeItemDataCommand(this, cmd);
        cmd7->setDataItems(items);
        ////////////////////////////////////////////////////////////////////////////////
        // TODO: have setData() take an index
        cmd7->setData(DataIndexer::self()->name(idx), newData);
    }




    if (cmd->childCount()==0)				// anything to actually do?
    {							// no changes above, so
        delete cmd;					// don't need this after all
        return;
    }

    cmd->setText(actText);
    mainWindow()->executeCommand(cmd);
}


void FilesController::slotSetWaypointStatus()
{
    QAction *act = qobject_cast<QAction *>(sender());
    if (act==NULL) return;
    int newStatus = act->data().toInt();

    ChangeItemDataCommand *cmd = new ChangeItemDataCommand(this);
    cmd->setText(i18n("Waypoint Status %1", CommandBase::senderText(sender())));
    cmd->setDataItems(view()->selectedItems());
    cmd->setData("status", (newStatus==0) ? "" : QString::number(newStatus));
    mainWindow()->executeCommand(cmd);
}


void FilesController::slotSplitSegment()
{
    QList<TrackDataItem *> items = view()->selectedItems();
    if (items.count()!=1) return;
    const TrackDataItem *item = items.first();

    TrackDataSegment *pnt = dynamic_cast<TrackDataSegment *>(item->parent());
    if (pnt==NULL) return;
    qDebug() << "split" << pnt->name() << "at" << item->name();

    int idx = pnt->childIndex(item);
    if (idx==0 || idx>=(pnt->childCount()-1))
    {
        KMessageBox::sorry(mainWindow(),
                           xi18nc("@info", "Cannot split the segment here<nl/>(at its start or end point)"),
                           i18n("Cannot split segment"));
        return;
    }

    SplitSegmentCommand *cmd = new SplitSegmentCommand(this);
    cmd->setSenderText(sender());
    cmd->setData(pnt, idx);
    mainWindow()->executeCommand(cmd);
}



static bool compareSegmentTimes(const TrackDataItem *item1, const TrackDataItem *item2)
{
    if (item1->childCount()==0) return (true);		// empty always sorts first
    if (item2->childCount()==0) return (false);

    const TrackDataAbstractPoint *pnt1 = dynamic_cast<TrackDataAbstractPoint *>(item1->childAt(0));
    Q_ASSERT(pnt1!=NULL);
    const TrackDataAbstractPoint *pnt2 = dynamic_cast<TrackDataAbstractPoint *>(item2->childAt(0));
    Q_ASSERT(pnt2!=NULL);

    return (pnt1->time()<pnt2->time());
}


void FilesController::slotMergeSegments()
{
    // There may be more than two segments selected.  They will all be merged
    // in time order, which must still result in a strict monotonic ordering
    // of the segment points.  In other words, the master segment will be the
    // one with the earliest start time and each segment merged into it in turn
    // must have a start time after the current end of the master.

    QList<TrackDataItem *> items = view()->selectedItems();
    if (items.count()<2) return;
    std::sort(items.begin(), items.end(), &compareSegmentTimes);

    qDebug() << "sorted segments:";
    QDateTime prevEnd;
    for (int i = 0; i<items.count(); ++i)
    {
        const TrackDataSegment *tds = dynamic_cast<const TrackDataSegment *>(items[i]);
        const TrackDataTrackpoint *pnt1 = dynamic_cast<TrackDataTrackpoint *>(tds->childAt(0));
        const TrackDataTrackpoint *pnt2 = dynamic_cast<TrackDataTrackpoint *>(tds->childAt(tds->childCount()-1));
        qDebug() << "  " << tds->name() << "start" << pnt1->formattedTime() << "end" << pnt2->formattedTime();

        if (i>0 && pnt1->time()<prevEnd)		// check no time overlap
        {						// all apart from first
            KMessageBox::sorry(mainWindow(), xi18nc("@info", "Cannot merge these segments<nl/><nl/>Start time of segment \"%1\"<nl/>overlaps the previous \"%2\"",
                                                  tds->name(), items[i-1]->name()),
                               i18n("Cannot merge segments"));
            return;
        }

        prevEnd = pnt2->time();				// note end for next time
    }

    TrackDataSegment *masterSeg = dynamic_cast<TrackDataSegment *>(items.takeFirst());

    MergeSegmentsCommand *cmd = new MergeSegmentsCommand(this);
    cmd->setSenderText(sender());
    cmd->setData(masterSeg, items);
    mainWindow()->executeCommand(cmd);
}


void FilesController::slotMoveItem()
{
    QList<TrackDataItem *> items = view()->selectedItems();
    //if (items.count()!=1) return;
    const TrackDataItem *item = items.first();

    MoveItemDialogue d(view());
    d.setSource(&items);

    QString capt;
    if (dynamic_cast<const TrackDataSegment *>(item)!=NULL) capt = i18nc("@title:window", "Move Segment");
    else if (dynamic_cast<const TrackDataFolder *>(item)!=NULL) capt = i18nc("@title:window", "Move Folder");
    else if (dynamic_cast<const TrackDataWaypoint *>(item)!=NULL) capt = i18nc("@title:window", "Move Waypoint");
    if (!capt.isEmpty()) d.setWindowTitle(capt);

    if (!d.exec()) return;

    TrackDataItem *dest = d.selectedItem();
    MoveItemCommand *cmd = new MoveItemCommand(this);
    cmd->setSenderText(sender());
    cmd->setData(items, dest);
    mainWindow()->executeCommand(cmd);
}


void FilesController::slotAddTrack()
{
    QList<TrackDataItem *> items = view()->selectedItems();
    if (items.count()!=1) return;
    TrackDataItem *pnt = items.first();			// parent item (must be file)
    Q_ASSERT(dynamic_cast<TrackDataFile *>(pnt)!=NULL);

    AddContainerCommand *cmd = new AddContainerCommand(this);
    cmd->setSenderText(sender());
    cmd->setData(TrackData::Track);
    mainWindow()->executeCommand(cmd);
}


void FilesController::slotAddRoute()
{
    QList<TrackDataItem *> items = view()->selectedItems();
    if (items.count()!=1) return;
    TrackDataItem *pnt = items.first();			// parent item (must be file)
    Q_ASSERT(dynamic_cast<TrackDataFile *>(pnt)!=NULL);

    AddContainerCommand *cmd = new AddContainerCommand(this);
    cmd->setSenderText(sender());
    cmd->setData(TrackData::Route);
    mainWindow()->executeCommand(cmd);
}


void FilesController::slotAddFolder()
{
    QList<TrackDataItem *> items = view()->selectedItems();
    if (items.count()!=1) return;
    TrackDataItem *pnt = items.first();			// parent item (file or folder)
    Q_ASSERT(dynamic_cast<TrackDataFile *>(pnt)!=NULL || dynamic_cast<TrackDataFolder *>(pnt)!=NULL);

    AddContainerCommand *cmd = new AddContainerCommand(this);
    cmd->setSenderText(sender());
    cmd->setData(TrackData::Folder, pnt);
    mainWindow()->executeCommand(cmd);
}


void FilesController::slotAddPoint()
{
    QList<TrackDataItem *> items = view()->selectedItems();
    if (items.count()!=1) return;

    AddPointCommand *cmd = new AddPointCommand(this);
    cmd->setSenderText(sender());
    cmd->setData(items.first());
    mainWindow()->executeCommand(cmd);
}




void FilesController::slotDeleteItems()
{
    QList<TrackDataItem *> items = view()->selectedItems();
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

    int result = KMessageBox::warningContinueCancel(mainWindow(), query,
                                                    i18n("Confirm Delete"),
                                                    KStandardGuiItem::del());
    if (result!=KMessageBox::Continue) return;

    DeleteItemsCommand *cmd = new DeleteItemsCommand(this);
    cmd->setSenderText(sender());
    cmd->setData(items);
    mainWindow()->executeCommand(cmd);
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

    CreatePointDialogue d(this, false);			// waypoint mode
    if (!d.canCreate())
    {
        KMessageBox::sorry(mainWindow(),
                           i18n("There are no folders where a waypoint can be created."),
                           i18n("Cannot Create Waypoint"));
        return;
    }

    QList<TrackDataItem *> items = view()->selectedItems();
    const TrackDataAbstractPoint *selPoint = NULL;

    if (!ISNAN(lat)) d.setSourceLatLong(lat, lon);	// coordinates supplied
    else if (items.count()==1)				// there is a selected item
    {							// a source point?
        selPoint = dynamic_cast<const TrackDataAbstractPoint *>(items.first());
        if (selPoint!=NULL) d.setSourcePoint(selPoint);
							// the destination folder?
        const TrackDataFolder *selFolder = dynamic_cast<const TrackDataFolder *>(items.first());
        if (selFolder!=NULL) d.setDestinationContainer(selFolder);
    }

    if (!d.exec()) return;

    d.pointPosition(&lat, &lon);
    const QString name = d.pointName();
    TrackDataFolder *destFolder = dynamic_cast<TrackDataFolder *>(d.selectedContainer());
    Q_ASSERT(destFolder!=NULL);

    qDebug() << "create" << name << "in" << destFolder->name() << "at" << lat << lon;

    AddWaypointCommand *cmd = new AddWaypointCommand(this);

    QObject *sdr = sender();				// may be called directly
    if (qobject_cast<MapView *>(sdr)!=NULL) cmd->setText(i18n("Create Waypoint on Map"));
    else cmd->setSenderText(sdr);

    cmd->setData(name, lat, lon, destFolder, selPoint);
    mainWindow()->executeCommand(cmd);
}


void FilesController::slotAddRoutepoint(qreal lat, qreal lon)
{
    CreatePointDialogue d(this, true);			// route point mode
    if (!d.canCreate())
    {
        KMessageBox::sorry(mainWindow(),
                           i18n("There are no routes where a point can be created."),
                           i18n("Cannot Create Routepoint"));
        return;
    }

    QList<TrackDataItem *> items = view()->selectedItems();
    const TrackDataAbstractPoint *selPoint = NULL;

    if (!ISNAN(lat)) d.setSourceLatLong(lat, lon);	// coordinates supplied
    else if (items.count()==1)				// there is a selected item
    {							// a source point?
        selPoint = dynamic_cast<const TrackDataAbstractPoint *>(items.first());
        if (selPoint!=NULL) d.setSourcePoint(selPoint);
							// the destination route?
        const TrackDataRoute *selRoute = dynamic_cast<const TrackDataRoute *>(items.first());
        if (selRoute!=NULL) d.setDestinationContainer(selRoute);
    }

    if (!d.exec()) return;

    d.pointPosition(&lat, &lon);
    const QString name = d.pointName();
    TrackDataRoute *destRoute = dynamic_cast<TrackDataRoute *>(d.selectedContainer());
    Q_ASSERT(destRoute!=NULL);

    qDebug() << "create" << name << "in" << destRoute->name() << "at" << lat << lon;

    AddRoutepointCommand *cmd = new AddRoutepointCommand(this);

    QObject *sdr = sender();				// may be called directly
    if (qobject_cast<MapView *>(sdr)!=NULL) cmd->setText(i18n("Create Route Point on Map"));
    else cmd->setSenderText(sdr);

    cmd->setData(name, lat, lon, destRoute, selPoint);
    mainWindow()->executeCommand(cmd);
}


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
