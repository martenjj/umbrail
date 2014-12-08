
#include "filescontroller.h"

#include <qundostack.h>
#include <qaction.h>
#include <qscopedpointer.h>
#ifdef SORTABLE_VIEW
#include <qsortfilterproxymodel.h>
#endif

#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kmimetype.h>
#include <kurl.h>

#include "filesmodel.h"
#include "filesview.h"
#include "commands.h"
#include "gpximporter.h"
#include "gpxexporter.h"
#include "mainwindow.h"
#include "trackpropertiesdialogue.h"
#include "movesegmentdialogue.h"
#include "style.h"

#define GROUP_FILES		"Files"



FilesController::FilesController(QObject *pnt)
    : QObject(pnt)
{
    kDebug();

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
}


FilesController::~FilesController()
{
    kDebug() << "done";
}


void FilesController::clear()
{
    kDebug();
    mDataModel->clear();
}


void FilesController::readProperties()
{
    view()->readProperties();
}

void FilesController::saveProperties()
{
    view()->saveProperties();
}



void FilesController::reportFileError(bool saving, const QString &msg)
{
    KMessageBox::sorry(mainWindow(), msg, (saving ? i18n("File Save Error") : i18n("File Load Error")));
    emit statusMessage(saving ? i18n("Error saving file") : i18n("Error loading file"));
}



bool FilesController::importFile(const KUrl &importFrom)
{
    if (!importFrom.isValid()) return (false);
    QString importType = KMimeType::extractKnownExtension(importFrom.path());
    if (importType.isEmpty())
    {
        QString fileName = importFrom.fileName();
        int i = fileName.lastIndexOf('.');
        if (i>0) importType = fileName.mid(i+1);
    }

    importType = importType.toUpper();
    kDebug() << "from" << importFrom << "type" << importType;

    QScopedPointer<ImporterBase> imp;			// importer for requested format
    if (importType=="GPX")				// import from GPX file
    {
        imp.reset(new GpxImporter);
    }

    if (imp.isNull())					// could not create importer
    {
        kDebug() << "Unknown import format" << importType;
        reportFileError(false, i18n("<qt>Unknown import format for<br><filename>%1</filename>",
                                    importFrom.pathOrUrl()));
        return (false);
    }

    emit statusMessage(i18n("Loading %1 from <filename>%2</filename>...", importType, importFrom.pathOrUrl()));
    TrackDataFile *tdf = imp->load(importFrom);		// do the import
    if (tdf==NULL)
    {
        reportFileError(false, i18n("<qt>Cannot import %1 from<br><filename>%3</filename><br><br>%2",
                                    importType, imp->lastError(), importFrom.pathOrUrl()));
        return (false);
    }

    if (model()->isEmpty())				// any data held so far?
    {							// no, pass to model directly
        model()->addToplevelItem(tdf);			// takes ownership of tracks
        delete tdf;					// not needed any more
        emit statusMessage(i18n("<qt>Loaded <filename>%1</filename>", importFrom.pathOrUrl()));
    }
    else						// model is not empty,
    {							// make the operation undo'able
        ImportFileCommand *cmd = new ImportFileCommand(this);
        cmd->setText(i18n("Import"));
        cmd->setData(tdf);				// takes ownership of tree
        mainWindow()->executeCommand(cmd);

        emit statusMessage(i18n("<qt>Imported <filename>%1</filename>", importFrom.pathOrUrl()));
    }

    emit modified();
    return (true);					// done, finished with importer
}



