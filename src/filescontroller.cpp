
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
        model()->addFile(tdf);				// takes ownership of tracks
        delete tdf;					// not needed any more
        emit statusMessage(i18n("<qt>Loaded <filename>%1</filename>", importFrom.pathOrUrl()));
    }
    else						// model is not empty,
    {							// make the operation undo'able
        ImportFileCommand *cmd = new ImportFileCommand(this);
        cmd->setText(i18n("Import"));
        cmd->setTrackData(tdf);				// takes ownership of tree
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
        QString name = tdi->name();

        switch (selType)
        {
case TrackData::File:       if (selCount==1) emit statusMessage(i18n("Selected file '%1'", name));
                            else emit statusMessage(i18np("Selected %1 file", "Selected %1 files", selCount));
                            break;

case TrackData::Track:      if (selCount==1) emit statusMessage(i18n("Selected track '%1'", name));
                            else emit statusMessage(i18np("Selected %1 track", "Selected %1 tracks", selCount));
                            break;

case TrackData::Segment:    if (selCount==1) emit statusMessage(i18n("Selected segment '%1'", name));
                            else emit statusMessage(i18np("Selected %1 segment", "Selected %1 segments", selCount));
                            break;

case TrackData::Point:      if (selCount==1) emit statusMessage(i18n("Selected point '%1'", name));
                            else emit statusMessage(i18np("Selected %1 point", "Selected %1 points", selCount));
                            break;
        }
    }

    emit updateActionState();
}



////////////////////////////////////////////////////////////////////////////
//
//void FilesController::slotDeleteSelection()
//{
//    FilesView::RowList rows = view()->selectedRows();
//    int cnt = rows.count();
//    if (cnt==0) return;
//    kDebug() << cnt << "rows";
//
//    QString name = model()->pointAt(rows.first())->name();
//
//    QString query;
//    if (cnt==1)
//    {
//        query = i18n("Delete the selected point '%1'?", name);
//        emit statusMessage(i18n("Deleting point '%1'", name));
//    }
//    else
//    {
//        query = i18n("Delete the %1 selected files?", cnt);
//        emit statusMessage(i18n("Deleting %1 files", cnt));
//    }
//        
//    if (KMessageBox::warningContinueCancel(mainWindow(), query,
//                                           i18n("Delete Files"),
//                                           KStandardGuiItem::del())!=KMessageBox::Continue)
//    {
//        emit statusMessage(i18n("Delete cancelled"));
//        return;
//    }
//
//    DeleteFilesCommand *cmd = new DeleteFilesCommand(this);
//    cmd->setText(i18np("Delete point", "Delete %1 files", cnt));
//    cmd->setRowList(rows);
//    executeCommand(cmd);
//
//    slotUpdateActionState();
//    if (cnt==1) emit statusMessage(i18n("Deleted point '%1'", name));
//    else emit statusMessage(i18n("Deleted %1 files", cnt));
//}
//


void FilesController::slotTrackProperties()
{
    kDebug();

    QList<TrackDataItem *> items = view()->selectedItems();
    if (items.count()==0) return;

    TrackPropertiesDialogue d(items, view());

    // Using the action's text() directly will include any '&' accelerators
    // if they have been added.  So we use the original text, which will have
    // been stored in the action's data(), instead.
    QAction *act = static_cast<QAction *>(sender());
    if (act!=NULL) d.setCaption(act->data().toString());

    if (!d.exec()) return;

    TrackDataItem *item = items.first();
    TrackDataDisplayable *dispItem = dynamic_cast<TrackDataDisplayable *>(item);
    Q_ASSERT(dispItem!=NULL);

    QUndoCommand *cmd = new QUndoCommand();		// parent command

    QString newItemName = d.newItemName();		// new name for item
    kDebug() << "new name" << newItemName;
    if (!newItemName.isEmpty() && newItemName!=item->name())
    {							// changing the name
        ChangeItemNameCommand *cmd1 = new ChangeItemNameCommand(this, cmd);
        cmd1->setDataItem(dispItem);
        cmd1->setNewName(newItemName);
    }

    QString newTimeZone = d.newTimeZone();
    kDebug() << "new timezone" << newTimeZone;
    if (newTimeZone!=dispItem->metadata("timezone"))
    {
        ChangeItemDataCommand *cmd2 = new ChangeItemDataCommand(this, cmd);
        cmd2->setDataItem(dispItem);
        cmd2->setNewData("timezone", newTimeZone);
    }

    const Style newStyle = d.newStyle();		// item style
    kDebug() << "new style" << newStyle;
    if (newStyle!=*dispItem->style())			// changing the style
    {
        ChangeItemStyleCommand *cmd3 = new ChangeItemStyleCommand(this, cmd);
        cmd3->setDataItem(dispItem);
        cmd3->setNewStyle(newStyle);
    }

    QString newType = d.newTrackType();
    if (newType!="-")					// new type is applicable
    {
        kDebug() << "new type" << newType;
        if (newType!=dispItem->metadata("type"))
        {
            ChangeItemDataCommand *cmd4 = new ChangeItemDataCommand(this, cmd);
            cmd4->setDataItem(dispItem);
            cmd4->setNewData("type", newType);
        }
    }

    QString newDesc = d.newItemDesc();
    if (newDesc!="-")					// new description is applicable
    {
        kDebug() << "new description" << newDesc;
        if (newDesc!=dispItem->metadata("desc"))
        {
            ChangeItemDataCommand *cmd5 = new ChangeItemDataCommand(this, cmd);
            cmd5->setDataItem(dispItem);
            cmd5->setNewData("desc", newDesc);
        }
    }

    if (cmd->childCount()==0)				// anything to actually do?
    {							// no changes above, so
        delete cmd;					// don't need this after all
        return;
    }

    cmd->setText(act!=NULL ? act->data().toString() : i18n("Properties"));
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
