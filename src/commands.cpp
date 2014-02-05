
#include "commands.h"

#include <kdebug.h>

#include "filesmodel.h"
//#include "pointsview.h"



ImportFileCommand::ImportFileCommand(FilesController *fc, QUndoCommand *parent)
    : FilesCommandBase(fc, parent)
{
    mTrackData = NULL;					// nothing held at present
}


ImportFileCommand::~ImportFileCommand()
{
    if (mTrackData!=NULL) delete mTrackData;		// delete if we hold data
}


void ImportFileCommand::redo()
{
    TrackDataFile *tdf = mTrackData;
    Q_ASSERT(tdf!=NULL);
    kDebug() << "file" << tdf->name();

    model()->addFile(tdf);				// add data tree to model
    mTrackData = NULL;					// model owns it now
}


void ImportFileCommand::undo()
{
    mTrackData = model()->removeFile();			// take ownership of data tree
    kDebug() << "saved" << mTrackData->name();
}





ChangeItemCommand::ChangeItemCommand(FilesController *fc, QUndoCommand *parent)
    : FilesCommandBase(fc, parent)
{
    mDataItem = NULL;					// nothing set at present
}



void ChangeItemCommand::redo()
{
    mFileWasModified = mDataItem->setFileModified(true);
}


void ChangeItemCommand::undo()
{
    mDataItem->setFileModified(mFileWasModified);
}








void ChangeItemNameCommand::redo()
{
    TrackDataItem *item = mDataItem;
    Q_ASSERT(item!=NULL);
    mSavedName = item->name();
    kDebug() << "item" << mSavedName << "->" << mNewName;

    item->setName(mNewName);
    model()->changedItem(item);
    ChangeItemCommand::redo();
}




void ChangeItemNameCommand::undo()
{
    TrackDataItem *item = mDataItem;
    Q_ASSERT(item!=NULL);
    kDebug() << "item" << item->name() << "back to" << mSavedName;

    item->setName(mSavedName);
    model()->changedItem(item);
    ChangeItemCommand::undo();
}










void ChangeFileUrlCommand::redo()
{
    TrackDataFile *item = dynamic_cast<TrackDataFile *>(mDataItem);
    Q_ASSERT(item!=NULL);
    mSavedUrl = item->fileName();
    kDebug() << "item" << mSavedUrl << "->" << mNewUrl;

    item->setFileName(mNewUrl);
    item->setName(mNewUrl.fileName());
    model()->changedItem(item);
    ChangeItemCommand::redo();
}





void ChangeFileUrlCommand::undo()
{
    TrackDataFile *item = dynamic_cast<TrackDataFile *>(mDataItem);
    Q_ASSERT(item!=NULL);
    kDebug() << "item" << item->fileName() << "back to" << mSavedUrl;

    item->setFileName(mSavedUrl);
    item->setName(mSavedUrl.fileName());
    model()->changedItem(item);
    ChangeItemCommand::undo();
}



