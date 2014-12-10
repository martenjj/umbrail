
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

//  TrackDataItem's manipulated by these commands may well refer to parts
//  of the main model data tree, or items intending to be added to or
//  removed from it.  Because of that, code here needs to be very careful
//  when holding on to items.
//
//  The rules to be followed are:
//
//   - If the command only refers to, or changes existing items in place,
//     and does not copy, delete or move them around in memory any way,
//     then it may retain a pointer (or any structure or list of pointers)
//     to them.  In this case, it must not delete the items on destruction;
//     it is sufficient to just throw away the pointer.
//
//   - If the command needs to retain a copy of existing items, because
//     they are being modified or deleted, then they must be stored in
//     the child list of an internal ItemContainer;  this is simply a
//     TrackDataItem that is not abstract so can be allocated.  Items must
//     be added or removed to/from this container using the functions
//     provided by TrackDataItem.
//
//     This is so that the parent of the item can be tracked as required.
//     When the ItemContainer is destroyed, any items that still belong to
//     it will finally be deleted.

//////////////////////////////////////////////////////////////////////////
//									//
//  ItemContainer							//
//									//
//////////////////////////////////////////////////////////////////////////

class ItemContainer : public TrackDataItem
{
public:
    ItemContainer();
    virtual ~ItemContainer();

    virtual QString iconName() const		{ return (QString::null); }

    virtual TrackPropertiesPage *createPropertiesGeneralPage(const QList<TrackDataItem *> *items, QWidget *pnt = NULL) const { return (NULL); }
    virtual TrackPropertiesPage *createPropertiesDetailPage(const QList<TrackDataItem *> *items, QWidget *pnt = NULL) const { return (NULL); }
    virtual TrackPropertiesPage *createPropertiesStylePage(const QList<TrackDataItem *> *items, QWidget *pnt = NULL) const { return (NULL); }

private:
    static int containerCounter;
};


int ItemContainer::containerCounter = 0;


ItemContainer::ItemContainer()
    : TrackDataItem(QString::null, "container_%04d", &containerCounter)
{
#ifdef DEBUG_ITEMS
    kDebug() << "created" << name();
#endif
}


