
#include "commands.h"

#include <qmetaobject.h>

#include <kdebug.h>
#include <klocale.h>

#include "filesmodel.h"
#include "style.h"
#include "dataindexer.h"




// These are intended to be used in a destructor to clean up data that
// we may be holding.  If the 'item' has a parent then it means that
// it is part of the main data structure, so it should not be deleted.
// If it has no parent then it is owned by us and can safely be deleted.

template<typename T>
static void deleteData(T *item)
{
    if (item==NULL) return;
    if (item->parent()!=NULL) return;
    kDebug() << "deleting" << item->name();
    delete item;
}


template<typename C>
static void deleteData(const C &list)
{
    for (int i = 0; i<list.count(); ++i) deleteData(list[i]);
}






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
    deleteData(mTrackData);
}



void ImportFileCommand::redo()
{
    Q_ASSERT(mTrackData!=NULL);
    mSavedCount = mTrackData->childCount();		// how many tracks contained
    kDebug() << "file" << mTrackData->name() << "count" << mSavedCount;

    model()->addToplevelItem(mTrackData);		// add data tree to model
    Q_ASSERT(mTrackData->childCount()==0);		// should have taken all tracks
							// retain original for undo
    updateMap();
}


void ImportFileCommand::undo()
{
    Q_ASSERT(mTrackData!=NULL);
    for (int i = 0; i<mSavedCount; ++i)			// how many tracks added last time
    {							// remove each from model
        TrackDataItem *item = model()->removeLastToplevelItem();
        if (item==NULL) continue;			// and re-add to saved file item
        mTrackData->addChildItem(item, 0);		// in the original order of course
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



SplitSegmentCommand::SplitSegmentCommand(FilesController *fc, QUndoCommand *parent)
    : FilesCommandBase(fc, parent)
{
    mParentSegment = NULL;				// nothing set at present
    mSplitIndex = -1;
    mNewSegment = NULL;
}



SplitSegmentCommand::~SplitSegmentCommand()
{
    deleteData(mParentSegment);
    deleteData(mNewSegment);
}





void SplitSegmentCommand::setSplitAt(TrackDataSegment *pnt, int idx)
{
    mParentSegment = pnt;
    mSplitIndex = idx;
}



static QString makeSplitName(const QString &orig)
{
    QString name = orig;
    if (name.contains(' ')) name += i18n(" (split)");
    else name += i18n("_split");
    return (name);
}






void SplitSegmentCommand::redo()
{
    Q_ASSERT(mParentSegment!=NULL);
    Q_ASSERT(mSplitIndex>0 && mSplitIndex<(mParentSegment->childCount()-1));

    mNewSegment = new TrackDataSegment(makeSplitName(mParentSegment->name()));
    mNewSegment->copyMetadata(mParentSegment);
    // TODO: copy style

    TrackDataPoint *splitPoint = dynamic_cast<TrackDataPoint *>(mParentSegment->childAt(mSplitIndex));
    Q_ASSERT(splitPoint!=NULL);

    TrackDataPoint *copyPoint = new TrackDataPoint(makeSplitName(splitPoint->name()));
    copyPoint->copyData(splitPoint);
    copyPoint->copyMetadata(splitPoint);
    // TODO: copy style
    mNewSegment->addChildItem(copyPoint);

    model()->splitItem(mParentSegment, mSplitIndex, mNewSegment);
    updateMap();
}



void SplitSegmentCommand::undo()
{
    Q_ASSERT(mNewSegment!=NULL);
    Q_ASSERT(mParentSegment!=NULL);

    model()->mergeItems(mParentSegment, mNewSegment);
    delete mNewSegment;					// new segment and copied point
    mNewSegment = NULL;
    updateMap();
}





MergeSegmentsCommand::MergeSegmentsCommand(FilesController *fc, QUndoCommand *parent)
    : FilesCommandBase(fc, parent)
{
    mMasterSegment = NULL;
}



MergeSegmentsCommand::~MergeSegmentsCommand()
{
    deleteData(mMasterSegment);
    deleteData(mOtherSegments);
    deleteData(mOtherParents);
}




void MergeSegmentsCommand::setMergeSegments(TrackDataSegment *master, const QList<TrackDataItem *> &others)
{
    mMasterSegment = master;
    mOtherSegments = others;
}




void MergeSegmentsCommand::redo()
{
    Q_ASSERT(mMasterSegment!=NULL);
    Q_ASSERT(!mOtherSegments.isEmpty());

    int num = mOtherSegments.count();
    mOtherCounts.resize(num);
    mOtherIndexes.resize(num);
    mOtherParents.resize(num);

    for (int i = 0; i<mOtherSegments.count(); ++i)
    {
        TrackDataItem *item = mOtherSegments[i];
        mOtherCounts[i] = item->childCount();

        TrackDataItem *parent = item->parent();
        Q_ASSERT(parent!=NULL);
        mOtherParents[i] = parent;
        mOtherIndexes[i] = parent->childIndex(item);

        model()->mergeItems(mMasterSegment, item, true);
    }

    updateMap();
}



void MergeSegmentsCommand::undo()
{
    Q_ASSERT(mMasterSegment!=NULL);
    Q_ASSERT(!mOtherSegments.isEmpty());
    Q_ASSERT(mOtherSegments.count()==mOtherCounts.count());
    Q_ASSERT(mOtherIndexes.count()==mOtherCounts.count());
    Q_ASSERT(mOtherParents.count()==mOtherCounts.count());

    for (int i = mOtherSegments.count()-1; i>=0; --i)
    {
        int num = mOtherCounts[i];
        TrackDataItem *item = mOtherSegments[i];
        int idx = mOtherIndexes[i];
        TrackDataItem *parent = mOtherParents[i];

        // The last 'num' points of the 'mMasterSegment' are those that
        // belong to the former 'item' segment, which was at index 'idx'
        // under parent 'parent'.
        model()->splitItem(mMasterSegment, mMasterSegment->childCount()-num-1,
                           item, parent, idx);
    }

    mOtherCounts.clear();
    mOtherIndexes.clear();
    mOtherParents.clear();

    updateMap();
}
