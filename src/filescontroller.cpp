
#include "filescontroller.h"

#include <qundostack.h>
#include <qaction.h>
#include <qscopedpointer.h>
#ifdef SORTABLE_VIEW
#include <qsortfilterproxymodel.h>
#endif

#include <kdebug.h>
#include <klocale.h>
#include <kconfiggroup.h>
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


void FilesController::readProperties(const KConfigGroup &grp)
{
    view()->readProperties(grp);
//    iconsManager()->readProperties(grp);
}

void FilesController::saveProperties(KConfigGroup &grp)
{
    view()->saveProperties(grp);
//    iconsManager()->saveProperties(grp);
}

//
//ImporterBase *FilesController::createGpxImporter()
//{
//    GpxImporter *imp = new GpxImporter(mainWindow());
//    connect(imp, SIGNAL(finished(const FilesList *)),
//            SLOT(slotImportFinished(const FilesList *)));
//    connect(imp, SIGNAL(statusMessage(const QString &)),
//            this, SIGNAL(statusMessage(const QString &)));
//    return (imp);
//}
//
//
//ExporterBase *FilesController::createGpxExporter()
//{
//    GpxExporter *exp = new GpxExporter(mainWindow());
//    connect(exp, SIGNAL(statusMessage(const QString &)),
//            this, SIGNAL(statusMessage(const QString &)));
//    return (exp);
//}
//
//
//void FilesController::slotImportFinished(const FilesList *files)
//{
//    int curCount = model()->filesCount();
//    int addedCount = files->count();
//    kDebug() << "cur" << curCount << "added" << addedCount;
//
//    int mergeCount = 0;
//    QList<int> mergeRows;
//    for (int ia = 0; ia<addedCount; ++ia)		// scan over added files
//    {
//        const PointData aPoint = files->at(ia);
//        int mergeRow = -1;
//        for (int ic = 0; ic<curCount; ++ic)		// scan over existing files
//        {
//            const PointData *cPoint = model()->pointAt(ic);
//            bool can = cPoint->canMerge(aPoint);
//            if (can)
//            {
//                mergeRow = ic;
//                ++mergeCount;
//                kDebug() << "merge with row" << ic << cPoint->name();
//                break;
//            }
//        }
//
//        if (mergeRow==-1) kDebug() << "no merge" << aPoint.name();
//        mergeRows.append(mergeRow);
//    }
//
//    MergeFilesCommand *cmd = new MergeFilesCommand(this);
//    cmd->setText(i18np("Import point", "Import %1 files", addedCount));
//    cmd->setFiles(files);
//    cmd->setRows(&mergeRows);
//    executeCommand(cmd);
//
//    slotUpdateActionState();
//    categoryManager()->scanForNew(model());
//    iconsManager()->scanForNew(model());
//    emit statusMessage(i18n("Imported %1 files, new %2, merged %3, total %4",
//                            addedCount,
//                            (addedCount-mergeCount),
//                            mergeCount,
//                            model()->filesCount()));
//}


QString FilesController::save(KConfig *conf)
{
    const int cnt = model()->fileCount();
    kDebug() << "saving" << cnt << "files to" << conf->name();

    KConfigGroup grp = conf->group(GROUP_FILES);
    for (int i = 0; i<cnt; ++i)
    {
        TrackDataFile *tdf = model()->fileAt(i);
        const KUrl file = tdf->fileName();
        kDebug() << "  " << i << "=" << file;
        grp.writeEntry(QString::number(i), file);

        if (tdf->isModified())
        {
            if (!exportFile(tdf->fileName(), tdf))
            {						// detailed error already given
                return (i18n("Save to <filename>%1</filename> failed"));
            }
            // TODO: will this affect undo?
            tdf->setModified(false);
        }
    }

    return (QString::null);
}


