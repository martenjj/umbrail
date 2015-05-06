
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
    virtual TrackPropertiesPage *createPropertiesMetadataPage(const QList<TrackDataItem *> *items, QWidget *pnt = NULL) const { return (NULL); }

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
    const QAction *act = qobject_cast<const QAction *>(sdr);
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
    mImportData = NULL;					// nothing held at present
    mSavedCount = 0;
}


ImportFileCommand::~ImportFileCommand()
{
    delete mImportData;
}


void ImportFileCommand::redo()
{
    Q_ASSERT(mImportData!=NULL);
    mSavedCount = mImportData->childCount();		// how many tracks contained
    kDebug() << "from" << mImportData->name() << "count" << mSavedCount;

    TrackDataFile *root = model()->rootFileItem();
    if (root==NULL)					// no data in model yet
    {
        // Set the top level imported file item as the model file root.
        model()->setRootFileItem(mImportData);		// use this as root item
        mImportData = NULL;				// now owned by model
    }
    else
    {
        model()->startLayoutChange();

        // Now, all items (expected to be tracks or folders) contained in
        // the new file are set as children of the file root.
        while (mImportData->childCount()>0)
        {
            TrackDataItem *tdi = mImportData->takeFirstChildItem();
            if (tdi!=NULL) root->addChildItem(tdi);
        }

        model()->endLayoutChange();
        Q_ASSERT(mImportData->childCount()==0);		// should have taken all tracks
    }

    controller()->view()->clearSelection();
    updateMap();
}


void ImportFileCommand::undo()
{
    TrackDataFile *root = model()->rootFileItem();
    Q_ASSERT(root!=NULL);

    if (mImportData==NULL)				// was set as file root
    {
        mImportData = model()->takeRootFileItem();
    }
    else						// not used as file root,
    {							// just take back its children
        Q_ASSERT(root->childCount()>=mSavedCount);
        model()->startLayoutChange();

        for (int i = 0; i<mSavedCount; ++i)		// how many added last time
        {						// remove each from model
            TrackDataItem *item = root->takeLastChildItem();
            if (item==NULL) continue;			// and re-add to saved file item
            mImportData->addChildItem(item, 0);		// in the original order
        }

        model()->endLayoutChange();
        Q_ASSERT(mImportData->childCount()==mSavedCount);
    }

    kDebug() << "saved" << mImportData->name() << "children" << mImportData->childCount();

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
    updateMap();
}


