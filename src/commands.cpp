
#include "commands.h"

#include <qmetaobject.h>

#include <kdebug.h>

#include "filesmodel.h"
#include "style.h"
#include "dataindexer.h"



void FilesCommandBase::updateMap() const
{
    // Cannot emit a signal directly, because we are not a QObject.
    // Ask the FilesController to emit the signal instead.
    QMetaObject::invokeMethod(controller(), "updateMap");
}



ImportFileCommand::ImportFileCommand(FilesController *fc, QUndoCommand *parent)
    : FilesCommandBase(fc, parent)
{
    mTrackData = NULL;					// nothing held at present
    mSavedCount = 0;
}



ImportFileCommand::~ImportFileCommand()
{
    if (mTrackData!=NULL) delete mTrackData;		// delete if we hold data
}



void ImportFileCommand::redo()
{
    Q_ASSERT(mTrackData!=NULL);
    mSavedCount = mTrackData->childCount();		// how many tracks contained
    kDebug() << "file" << mTrackData->name() << "count" << mSavedCount;

    model()->addFile(mTrackData);			// add data tree to model
    Q_ASSERT(mTrackData->childCount()==0);		// should have taken all tracks
							// retain original for undo
    updateMap();
}


void ImportFileCommand::undo()
{
    Q_ASSERT(mTrackData!=NULL);
    for (int i = 0; i<mSavedCount; ++i)			// how many tracks added last time
    {
        TrackDataItem *item = model()->removeLast();	// remove each from model
        if (item==NULL) continue;			// and re-add to saved file item
        mTrackData->addChildItem(item, true);		// in the original order of course
    }

    kDebug() << "saved" << mTrackData->name() << "children" << mTrackData->childCount();
    updateMap();
}





ChangeItemCommand::ChangeItemCommand(FilesController *fc, QUndoCommand *parent)
    : FilesCommandBase(fc, parent)
{
    mDataItem = NULL;					// nothing set at present
}






void ChangeItemNameCommand::redo()
{
    TrackDataItem *item = mDataItem;
    Q_ASSERT(item!=NULL);
    mSavedName = item->name();
    kDebug() << "item" << mSavedName << "->" << mNewName;

    item->setName(mNewName);
    model()->changedItem(item);
}




void ChangeItemNameCommand::undo()
{
    TrackDataItem *item = mDataItem;
    Q_ASSERT(item!=NULL);
    kDebug() << "item" << item->name() << "back to" << mSavedName;

    item->setName(mSavedName);
    model()->changedItem(item);
}




void ChangeItemStyleCommand::redo()
{
    TrackDataDisplayable *item = mDataItem;
    Q_ASSERT(item!=NULL);
    kDebug() << "item" << item->name() << "->" << mNewStyle.toString();

    mSavedStyle = *item->style();			// save original item style
    item->setStyle(mNewStyle);				// set new item style

    model()->changedItem(item);
    updateMap();
}




void ChangeItemStyleCommand::undo()
{
    TrackDataDisplayable *item = mDataItem;
    Q_ASSERT(item!=NULL);
    kDebug() << "item" << item->name() << "back to" << mSavedStyle.toString();

    item->setStyle(mSavedStyle);			// restore original style

    model()->changedItem(item);
    updateMap();
}





void ChangeItemDataCommand::redo()
{
    TrackDataDisplayable *item = mDataItem;
    Q_ASSERT(item!=NULL);
    kDebug() << "item" << item->name() << "data" << mKey << "->" << mNewValue;

    int idx = DataIndexer::self()->index(mKey);
    mSavedValue = item->metadata(idx);
    item->setMetadata(idx, mNewValue);
    model()->changedItem(item);
    //updateMap();
}




void ChangeItemDataCommand::undo()
{
    TrackDataDisplayable *item = mDataItem;
    Q_ASSERT(item!=NULL);
    kDebug() << "item" << item->name() << "data" << mKey << "back to" << mSavedValue;
    int idx = DataIndexer::self()->index(mKey);
    item->setMetadata(idx, mSavedValue);
    model()->changedItem(item);
//    updateMap();
}