bool FilesController::exportFile(const KUrl &exportTo, const TrackDataFile *tdf)
{
    if (!exportTo.isValid()) return (false);
    QString exportType = KMimeType::extractKnownExtension(exportTo.path());
    if (exportType.isEmpty())
    {
        QString fileName = exportTo.fileName();
        int i = fileName.lastIndexOf('.');
        if (i>0) exportType = fileName.mid(i+1);
    }

    exportType = exportType.toUpper();
    kDebug() << "to" << exportTo << "type" << exportType;

    QScopedPointer<ExporterBase> exp;			// exporter for requested format
    if (exportType=="GPX")				// export to GPX file
    {
        exp.reset(new GpxExporter);
    }

    if (exp.isNull())					// could not create exporter
    {
        kDebug() << "Unknown export format" << exportType;
        reportFileError(true, i18n("<qt>Unknown export format for<br><filename>%1</filename>",
                                   exportTo.pathOrUrl()));
        return (false);
    }

    if (exportTo.isLocalFile() && QFile::exists(exportTo.path()))
    {							// to a local file?
        KUrl backupFile = exportTo;			// make path for backup file
        backupFile.setFileName(exportTo.fileName()+".orig");
// TODO: use KIO
// ask whether to backup if a remote file (if cannot test for exists)
        if (!QFile::exists(backupFile.path()))		// no backup already?
        {
            if (!QFile::copy(exportTo.path(), backupFile.path()))
            {
                reportFileError(true, i18n("<qt>Cannot save original<br>of <filename>%1</filename><br>as <filename>%2</filename>",
                                           exportTo.pathOrUrl(), backupFile.pathOrUrl()));
                emit statusMessage(i18n("Backup failed"));
                return (false);
            }

            KMessageBox::information(mainWindow(),
                                     i18n("<qt>Original of<br><filename>%1</filename><br>has been backed up as<br><filename>%2</filename>",
                                          exportTo.pathOrUrl(), backupFile.pathOrUrl()),
                                     i18n("Original file backed up"),
                                     "fileBackupInfo");
        }
    }

    emit statusMessage(i18n("Saving %1 to <filename>%2</filename>...", exportType, exportTo.pathOrUrl()));
    if (!exp->save(exportTo, tdf))
    {
        reportFileError(true, i18n("<qt>Cannot save %1 to<br><filename>%3</filename><br><br>%2",
                             exportType, exp->lastError(), exportTo.pathOrUrl()));
        return (false);
    }

    emit statusMessage(i18n("<qt>Exported <filename>%1</filename>", exportTo.pathOrUrl()));
    return (true);					// done, finished with exporter
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
        QString msg = QString::null;

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

case TrackData::Point:      if (selCount==1) msg = i18n("Selected point '%1'", name);
                            else msg = i18np("Selected %1 point", "Selected %1 points", selCount);
                            break;

case TrackData::Waypoint:   if (selCount==1) msg = i18n("Selected waypoint '%1'", name);
                            else msg = i18np("Selected %1 waypoint", "Selected %1 waypoints", selCount);
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



void FilesController::slotTrackProperties()
{
    kDebug();

    QList<TrackDataItem *> items = view()->selectedItems();
    if (items.count()==0) return;

    TrackPropertiesDialogue d(items, view());
    QString actText = CommandBase::senderText(sender());
    d.setCaption(actText);

    if (!d.exec()) return;

    TrackDataItem *item = items.first();
    QUndoCommand *cmd = new QUndoCommand();		// parent command

    QString newItemName = d.newItemName();		// new name for item
    kDebug() << "new name" << newItemName;
    if (!newItemName.isEmpty() && newItemName!=item->name())
    {							// changing the name
        ChangeItemNameCommand *cmd1 = new ChangeItemNameCommand(this, cmd);
        cmd1->setDataItem(item);
        cmd1->setData(newItemName);
    }

    QString newTimeZone = d.newTimeZone();
    kDebug() << "new timezone" << newTimeZone;
    if (newTimeZone!=item->metadata("timezone"))
    {
        ChangeItemDataCommand *cmd2 = new ChangeItemDataCommand(this, cmd);
        cmd2->setDataItem(item);
        cmd2->setData("timezone", newTimeZone);
    }

    const Style newStyle = d.newStyle();		// item style
    kDebug() << "new style" << newStyle;
    if (newStyle!=*item->style())			// changing the style
    {
        ChangeItemStyleCommand *cmd3 = new ChangeItemStyleCommand(this, cmd);
        cmd3->setDataItem(item);
        cmd3->setData(newStyle);
    }

    QString newType = d.newTrackType();
    if (newType!="-")					// new type is applicable
    {
        kDebug() << "new type" << newType;
        if (newType!=item->metadata("type"))
        {
            ChangeItemDataCommand *cmd4 = new ChangeItemDataCommand(this, cmd);
            cmd4->setDataItem(item);
            cmd4->setData("type", newType);
        }
    }

    QString newDesc = d.newItemDesc();
    if (newDesc!="-")					// new description is applicable
    {
        kDebug() << "new description" << newDesc;
        if (newDesc!=item->metadata("desc"))
        {
            ChangeItemDataCommand *cmd5 = new ChangeItemDataCommand(this, cmd);
            cmd5->setDataItem(item);
            cmd5->setData("desc", newDesc);
        }
    }

    double newLat;
    double newLon;
    if (d.newPointPosition(&newLat, &newLon))
    {
        TrackDataAbstractPoint *p = dynamic_cast<TrackDataAbstractPoint *>(item);
        Q_ASSERT(p!=NULL);
        MovePointsCommand *cmd6 = new MovePointsCommand(this, cmd);
        cmd6->setDataItems((QList<TrackDataItem *>() << p));
        cmd6->setData(newLat-p->latitude(), newLon-p->longitude());
    }

    if (cmd->childCount()==0)				// anything to actually do?
    {							// no changes above, so
        delete cmd;					// don't need this after all
        return;
    }

    cmd->setText(actText);
    mainWindow()->executeCommand(cmd);
}


void FilesController::slotSplitSegment()
{
    QList<TrackDataItem *> items = view()->selectedItems();
    if (items.count()!=1) return;
    const TrackDataItem *item = items.first();

    TrackDataSegment *pnt = dynamic_cast<TrackDataSegment *>(item->parent());
    if (pnt==NULL) return;
    kDebug() << "split" << pnt->name() << "at" << item->name();

    int idx = pnt->childIndex(item);
    if (idx==0 || idx>=(pnt->childCount()-1))
    {
        KMessageBox::sorry(mainWindow(),
                           i18n("<qt>Cannot split the segment here<br>(at its start or end point)"),
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
    qSort(items.begin(), items.end(), &compareSegmentTimes);

    kDebug() << "sorted segments:";
    QDateTime prevEnd;
    for (int i = 0; i<items.count(); ++i)
    {
        const TrackDataSegment *tds = dynamic_cast<const TrackDataSegment *>(items[i]);
        const TrackDataTrackpoint *pnt1 = dynamic_cast<TrackDataTrackpoint *>(tds->childAt(0));
        const TrackDataTrackpoint *pnt2 = dynamic_cast<TrackDataTrackpoint *>(tds->childAt(tds->childCount()-1));
        kDebug() << "  " << tds->name() << "start" << pnt1->formattedTime() << "end" << pnt2->formattedTime();

        if (i>0 && pnt1->time()<prevEnd)		// check no time overlap
        {						// all apart from first
            KMessageBox::sorry(mainWindow(), i18n("<qt>Cannot merge these segments<p>Start time of segment \"%1\"<br>overlaps the previous \"%2\"",
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



void FilesController::slotMoveSegment()
{
    QList<TrackDataItem *> items = view()->selectedItems();
    if (items.count()!=1) return;
    TrackDataSegment *tds = dynamic_cast<TrackDataSegment *>(items.first());
    Q_ASSERT(tds!=NULL);

    MoveSegmentDialogue d(this, view());
    d.setSegment(tds);

    if (!d.exec()) return;

    TrackDataTrack *tdt = d.selectedTrack();

    TrackDataTrack *sourceTrack = dynamic_cast<TrackDataTrack *>(tds->parent());
    if (tdt==sourceTrack)
    {
        KMessageBox::sorry(mainWindow(), i18n("<qt>Segment \"%1\" is already part of track \"%2\"",
                                              tds->name(), tdt->name()),
                           i18n("Cannot move track"));
        return;
    }

    MoveSegmentCommand *cmd = new MoveSegmentCommand(this);
    cmd->setSenderText(sender());
    cmd->setData(tds, tdt);
    mainWindow()->executeCommand(cmd);
}



void FilesController::slotAddTrack()
{
    AddTrackCommand *cmd = new AddTrackCommand(this);
    cmd->setSenderText(sender());
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
            query = i18n("Delete the selected item \"%1\"?", tdi->name());
        }
        else
        {
            query = i18n("<qt>Delete the selected item \"%1\"<br>and everything under it?", tdi->name());
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












MainWindow *FilesController::mainWindow() const
{
    return (qobject_cast<MainWindow *>(parent()));
}



//////////////////////////////////////////////////////////////////////////

static const char allFilter[] = "*|All Files";


QString FilesController::allExportFilters()
{
    QStringList filters;
    filters << GpxExporter::filter();
    return (filters.join("\n"));
}



QString FilesController::allImportFilters()
{
    QStringList filters;
    filters << GpxImporter::filter();
    filters << allFilter;
    return (filters.join("\n"));
}



QString FilesController::allProjectFilters(bool includeAllFiles)
{
    QStringList filters;
    filters << GpxImporter::filter();
    if (includeAllFiles) filters << allFilter;
    return (filters.join("\n"));
}