void ChangeItemNameCommand::undo()
{
    TrackDataItem *item = mDataItem;
    Q_ASSERT(item!=NULL);
    kDebug() << "item" << item->name() << "back to" << mSavedName;

    item->setName(mSavedName);
    model()->changedItem(item);
    updateMap();
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
//  Add Container (track or folder)					//
//									//
//  We create the new container, and store it when required.  The	//
//  parent container is only referred to, and can be NULL meaning	//
//  the top level.							//
//									//
//////////////////////////////////////////////////////////////////////////

AddContainerCommand::AddContainerCommand(FilesController *fc, QUndoCommand *parent)
    : FilesCommandBase(fc, parent)
{
    mNewItemContainer = NULL;
    mParent = NULL;
    mType = TrackData::None;
}


AddContainerCommand::~AddContainerCommand()
{
    delete mNewItemContainer;
}


void AddContainerCommand::setData(TrackData::Type type, TrackDataItem *pnt)
{
    mType = type;
    mParent = pnt;
}


void AddContainerCommand::redo()
{
    controller()->view()->clearSelection();
    model()->startLayoutChange();

    if (mNewItemContainer==NULL)			// need to create new container
    {
        mNewItemContainer = new ItemContainer;

        TrackDataItem *addedItem = NULL;
        if (mType==TrackData::Track)
        {
            addedItem = new TrackDataTrack(QString::null);
            addedItem->setMetadata(DataIndexer::self()->index("creator"), KGlobal::mainComponent().aboutData()->appName());
        }
        else if (mType==TrackData::Folder) addedItem = new TrackDataFolder(QString::null);
        Q_ASSERT(addedItem!=NULL);

        kDebug() << "created" << addedItem->name();
        mNewItemContainer->addChildItem(addedItem);
    }

    Q_ASSERT(mNewItemContainer->childCount()==1);
    TrackDataItem *newItem = mNewItemContainer->takeFirstChildItem();

    if (mParent==NULL) mParent = model()->rootFileItem();
    Q_ASSERT(mParent!=NULL);
    mParent->addChildItem(newItem);

    model()->endLayoutChange();
    controller()->view()->selectItem(newItem);
    //updateMap();					// cannot cause a visual change
}


void AddContainerCommand::undo()
{
    Q_ASSERT(mNewItemContainer!=NULL);
    Q_ASSERT(mNewItemContainer->childCount()==0);
    Q_ASSERT(mParent!=NULL);

    controller()->view()->clearSelection();
    model()->startLayoutChange();

    TrackDataItem *newItem = mParent->takeLastChildItem();
    mNewItemContainer->addChildItem(newItem);

    model()->endLayoutChange();
    //updateMap();					// cannot cause a visual change
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
//  The source and destination are only referred to, and so can be	//
//  simple pointers.							//
//									//
//////////////////////////////////////////////////////////////////////////

MoveItemCommand::MoveItemCommand(FilesController *fc, QUndoCommand *parent)
    : FilesCommandBase(fc, parent)
{
    mDestination = NULL;
}


MoveItemCommand::~MoveItemCommand()
{
}


void MoveItemCommand::setData(const QList<TrackDataItem *> &items, TrackDataItem *dest)
{
    mItems = items;
    mDestination = dest;
}


void MoveItemCommand::redo()
{
    Q_ASSERT(mDestination!=NULL);
    Q_ASSERT(!mItems.isEmpty());

    controller()->view()->clearSelection();
    model()->startLayoutChange();

    const int num = mItems.count();
    mParentItems.resize(num);
    mParentIndexes.resize(num);

    for (int i = 0; i<num; ++i)
    {
        TrackDataItem *item = mItems[i];
        TrackDataItem *par = item->parent();
        Q_ASSERT(par!=NULL);
        mParentItems[i] = par;
        int idx = par->childIndex(item);
        mParentIndexes[i] = idx;

        kDebug() << "move" << item->name()
                 << "from" << par->name() << "index" << idx
                 << "->" << mDestination->name();

        par->takeChildItem(idx);
        mDestination->addChildItem(item);
        controller()->view()->selectItem(item, true);
    }

    model()->endLayoutChange();
    updateMap();
}


void MoveItemCommand::undo()
{
    Q_ASSERT(mDestination!=NULL);
    Q_ASSERT(!mItems.isEmpty());
    const int cnt = mItems.count();
    Q_ASSERT(mParentItems.count()==cnt);
    Q_ASSERT(mParentIndexes.count()==cnt);

    controller()->view()->clearSelection();
    model()->startLayoutChange();

    for (int i = cnt-1; i>=0; --i)
    {
        TrackDataItem *item = mItems[i];
        Q_ASSERT(item->parent()==mDestination);
        TrackDataItem *par = mParentItems[i];
        int idx = mParentIndexes[i];

        kDebug() << "move" << item->name() << "from" << mDestination->name()
                 << "->" << par->name() << "index" << idx;

        mDestination->removeChildItem(item);
        par->addChildItem(item, idx);
        controller()->view()->selectItem(item, true);
    }

    model()->endLayoutChange();
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

    const int num = mItems.count();
    mParentIndexes.resize(num);
    mParentItems.resize(num);

    for (int i = 0; i<num; ++i)
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
    const int cnt = mItems.count();
    Q_ASSERT(mParentItems.count()==cnt);
    Q_ASSERT(mParentIndexes.count()==cnt);

    controller()->view()->clearSelection();
    model()->startLayoutChange();

    for (int i = cnt-1; i>=0; --i)
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

//////////////////////////////////////////////////////////////////////////
//									//
//  Add Waypoint							//
//									//
//  We create the new point and store it.  We only refer to the		//
//  input folder to identify where to create it.			//
//									//
//////////////////////////////////////////////////////////////////////////

AddWaypointCommand::AddWaypointCommand(FilesController *fc, QUndoCommand *parent)
    : FilesCommandBase(fc, parent)
{
    mWaypointFolder = NULL;
    mSourcePoint = NULL;
    mNewWaypointContainer = NULL;
}


AddWaypointCommand::~AddWaypointCommand()
{
    delete mNewWaypointContainer;
}


void AddWaypointCommand::setData(const QString &name, qreal lat, qreal lon,
                                 TrackDataFolder *folder,
                                 const TrackDataAbstractPoint *sourcePoint)
{
    mWaypointName = name;
    mWaypointFolder = folder;
    mLatitude = lat;
    mLongitude = lon;
    mSourcePoint = sourcePoint;
}


void AddWaypointCommand::redo()
{
    Q_ASSERT(mWaypointFolder!=NULL);

    controller()->view()->clearSelection();
    model()->startLayoutChange();

    if (mNewWaypointContainer==NULL)			// need to create new waypoint
    {
        mNewWaypointContainer = new ItemContainer;

        TrackDataWaypoint *newWaypoint = new TrackDataWaypoint(mWaypointName);
        newWaypoint->setLatLong(mLatitude, mLongitude);
        if (mSourcePoint!=NULL)
        {
            newWaypoint->setElevation(mSourcePoint->elevation());
            newWaypoint->setTime(mSourcePoint->time());
            //newWaypoint->copyMetadata(mSourcePoint);
            newWaypoint->setMetadata(DataIndexer::self()->index("source"), mSourcePoint->name());
        }

        mNewWaypointContainer->addChildItem(newWaypoint);
    }

    Q_ASSERT(mNewWaypointContainer->childCount()==1);
    TrackDataItem *newPoint = mNewWaypointContainer->takeFirstChildItem();
    mWaypointFolder->addChildItem(newPoint);

    model()->endLayoutChange();
    controller()->view()->selectItem(newPoint);
    updateMap();
}


void AddWaypointCommand::undo()
{
    Q_ASSERT(mNewWaypointContainer!=NULL);
    Q_ASSERT(mNewWaypointContainer->childCount()==0);
    Q_ASSERT(mWaypointFolder!=NULL);

    controller()->view()->clearSelection();
    model()->startLayoutChange();

    TrackDataItem *newPoint = mWaypointFolder->takeLastChildItem();
    mNewWaypointContainer->addChildItem(newPoint);
    Q_ASSERT(mNewWaypointContainer->childCount()==1);

    model()->endLayoutChange();
    controller()->view()->selectItem(mWaypointFolder);
    updateMap();
}
