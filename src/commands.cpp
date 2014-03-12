
#include "commands.h"

#include <qaction.h>
#include <qmetaobject.h>

#include <kdebug.h>
#include <klocale.h>
#include <kglobal.h>
#include <kcomponentdata.h>
#include <kaboutdata.h>

#include "filesmodel.h"
#include "filesview.h"
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






QString CommandBase::senderText(const QObject *sdr)
{
    const QAction *act = static_cast<const QAction *>(sdr);
    if (act==NULL) return (i18n("Action"));		// not called by action
    QString t = act->text();				// GUI text of action

    // the "..." is I18N'ed so that translations can change it to something that
    // will never match, if the target language does not use "..."
    QString dotdotdot = i18nc("as added to actions", "...");
    if (t.endsWith(dotdotdot)) t.chop(dotdotdot.length());
    return (KGlobal::locale()->removeAcceleratorMarker(t));
}



void CommandBase::setSenderText(const QObject *sdr)
{
    setText(senderText(sdr));
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
    controller()->view()->clearSelection();
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

    controller()->view()->clearSelection();
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





void SplitSegmentCommand::setData(TrackDataSegment *pnt, int idx)
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

    controller()->view()->selectItem(mParentSegment);
    controller()->view()->selectItem(mNewSegment, true);
    updateMap();
}



void SplitSegmentCommand::undo()
{
    Q_ASSERT(mNewSegment!=NULL);
    Q_ASSERT(mParentSegment!=NULL);

    model()->mergeItems(mParentSegment, mNewSegment);
    delete mNewSegment;					// new segment and copied point
    mNewSegment = NULL;

    controller()->view()->selectItem(mParentSegment);
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




void MergeSegmentsCommand::setData(TrackDataSegment *master, const QList<TrackDataItem *> &others)
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

    controller()->view()->selectItem(mMasterSegment);
    updateMap();
}



void MergeSegmentsCommand::undo()
{
    Q_ASSERT(mMasterSegment!=NULL);
    Q_ASSERT(!mOtherSegments.isEmpty());
    Q_ASSERT(mOtherSegments.count()==mOtherCounts.count());
    Q_ASSERT(mOtherIndexes.count()==mOtherCounts.count());
    Q_ASSERT(mOtherParents.count()==mOtherCounts.count());

    controller()->view()->selectItem(mMasterSegment);
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
        controller()->view()->selectItem(item, true);
    }

    mOtherCounts.clear();
    mOtherIndexes.clear();
    mOtherParents.clear();

    updateMap();
}





AddTrackCommand::AddTrackCommand(FilesController *fc, QUndoCommand *parent)
    : FilesCommandBase(fc, parent)
{
    mNewTrack = NULL;
}



AddTrackCommand::~AddTrackCommand()
{
    deleteData(mNewTrack);
}




void AddTrackCommand::redo()
{
    if (mNewTrack==NULL)				// need to create new track
    {
        mNewTrack = new TrackDataTrack(QString::null);
        mNewTrack->setMetadata(DataIndexer::self()->index("creator"), KGlobal::mainComponent().aboutData()->appName());
        kDebug() << "created" << mNewTrack->name();
    }

    TrackDataFile tdf(QString::null);
    tdf.addChildItem(mNewTrack);
    model()->addToplevelItem(&tdf);

    controller()->view()->selectItem(mNewTrack);
    updateMap();
}



void AddTrackCommand::undo()
{
    controller()->view()->clearSelection();
    mNewTrack = dynamic_cast<TrackDataTrack *>(model()->removeLastToplevelItem());

    updateMap();
}









MoveSegmentCommand::MoveSegmentCommand(FilesController *fc, QUndoCommand *parent)
    : FilesCommandBase(fc, parent)
{
    mMoveSegment = NULL;
    mOrigTrack = NULL;
    mDestTrack = NULL;
}



MoveSegmentCommand::~MoveSegmentCommand()
{
    deleteData(mMoveSegment);
}



void MoveSegmentCommand::setData(TrackDataSegment *tds, TrackDataTrack *destTrack)
{
    mMoveSegment = tds;
    mDestTrack = destTrack;
}




void MoveSegmentCommand::redo()
{
    Q_ASSERT(mMoveSegment!=NULL);
    Q_ASSERT(mDestTrack!=NULL);

    mOrigTrack = mMoveSegment->parent();
    Q_ASSERT(mOrigTrack!=NULL);
    mOrigIndex = mOrigTrack->childIndex(mMoveSegment);

    kDebug() << "move" << mMoveSegment->name()
             << "from" << mOrigTrack->name() << "index" << mOrigIndex
             << "->" << mDestTrack->name();

    model()->moveItem(mMoveSegment, mDestTrack);
    controller()->view()->selectItem(mMoveSegment);
    updateMap();
}




void MoveSegmentCommand::undo()
{
    Q_ASSERT(mMoveSegment!=NULL);
    Q_ASSERT(mDestTrack!=NULL);
    Q_ASSERT(mOrigTrack!=NULL);

    kDebug() << "move" << mMoveSegment->name()
             << "from" << mDestTrack->name()
             << "->" << mOrigTrack->name() << "index" << mOrigIndex;

    model()->moveItem(mMoveSegment, mOrigTrack, mOrigIndex);
    controller()->view()->selectItem(mMoveSegment);
    updateMap();
}
