
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


#undef DEBUG_ITEMS


// TrackDataItem's passed in to here may well refer to parts of the main
// model data tree, or items intending to be added to or removed from it.
// Because of that, we need to be very careful when holding on to items,
// or deleting them when they may appear to be not needed.
//
// The rules to be followed are:
//
//   - Any pointer to an item passed in must be copied by using acceptItem().
//     This increments its reference count to indicate that we are holding a
//     pointer to it.
//
//   - A list of items passed in must be copied and then acceptList() used.
//     This increments each contained item's reference count as above.
//
//   - Any item that we create must either use acceptItem() or be added as
//     a child of another item.
//
//   - Any item or list of items that either was passed in and copied as above,
//     or created by us, must be deleted using deleteItem() or deleteList() as
//     appropriate.  The item, or the list's contained items, will not actually
//     be deleted if there are any other references to it.  Note that the item
//     having a parent (i.e. being a child of another) automatically counts as
//     a reference.
//
//     An item created by us may be deleted either when it is no longer
//     required, or in a destructor.  If not done in a destructor, the pointer
//     must be immediately set to NULL to avoid a possible double deletion
//     in the destructor.
//
// Not following these rules and deleting anything without these checks may result
// in double-deletion crashes, or deleting something that is still in use as part
// of the model's data tree.  Not deleting anything will probably have no
// ill effects apart from memory leaks.  Your choice...


// Needs to be a template, so that the type of the returned item is
// the same as the input.
template <typename T>
static inline T *acceptItem(T *item)
{
    item->ref();
#ifdef DEBUG_ITEMS
    kDebug() << "accepting item" << item << item->name();
#endif
    return (item);
}


// A template, so that it will work for any collection.
template<typename C>
static void acceptList(C &list)
{
    for (int i = 0; i<list.count(); ++i) acceptItem(list[i]);
}


// Can be generic, will work using the virtual destructor.
static void deleteItem(TrackDataItem *item)
{
    if (item==NULL) return;
    if (item->deref())
    {
#ifdef DEBUG_ITEMS
        kDebug() << "deleting" << item << "=" << item->name();
#endif
        delete item;
    }
#ifdef DEBUG_ITEMS
    else
    {
        kDebug() << "keeping" << item << "=" << item->name();
    }
#endif
}


// A template, so that it will work for any collection.
template<typename C>
static void deleteList(C &list)
{
    for (int i = 0; i<list.count(); ++i) deleteItem(list[i]);
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
    deleteItem(mTrackData);
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
    TrackDataItem *item = mDataItem;
    Q_ASSERT(item!=NULL);
    kDebug() << "item" << item->name() << "->" << mNewStyle.toString();

    mSavedStyle = *item->style();			// save original item style
    item->setStyle(mNewStyle);				// set new item style

    model()->changedItem(item);
    updateMap();
}




void ChangeItemStyleCommand::undo()
{
    TrackDataItem *item = mDataItem;
    Q_ASSERT(item!=NULL);
    kDebug() << "item" << item->name() << "back to" << mSavedStyle.toString();

    item->setStyle(mSavedStyle);			// restore original style

    model()->changedItem(item);
    updateMap();
}





void ChangeItemDataCommand::redo()
{
    TrackDataItem *item = mDataItem;
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
    TrackDataItem *item = mDataItem;
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
#ifdef DEBUG_ITEMS
    kDebug() << "start";
#endif
    deleteItem(mParentSegment);
    deleteItem(mNewSegment);
#ifdef DEBUG_ITEMS
    kDebug() << "done";
#endif
}


void SplitSegmentCommand::setData(TrackDataSegment *pnt, int idx)
{
#ifdef DEBUG_ITEMS
    kDebug() << "start";
#endif
    mParentSegment = acceptItem(pnt);
    mSplitIndex = idx;
#ifdef DEBUG_ITEMS
    kDebug() << "done";
#endif
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

    controller()->view()->clearSelection();

    TrackDataPoint *splitPoint = dynamic_cast<TrackDataPoint *>(mParentSegment->childAt(mSplitIndex));
    Q_ASSERT(splitPoint!=NULL);

    if (mNewSegment==NULL)
    {
        mNewSegment = new TrackDataSegment(makeSplitName(mParentSegment->name()));
        mNewSegment->copyMetadata(mParentSegment);
        acceptItem(mNewSegment);
        // TODO: copy style

        TrackDataPoint *copyPoint = new TrackDataPoint(makeSplitName(splitPoint->name()));
        copyPoint->copyData(splitPoint);
        copyPoint->copyMetadata(splitPoint);
        // TODO: copy style
        mNewSegment->addChildItem(copyPoint);
    }

    model()->splitItem(mParentSegment, mSplitIndex, mNewSegment);

    controller()->view()->selectItem(mParentSegment);
    controller()->view()->selectItem(mNewSegment, true);
    updateMap();
}


void SplitSegmentCommand::undo()
{
    Q_ASSERT(mNewSegment!=NULL);
    Q_ASSERT(mParentSegment!=NULL);

    controller()->view()->clearSelection();

    model()->mergeItems(mParentSegment, mNewSegment);
    deleteItem(mNewSegment);
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
#ifdef DEBUG_ITEMS
    kDebug() << "start";
#endif
    deleteItem(mMasterSegment);
    deleteList(mOtherSegments);
#ifdef DEBUG_ITEMS
    kDebug() << "done";
#endif
}


void MergeSegmentsCommand::setData(TrackDataSegment *master, const QList<TrackDataItem *> &others)
{
#ifdef DEBUG_ITEMS
    kDebug() << "start";
#endif
    mMasterSegment = acceptItem(master);
    mOtherSegments = others;
    acceptList(mOtherSegments);
#ifdef DEBUG_ITEMS
    kDebug() << "done";
#endif
}


void MergeSegmentsCommand::redo()
{
    Q_ASSERT(mMasterSegment!=NULL);
    Q_ASSERT(!mOtherSegments.isEmpty());

    controller()->view()->clearSelection();

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
#ifdef DEBUG_ITEMS
    kDebug() << "start";
#endif
    deleteItem(mNewTrack);
#ifdef DEBUG_ITEMS
    kDebug() << "done";
#endif
}


void AddTrackCommand::redo()
{
    controller()->view()->clearSelection();

    if (mNewTrack==NULL)				// need to create new track
    {
        mNewTrack = new TrackDataTrack(QString::null);
        acceptItem(mNewTrack);
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
#ifdef DEBUG_ITEMS
    kDebug() << "start";
#endif
    deleteItem(mMoveSegment);
#ifdef DEBUG_ITEMS
    kDebug() << "done";
#endif
}



void MoveSegmentCommand::setData(TrackDataSegment *tds, TrackDataTrack *destTrack)
{
#ifdef DEBUG_ITEMS
    kDebug() << "start";
#endif
    mMoveSegment = acceptItem(tds);
    mDestTrack = acceptItem(destTrack);
#ifdef DEBUG_ITEMS
    kDebug() << "done";
#endif
}




void MoveSegmentCommand::redo()
{
    Q_ASSERT(mMoveSegment!=NULL);
    Q_ASSERT(mDestTrack!=NULL);

    controller()->view()->clearSelection();

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

    controller()->view()->clearSelection();
    model()->moveItem(mMoveSegment, mOrigTrack, mOrigIndex);
    controller()->view()->selectItem(mMoveSegment);
    updateMap();
}