QString FilesController::load(const KConfig *conf)
{
    kDebug() << "from" << conf->name();

    KConfigGroup grp = conf->group(GROUP_FILES);
    for (int i = 0; ; ++i)
    {
        KUrl file = grp.readEntry(QString::number(i), KUrl());
        if (!file.isValid()) break;
        kDebug() << "  " << i << "=" << file;
        importFile(file);
    }

    return (QString::null);
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
        return (false);
    }

    TrackDataFile *tdf = imp->load(importFrom);		// do the import
    if (tdf==NULL)
    {
        KMessageBox::sorry(mainWindow(), i18n("<qt>Cannot import %1 from<br><filename>%3</filename><br><br>%2",
                                              importType, imp->lastError(), importFrom.pathOrUrl()));
        emit statusMessage(i18n("Import failed"));
        return (false);
    }
    else
    {
        ImportFileCommand *cmd = new ImportFileCommand(this);
        cmd->setText(i18n("Import"));
        cmd->setTrackData(tdf);
        mainWindow()->executeCommand(cmd);

        emit statusMessage(i18n("<qt>Imported <filename>%1</filename>", importFrom.pathOrUrl()));
        emit modified();
    }

    return (true);					// done, finished with exporter
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
        return (false);
    }

    if (exportTo.isLocalFile())				// to a local file?
    {
        KUrl backupFile = exportTo;			// make path for backup file
        backupFile.setFileName(exportTo.fileName()+".orig");
// TODO: use KIO
// ask whether to backup if a remote file (if cannot test for exists)
        if (!QFile::exists(backupFile.path()))		// no backup already?
        {
            if (!QFile::copy(exportTo.path(), backupFile.path()))
            {
                KMessageBox::sorry(mainWindow(),
                                   i18n("<qt>Cannot save original<br>of<filename>%1</filename><br>as <filename>%2</filename>",
                                        exportTo.pathOrUrl(), backupFile.pathOrUrl()),
                                   i18n("Cannot save original file"));
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

    if (!exp->save(exportTo, tdf))
    {
        KMessageBox::sorry(mainWindow(), i18n("<qt>Cannot export %1 to<br><filename>%3</filename><br><br>%2",
                                     exportType, exp->lastError(), exportTo.pathOrUrl()));
        emit statusMessage(i18n("Export failed"));
        return (false);
    }
    else
    {
        emit statusMessage(i18n("<qt>Exported <filename>%1</filename>", exportTo.pathOrUrl()));
    }

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

    QString newItemName = d.newItemName();		// name of item
    if (!newItemName.isEmpty())				// valid entry?
    {							// has name changed?
        if (newItemName==item->name()) newItemName.clear();
    }							// no, throw entry away

    KUrl newFileUrl = d.newFileUrl();			// URL for file item
    if (newFileUrl.isValid())				// valid entry?
    {
        TrackDataFile *fileItem = dynamic_cast<TrackDataFile *>(item);
        Q_ASSERT(fileItem!=NULL);			// has URL changed?
        if (newFileUrl==fileItem->fileName()) newFileUrl.clear();
    }							// no, throw entry away

    if (newItemName.isEmpty() && !newFileUrl.isValid()) return;
        						// nothing to do
    kDebug() << "new name" << newItemName;
    kDebug() << "new url" << newFileUrl;

    QUndoCommand *cmd = new QUndoCommand();		// parent command
    cmd->setText(act!=NULL ? act->data().toString() : i18n("Properties"));

    if (!newItemName.isEmpty())				// changing the name
    {
        ChangeItemNameCommand *cmd1 = new ChangeItemNameCommand(this, cmd);
        cmd1->setDataItem(item);
        cmd1->setNewName(newItemName);
    }

    if (!newFileUrl.isEmpty())				// changing the URL
    {
        ChangeFileUrlCommand *cmd2 = new ChangeFileUrlCommand(this, cmd);
        cmd2->setDataItem(item);
        cmd2->setNewUrl(newFileUrl);

        KMessageBox::information(mainWindow(),
                                 i18n("<qt>Changing the location of a file will not immediately "
                                      "copy the existing file to the new location. "
                                      "It will be saved to the new location when the project is saved."),
                                 QString::null, "fileMoveInfo");
    }

    mainWindow()->executeCommand(cmd);
}


//
//
//void FilesController::slotNewPoint()
//{
//    kDebug();
//
//    PointPropertiesDialog d(this, mainWindow());
//    d.setCaption(i18n("New Point"));
//
//    emit statusMessage(i18n("Creating new point"));
//    d.show();
//    if (!d.exec())
//    {
//        emit statusMessage(i18n("Create cancelled"));
//        return;
//    }
//
//    const PointData *pnt = d.resultPoint();
//    AddFilesCommand *cmd = new AddFilesCommand(this);
//    cmd->setText(i18n("New point"));
//    cmd->setPoint(pnt);
//    executeCommand(cmd);
//
//    slotUpdateActionState();
//    emit statusMessage(i18n("Created new point '%1'", pnt->name()));
//}
//
//
//
//
//
//
//
//void FilesController::slotMergeSelection()
//{
//    FilesView::RowList rows = view()->selectedRows();
//    int cnt = rows.count();
//    if (cnt<2) return;
//    kDebug() << "rows" << rows;
//
//    emit statusMessage(i18n("Checking positions"));
//
//    FilesList files;					// files to be merged
//    bool mergeOk = true;				// positions close enough?
//    const PointData *refPoint = model()->pointAt(rows.first());
//    files.append(*refPoint);
//    for (int i = 1; i<cnt; ++i)				// check reference against others
//    {
//        int row = rows[i];
//        const PointData *pnt = model()->pointAt(row);
//        files.append(*pnt);
//
//        if (!refPoint->canMerge(*pnt, true))
//        {
//            mergeOk = false;
//        }
//    }
//
//    if (!mergeOk)
//    {
//        // TODO: show the difference distance
//        QString query = i18n("<qt>Some of the selected files are not close together."
//                             "<p>Really merge the %1 files?", cnt);
//        if (KMessageBox::warningContinueCancel(mainWindow(), query,
//                                               i18n("Merge Files"))!=KMessageBox::Continue)
//        {
//            emit statusMessage(i18n("Merge cancelled"));
//            return;
//        }
//    }
//
//    emit statusMessage(i18n("Manual merge"));
//
//    MergeFilesDialogue d(this, mainWindow());
//    d.setFiles(&files);
//    if (!d.exec())
//    {
//        emit statusMessage(i18n("Merge cancelled"));
//        return;
//    }
//
//    CombineFilesCommand *cmd = new CombineFilesCommand(this);
//    cmd->setText(i18np("Merge point", "Merge %1 files", cnt));
//    cmd->setPoint(d.resultPoint());
//    cmd->setRowList(&rows);
//    executeCommand(cmd);
//
//    slotUpdateActionState();
//    emit statusMessage(i18n("Merged %1 files", cnt));
//}
//
//
//
//
//



MainWindow *FilesController::mainWindow() const
{
    return (qobject_cast<MainWindow *>(parent()));
}


QStringList FilesController::modifiedFiles() const
{
    QStringList result;

    const TrackDataItem *root = model()->rootItem();
    int num = root->childCount();
    for (int i = 0; i<num; ++i)
    {
        const TrackDataFile *fileItem = dynamic_cast<const TrackDataFile *>(root->childAt(i));
        if (fileItem!=NULL)				// should always be so
        {
            if (fileItem->isModified()) result.append(fileItem->fileName().pathOrUrl());
        }
    }

    return (result);
}





//////////////////////////////////////////////////////////////////////////

static const char dataFilter[] = "*.tracks|Track project (*.tracks)";
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
    filters << dataFilter;
    if (includeAllFiles) filters << allFilter;
    return (filters.join("\n"));
}