ItemContainer::~ItemContainer()
{
#ifdef DEBUG_ITEMS
    kDebug() << "destroying" << name();
#endif
    for (int i = 0; i<childCount(); ++i)
    {
#ifdef DEBUG_ITEMS
        kDebug() << "  child" << childAt(i)->name();
#endif
    }
#ifdef DEBUG_ITEMS
    kDebug() << "done";
#endif
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



/////////// TODO: function in controller
void FilesCommandBase::updateMap() const
{
    // Cannot emit a signal directly, because we are not a QObject.
    // Ask the FilesController to emit the signal instead.
    QMetaObject::invokeMethod(controller(), "updateMap");
}

//////////////////////////////////////////////////////////////////////////
//									//
//  Import File								//
//									//
//  This command is special.  It is only used for that operation, by	//
//  FilesController::importFile(), and it takes ownership of the	//
//  TrackDataFile passed in.  So we can retain a pointer to that,	//
//  and delete it on destruction.					//
//									//
//////////////////////////////////////////////////////////////////////////

ImportFileCommand::ImportFileCommand(FilesController *fc, QUndoCommand *parent)
    : FilesCommandBase(fc, parent)
{
    mTrackData = NULL;					// nothing held at present
    mSavedCount = 0;
}


ImportFileCommand::~ImportFileCommand()
{
    delete mTrackData;
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

//////////////////////////////////////////////////////////////////////////
//									//
//  Change Item (name, style, metadata)					//
//									//
//  All of these simply change the specified item in place, so they	//
//  can retain a pointer to it.						//
//									//
//////////////////////////////////////////////////////////////////////////

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
    //updateMap();
}

//////////////////////////////////////////////////////////////////////////
//									//
//  Split Segment							//
//									//
//  Leave the parent segment in place, and move all its child items	//
//  after the split point to a new segment.  The parent segment can	//
//  therefore just keep a pointer.  The new segment is created when	//
//  required and needs to be stored in a container.			//
//									//
//////////////////////////////////////////////////////////////////////////

SplitSegmentCommand::SplitSegmentCommand(FilesController *fc, QUndoCommand *parent)
    : FilesCommandBase(fc, parent)
{
    mParentSegment = NULL;				// nothing set at present
    mSplitIndex = -1;
    mNewSegmentContainer = NULL;
}


SplitSegmentCommand::~SplitSegmentCommand()
{
    delete mNewSegmentContainer;
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

    controller()->view()->clearSelection();
    model()->startLayoutChange();

    TrackDataTrackpoint *splitPoint = dynamic_cast<TrackDataTrackpoint *>(mParentSegment->childAt(mSplitIndex));
    Q_ASSERT(splitPoint!=NULL);

    if (mNewSegmentContainer==NULL)
    {
        mNewSegmentContainer = new ItemContainer;

        TrackDataSegment *copySegment = new TrackDataSegment(makeSplitName(mParentSegment->name()));
        copySegment->copyMetadata(mParentSegment);
        mNewSegmentContainer->addChildItem(copySegment);
        // TODO: copy style

        // TODO: can eliminate copyData with a copy constructor? or a clone()?
        TrackDataTrackpoint *copyPoint = new TrackDataTrackpoint(makeSplitName(splitPoint->name()));
        copyPoint->copyData(splitPoint);
        copyPoint->copyMetadata(splitPoint);
        // TODO: copy style
        copySegment->addChildItem(copyPoint);
    }

    Q_ASSERT(mNewSegmentContainer->childCount()==1);
    TrackDataSegment *newSegment = static_cast<TrackDataSegment *>(mNewSegmentContainer->takeFirstChildItem());
    Q_ASSERT(newSegment!=NULL);

    int takeFrom = mSplitIndex+1;
    kDebug() << "from" << mParentSegment->name() << "start" << takeFrom << "->" << newSegment->name();

    // Move child items following the split index to the receiving item
    while (mParentSegment->childCount()>takeFrom)
    {
        TrackDataItem *movedItem = mParentSegment->takeChildItem(takeFrom);
        newSegment->addChildItem(movedItem);
    }

    // Adopt the receiving item as the next sibling of the split item
    TrackDataItem *parentItem = mParentSegment->parent();
    int parentIndex = (parentItem->childIndex(mParentSegment)+1);
    Q_ASSERT(parentItem!=NULL);
    kDebug() << "add" << newSegment->name() << "to" << parentItem->name() << "as index" << parentIndex;
    parentItem->addChildItem(newSegment, parentIndex);

    model()->endLayoutChange();
    controller()->view()->selectItem(mParentSegment);
    controller()->view()->selectItem(newSegment, true);
    updateMap();
}


void SplitSegmentCommand::undo()
{
    Q_ASSERT(mParentSegment!=NULL);
    Q_ASSERT(mNewSegmentContainer->childCount()==0);

    controller()->view()->clearSelection();
    model()->startLayoutChange();

    // mParentSegment is the original segment that the split items are to be
    // merged back in to.  The added segment will be its next sibling.

    TrackDataItem *parentItem = mParentSegment->parent();
    Q_ASSERT(parentItem!=NULL);
    const int parentIndex = parentItem->childIndex(mParentSegment);
    TrackDataSegment *newSegment = static_cast<TrackDataSegment *>(parentItem->childAt(parentIndex+1));

    const int startIndex = 1;				// all apart from first point
    kDebug() << "from" << newSegment->name() << "count" << newSegment->childCount()
             << "->" << mParentSegment->name();

    // Append all the added segment's child items, apart from the first,
    // to the original parent item
    while (newSegment->childCount()>startIndex)
    {
        TrackDataItem *movedItem = newSegment->takeChildItem(startIndex);
        mParentSegment->addChildItem(movedItem);
    }

    // Remove and reclaim the now (effectively) empty source item
    kDebug() << "remove" << newSegment->name() << "from" << parentItem->name();
    parentItem->removeChildItem(newSegment);
    mNewSegmentContainer->addChildItem(newSegment);
    Q_ASSERT(mNewSegmentContainer->childCount()==1);

    model()->endLayoutChange();
    controller()->view()->selectItem(mParentSegment);
    updateMap();
}

//////////////////////////////////////////////////////////////////////////
//									//
//  Merge Segments							//
//									//
//  The master segment is left in place, and the children of all the	//
//  other source segments are merged into it.  The source segments	//
//  are then removed from their original place and stored here, along	//
//  with the information as to where they came from (original parent	//
//  and index).								//
//									//
//  The GUI enforces that the master and all the source segments must	//
//  have the same parent, but this is not mandated here.		//
//									//
//////////////////////////////////////////////////////////////////////////

MergeSegmentsCommand::MergeSegmentsCommand(FilesController *fc, QUndoCommand *parent)
    : FilesCommandBase(fc, parent)
{
    mMasterSegment = NULL;
    mSavedSegmentContainer = NULL;
}


MergeSegmentsCommand::~MergeSegmentsCommand()
{
    delete mSavedSegmentContainer;
}


void MergeSegmentsCommand::setData(TrackDataSegment *master, const QList<TrackDataItem *> &others)
{
    mMasterSegment = master;
    mSourceSegments = others;
}


void MergeSegmentsCommand::redo()
{
    Q_ASSERT(mMasterSegment!=NULL);
    Q_ASSERT(!mSourceSegments.isEmpty());

    controller()->view()->clearSelection();
    model()->startLayoutChange();

    if (mSavedSegmentContainer==NULL) mSavedSegmentContainer = new ItemContainer;
    Q_ASSERT(mSavedSegmentContainer->childCount()==0);

    const int num = mSourceSegments.count();
    mSourceCounts.resize(num);
    mSourceIndexes.resize(num);
    mSourceParents.resize(num);

    for (int i = 0; i<num; ++i)
    {
        TrackDataItem *item = mSourceSegments[i];
        mSourceCounts[i] = item->childCount();

        TrackDataItem *parent = item->parent();
        Q_ASSERT(parent!=NULL);
        mSourceParents[i] = parent;
        mSourceIndexes[i] = parent->childIndex(item);

        kDebug() << "from" << item->name() << "count" << item->childCount()
                 << "->" << mMasterSegment->name();

        // Append all the source segment's child items to the master segment
        while (item->childCount()>0)
        {
            TrackDataItem *movedItem = item->takeFirstChildItem();
            mMasterSegment->addChildItem(movedItem);
        }

        // Remove and adopt the now empty source segment
        TrackDataItem *parentItem = item->parent();
        Q_ASSERT(parentItem!=NULL);
        kDebug() << "remove" << item->name() << "from" << parentItem->name();
        parentItem->removeChildItem(item);
        mSavedSegmentContainer->addChildItem(item);
    }
    Q_ASSERT(mSavedSegmentContainer->childCount()==num);

    model()->endLayoutChange();
    controller()->view()->selectItem(mMasterSegment);
    updateMap();
}


void MergeSegmentsCommand::undo()
{
    Q_ASSERT(mMasterSegment!=NULL);
    Q_ASSERT(mSavedSegmentContainer!=NULL);

    const int segCount = mSavedSegmentContainer->childCount();
    Q_ASSERT(segCount>0);
    Q_ASSERT(mSourceCounts.count()==segCount);
    Q_ASSERT(mSourceIndexes.count()==segCount);
    Q_ASSERT(mSourceParents.count()==segCount);

    controller()->view()->selectItem(mMasterSegment);
    model()->startLayoutChange();

    for (int i = segCount-1; i>=0; --i)
    {
        const int num = mSourceCounts[i];
        TrackDataItem *item = mSavedSegmentContainer->takeLastChildItem();

        // The last 'num' points of the 'mMasterSegment' are those that
        // originally belonged to the former 'item' segment.

        int takeFrom = mMasterSegment->childCount()-num;
        kDebug() << "from" << mMasterSegment->name()  << "start" << takeFrom
                 << "->" << item->name();

        // Move child items following the split index to the original source item
        while (mMasterSegment->childCount()>takeFrom)
        {
            TrackDataItem *movedItem = mMasterSegment->takeChildItem(takeFrom);
            item->addChildItem(movedItem);
        }

        // Put the receiving segment, which was originally at index 'idx'
        // under parent 'parent', back in its original place.
        const int idx = mSourceIndexes[i];
        TrackDataItem *parent = mSourceParents[i];
        Q_ASSERT(parent!=NULL);
        kDebug() << "add to" << parent->name() << "as index" << idx;
        parent->addChildItem(item, idx);
    }

    Q_ASSERT(mSavedSegmentContainer->childCount()==0);
    mSourceCounts.clear();
    mSourceIndexes.clear();
    mSourceParents.clear();

    model()->endLayoutChange();
    for (int i = 0; i<segCount; ++i)
    {
        const TrackDataItem *item = mSourceSegments[i];
        controller()->view()->selectItem(item, true);
    }
    updateMap();
}

//////////////////////////////////////////////////////////////////////////
//									//
//  Add Track								//
//									//
//  We create a new track, and store it when required.			//
//									//
//////////////////////////////////////////////////////////////////////////

AddTrackCommand::AddTrackCommand(FilesController *fc, QUndoCommand *parent)
    : FilesCommandBase(fc, parent)
{
    mNewTrackContainer = NULL;
}


AddTrackCommand::~AddTrackCommand()
{
    delete mNewTrackContainer;
}


void AddTrackCommand::redo()
{
    controller()->view()->clearSelection();

    if (mNewTrackContainer==NULL)			// need to create new track
    {
        mNewTrackContainer = new ItemContainer;

        TrackDataTrack *copyTrack = new TrackDataTrack(QString::null);
        copyTrack->setMetadata(DataIndexer::self()->index("creator"), KGlobal::mainComponent().aboutData()->appName());
        kDebug() << "created" << copyTrack->name();
        mNewTrackContainer->addChildItem(copyTrack);
    }

    Q_ASSERT(mNewTrackContainer->childCount()==1);
    TrackDataItem *newTrack = mNewTrackContainer->takeFirstChildItem();

    // The TrackDataTrack needs to be enclosed in a temporary TrackDataFile
    // in order to be passed to the model.
    TrackDataFile tdf(QString::null);
    tdf.addChildItem(newTrack);
    model()->addToplevelItem(&tdf);

    controller()->view()->selectItem(newTrack);
    updateMap();
}


void AddTrackCommand::undo()
{
    Q_ASSERT(mNewTrackContainer!=NULL);
    Q_ASSERT(mNewTrackContainer->childCount()==0);

    controller()->view()->clearSelection();

    TrackDataItem *newTrack = model()->removeLastToplevelItem();
    mNewTrackContainer->addChildItem(newTrack);

    updateMap();
}

//////////////////////////////////////////////////////////////////////////
//									//
//  Add Point								//
//									//
//  We create the new point and store it.  We only refer to the point	//
//  identifying the add position.					//
//									//
//////////////////////////////////////////////////////////////////////////

AddPointCommand::AddPointCommand(FilesController *fc, QUndoCommand *parent)
    : FilesCommandBase(fc, parent)
{
    mNewPointContainer = NULL;
    mAtPoint = NULL;
}


AddPointCommand::~AddPointCommand()
{
    delete mNewPointContainer;
}


void AddPointCommand::setData(TrackDataItem *item)
{
    mAtPoint = dynamic_cast<TrackDataTrackpoint *>(item);
    Q_ASSERT(mAtPoint!=NULL);
}


void AddPointCommand::redo()
{
    Q_ASSERT(mAtPoint!=NULL);

    controller()->view()->clearSelection();
    model()->startLayoutChange();

    TrackDataItem *parent = mAtPoint->parent();
    Q_ASSERT(parent!=NULL);

    if (mNewPointContainer==NULL)			// need to create new point
    {
        mNewPointContainer = new ItemContainer;

        TrackDataTrackpoint *copyPoint = new TrackDataTrackpoint(QString::null);

        const int idx = parent->childIndex(mAtPoint);
        Q_ASSERT(idx>0);				// not allowed at first point
        const TrackDataTrackpoint *prevPoint = dynamic_cast<const TrackDataTrackpoint *>(parent->childAt(idx-1));
        Q_ASSERT(prevPoint!=NULL);

        double lat = (mAtPoint->latitude()+prevPoint->latitude())/2;
        double lon = (mAtPoint->longitude()+prevPoint->longitude())/2;
        copyPoint->setLatLong(lat, lon);		// interpolate position

        kDebug() << "created" << copyPoint->name();
        mNewPointContainer->addChildItem(copyPoint);
    }

    Q_ASSERT(mNewPointContainer->childCount()==1);
    TrackDataItem *newPoint = mNewPointContainer->takeFirstChildItem();
    parent->addChildItem(newPoint, parent->childIndex(mAtPoint));

    model()->endLayoutChange();
    controller()->view()->selectItem(newPoint);
    updateMap();
}


void AddPointCommand::undo()
{
    Q_ASSERT(mAtPoint!=NULL);
    Q_ASSERT(mNewPointContainer->childCount()==0);

    controller()->view()->clearSelection();
    model()->startLayoutChange();

    TrackDataItem *parent = mAtPoint->parent();
    Q_ASSERT(parent!=NULL);
    const int idx = parent->childIndex(mAtPoint);
    Q_ASSERT(idx>0);

    TrackDataItem *newPoint = parent->childAt(idx-1);	// the one we added
    parent->removeChildItem(newPoint);
    mNewPointContainer->addChildItem(newPoint);
    Q_ASSERT(mNewPointContainer->childCount()==1);

    model()->endLayoutChange();
    controller()->view()->selectItem(mAtPoint);
    updateMap();
}

//////////////////////////////////////////////////////////////////////////
//									//
//  Move Segment							//
//									//
//  The segment, origin and destination are only referred to and so	//
//  can be simple pointers.						//
//									//
//////////////////////////////////////////////////////////////////////////

MoveSegmentCommand::MoveSegmentCommand(FilesController *fc, QUndoCommand *parent)
    : FilesCommandBase(fc, parent)
{
    mMoveSegment = NULL;
    mOrigTrack = NULL;
    mDestTrack = NULL;
}


MoveSegmentCommand::~MoveSegmentCommand()
{
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

    controller()->view()->clearSelection();
    model()->startLayoutChange();

    mOrigTrack = dynamic_cast<TrackDataTrack *>(mMoveSegment->parent());
    Q_ASSERT(mOrigTrack!=NULL);
    mOrigIndex = mOrigTrack->childIndex(mMoveSegment);

    kDebug() << "move" << mMoveSegment->name()
             << "from" << mOrigTrack->name() << "index" << mOrigIndex
             << "->" << mDestTrack->name();

    mOrigTrack->removeChildItem(mMoveSegment);
    mDestTrack->addChildItem(mMoveSegment);

    model()->endLayoutChange();
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
    model()->startLayoutChange();

    mDestTrack->removeChildItem(mMoveSegment);
    mOrigTrack->addChildItem(mMoveSegment, mOrigIndex);

    model()->endLayoutChange();
    controller()->view()->selectItem(mMoveSegment);
    updateMap();
}

//////////////////////////////////////////////////////////////////////////
//									//
//  Delete Items							//
//									//
//  The deleted items (which may be the root of a complete tree) are	//
//  retained in our container.  The source parents are only referred	//
//  to and can be simple pointers.					//
//									//
//////////////////////////////////////////////////////////////////////////

DeleteItemsCommand::DeleteItemsCommand(FilesController *fc, QUndoCommand *parent)
    : FilesCommandBase(fc, parent)
{
    mDeletedItemsContainer = NULL;
}


DeleteItemsCommand::~DeleteItemsCommand()
{
    delete mDeletedItemsContainer;
}


void DeleteItemsCommand::setData(const QList<TrackDataItem *> &items)
{
     mItems = items;
}


void DeleteItemsCommand::redo()
{
    Q_ASSERT(!mItems.isEmpty());

    controller()->view()->clearSelection();
    model()->startLayoutChange();

    if (mDeletedItemsContainer==NULL) mDeletedItemsContainer = new ItemContainer;
    Q_ASSERT(mDeletedItemsContainer->childCount()==0);

    int num = mItems.count();
    mParentIndexes.resize(num);
    mParentItems.resize(num);

    for (int i = 0; i<mItems.count(); ++i)
    {
        TrackDataItem *item = mItems[i];
        TrackDataItem *parent = item->parent();
        Q_ASSERT(parent!=NULL);
        mParentItems[i] = parent;
        mParentIndexes[i] = parent->childIndex(item);

        parent->removeChildItem(item);
        mDeletedItemsContainer->addChildItem(item);
    }

    model()->endLayoutChange();
    updateMap();
}


void DeleteItemsCommand::undo()
{
    Q_ASSERT(!mItems.isEmpty());
    Q_ASSERT(mParentItems.count()==mItems.count());
    Q_ASSERT(mParentIndexes.count()==mItems.count());

    controller()->view()->clearSelection();
    model()->startLayoutChange();

    for (int i = mItems.count()-1; i>=0; --i)
    {
        TrackDataItem *item = mDeletedItemsContainer->takeLastChildItem();
        TrackDataItem *parent = mParentItems[i];
        parent->addChildItem(item, mParentIndexes[i]);

        controller()->view()->selectItem(item, true);
    }
    Q_ASSERT(mDeletedItemsContainer->childCount()==0);

    mParentItems.clear();
    mParentIndexes.clear();

    model()->endLayoutChange();
    updateMap();
}

//////////////////////////////////////////////////////////////////////////
//									//
//  Move Points								//
//									//
//  Changes the specified items in place, so we can retain a pointer	//
//  to them.								//
//									//
//////////////////////////////////////////////////////////////////////////

void MovePointsCommand::setDataItems(const QList<TrackDataItem *> &items)
{
    mItems = items;
}


void MovePointsCommand::redo()
{
    Q_ASSERT(!mItems.isEmpty());
    for (int i = 0; i<mItems.count(); ++i)
    {
        TrackDataAbstractPoint *item = dynamic_cast<TrackDataAbstractPoint *>(mItems[i]);
        if (item==NULL) continue;
        item->setLatLong(item->latitude()+mLatOff, item->longitude()+mLonOff);
        model()->changedItem(item);
    }

    updateMap();
}


void MovePointsCommand::undo()
{
    Q_ASSERT(!mItems.isEmpty());
    for (int i = 0; i<mItems.count(); ++i)
    {
        TrackDataAbstractPoint *item = dynamic_cast<TrackDataAbstractPoint *>(mItems[i]);
        if (item==NULL) continue;
        item->setLatLong(item->latitude()-mLatOff, item->longitude()-mLonOff);
        model()->changedItem(item);
    }

    updateMap();
}
