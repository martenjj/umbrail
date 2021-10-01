//////////////////////////////////////////////////////////////////////////
//									//
//  Project:	Umbrail - GPX track viewer and editor			//
//									//
//////////////////////////////////////////////////////////////////////////
//									//
//  Copyright (c) 2014-2021 Jonathan Marten <jjm@keelhaul.me.uk>	//
//  Home and download page: <http://github.com/martenjj/umbrail>	//
//									//
//  This program is free software; you can redistribute it and/or	//
//  modify it under the terms of the GNU General Public License as	//
//  published by the Free Software Foundation, either version 3 of	//
//  the License or (at your option) any later version.			//
//									//
//  It is distributed in the hope that it will be useful, but		//
//  WITHOUT ANY WARRANTY;  without even the implied warranty of		//
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the	//
//  GNU General Public License for more details.			//
//									//
//  You should have received a copy of the GNU General Public License	//
//  along with this program;  see the file COPYING for further		//
//  details.  If not, see <http://gnu.org/licenses/gpl>.      		//
//									//
//////////////////////////////////////////////////////////////////////////

#include "commands.h"

#include <qaction.h>
#include <qmetaobject.h>
#include <qapplication.h>
#include <qdebug.h>

#include <klocalizedstring.h>

#include "filesmodel.h"
#include "filesview.h"
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
//     and does not copy, delete or move them around in the file tree in any
//     way, then it may retain a pointer (or any structure or list of pointers)
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
    explicit ItemContainer();
    virtual ~ItemContainer();

    TrackData::Type type() const override	{ return (TrackData::None); }
    virtual QString iconName() const override	{ return (QString()); }

private:
    static int containerCounter;
};


int ItemContainer::containerCounter = 0;


ItemContainer::ItemContainer()
    : TrackDataItem("container_%04d", &containerCounter)
{
#ifdef DEBUG_ITEMS
    qDebug() << "created" << name();
#endif
}


ItemContainer::~ItemContainer()
{
#ifdef DEBUG_ITEMS
    qDebug() << "destroying" << name();
#endif
    for (int i = 0; i<childCount(); ++i)
    {
#ifdef DEBUG_ITEMS
        qDebug() << "  child" << childAt(i)->name();
#endif
    }
#ifdef DEBUG_ITEMS
    qDebug() << "done";
#endif
}


QString CommandBase::senderText(const QObject *sdr)
{
    const QAction *act = qobject_cast<const QAction *>(sdr);
    if (act==nullptr) return (i18n("Action"));		// not called by action
    QString t = act->text();				// GUI text of action

    // the "..." is I18N'ed so that translations can change it to something that
    // will never match, if the target language does not use "..."
    QString dotdotdot = i18nc("as added to actions", "...");
    if (t.endsWith(dotdotdot)) t.chop(dotdotdot.length());
    return (KLocalizedString::removeAcceleratorMarker(t));
}


void CommandBase::setSenderText(const QObject *sdr)
{
    setText(senderText(sdr));
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
    mImportData = nullptr;				// nothing held at present
    mSavedCount = 0;
}


ImportFileCommand::~ImportFileCommand()
{
    delete mImportData;
}


void ImportFileCommand::redo()
{
    Q_ASSERT(mImportData!=nullptr);
    mSavedCount = mImportData->childCount();		// how many tracks contained
    qDebug() << "from" << mImportData->name() << "count" << mSavedCount;

    TrackDataFile *root = model()->rootFileItem();
    if (root==nullptr)					// no data in model yet
    {
        // Set the top level imported file item as the model file root.
        model()->setRootFileItem(mImportData);		// use this as root item
        mImportData = nullptr;				// now owned by model
    }
    else
    {
        model()->startLayoutChange();

        // Now, all items (expected to be tracks or folders) contained in
        // the new file are set as children of the file root.
        while (mImportData->childCount()>0)
        {
            TrackDataItem *tdi = mImportData->takeFirstChildItem();
            if (tdi!=nullptr) root->addChildItem(tdi);
        }

        model()->endLayoutChange();
        Q_ASSERT(mImportData->childCount()==0);		// should have taken all tracks
    }

    controller()->view()->clearSelection();
    controller()->doUpdateMap();
}


void ImportFileCommand::undo()
{
    TrackDataFile *root = model()->rootFileItem();
    Q_ASSERT(root!=nullptr);

    if (mImportData==nullptr)				// was set as file root
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
            if (item==nullptr) continue;		// and re-add to saved file item
            mImportData->addChildItem(item, 0);		// in the original order
        }

        model()->endLayoutChange();
        Q_ASSERT(mImportData->childCount()==mSavedCount);
    }

    qDebug() << "saved" << mImportData->name() << "children" << mImportData->childCount();

    controller()->view()->clearSelection();
    controller()->doUpdateMap();
}

//////////////////////////////////////////////////////////////////////////
//									//
//  Change Item (name, metadata)					//
//									//
//  All of these simply change the specified items in place, so they	//
//  can retain a pointer to them.					//
//									//
//////////////////////////////////////////////////////////////////////////

ChangeItemCommand::ChangeItemCommand(FilesController *fc, QUndoCommand *parent)
    : FilesCommandBase(fc, parent)
{
    mDataItems.clear();					// nothing set at present
}


void ChangeItemNameCommand::redo()
{
    Q_ASSERT(mDataItems.count()==1);
    TrackDataItem *item = mDataItems.first();
    Q_ASSERT(item!=nullptr);
    mSavedName = item->name();
    mSavedExplicit = item->hasExplicitName();
    qDebug() << "item" << mSavedName << "explicit?" << mSavedExplicit << "->" << mNewName;

    item->setName(mNewName, true);
    model()->changedItem(item);
    controller()->doUpdateMap();
}


void ChangeItemNameCommand::undo()
{
    Q_ASSERT(mDataItems.count()==1);
    TrackDataItem *item = mDataItems.first();
    Q_ASSERT(item!=nullptr);
    qDebug() << "item" << item->name() << "back to" << mSavedName << "explicit?" << mSavedExplicit;

    item->setName(mSavedName, mSavedExplicit);
    model()->changedItem(item);
    controller()->doUpdateMap();
}


void ChangeItemDataCommand::redo()
{
    mSavedValues.clear();

    const int idx = DataIndexer::index(mKey);
    for (QList<TrackDataItem *>::const_iterator it = mDataItems.constBegin(); it!=mDataItems.constEnd(); ++it)
    {
        TrackDataItem *item = (*it);
        Q_ASSERT(item!=nullptr);
        qDebug() << "item" << item->name() << "data" << mKey << "->" << mNewValue;

        mSavedValues.append(item->metadata(idx));
        item->setMetadata(idx, mNewValue);
        model()->changedItem(item);
    }

    Q_ASSERT(mSavedValues.count()==mDataItems.count());
}


void ChangeItemDataCommand::undo()
{
    Q_ASSERT(mSavedValues.count()==mDataItems.count());

    const int idx = DataIndexer::index(mKey);
    for (QList<TrackDataItem *>::const_iterator it = mDataItems.constBegin(); it!=mDataItems.constEnd(); ++it)
    {
        TrackDataItem *item = (*it);
        Q_ASSERT(item!=nullptr);
        QVariant savedValue = mSavedValues.takeFirst();
        qDebug() << "item" << item->name() << "data" << mKey << "back to" << savedValue;
        item->setMetadata(idx, savedValue);
        model()->changedItem(item);
    }

    Q_ASSERT(mSavedValues.count()==0);
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
//  Despite the historical naming, this works on routes also.		//
//									//
//////////////////////////////////////////////////////////////////////////

SplitSegmentCommand::SplitSegmentCommand(FilesController *fc, QUndoCommand *parent)
    : FilesCommandBase(fc, parent)
{
    mParentSegment = nullptr;				// nothing set at present
    mSplitIndex = -1;
    mNewSegmentContainer = nullptr;
}


SplitSegmentCommand::~SplitSegmentCommand()
{
    delete mNewSegmentContainer;
}


void SplitSegmentCommand::setData(TrackDataItem *pnt, int idx)
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
    Q_ASSERT(mParentSegment!=nullptr);
    Q_ASSERT(mSplitIndex>0 && mSplitIndex<(mParentSegment->childCount()-1));

    controller()->view()->clearSelection();
    model()->startLayoutChange();

    TrackDataAbstractPoint *splitPoint = dynamic_cast<TrackDataAbstractPoint *>(mParentSegment->childAt(mSplitIndex));
    Q_ASSERT(splitPoint!=nullptr);

    if (mNewSegmentContainer==nullptr)
    {
        mNewSegmentContainer = new ItemContainer;

        TrackDataItem *copySegment;
        TrackDataAbstractPoint *copyPoint;

        if (dynamic_cast<TrackDataTrackpoint *>(splitPoint)!=nullptr)
        {
            copySegment = new TrackDataSegment;
            copyPoint = new TrackDataTrackpoint;
        }
        else if (dynamic_cast<TrackDataRoutepoint *>(splitPoint)!=nullptr)
        {
            copySegment = new TrackDataRoute;
            copyPoint = new TrackDataRoutepoint;
        }
        else Q_ASSERT(false);

        copySegment->setName(makeSplitName(mParentSegment->name()), false);
        copySegment->copyMetadata(mParentSegment);
        mNewSegmentContainer->addChildItem(copySegment);

        copyPoint->setName(makeSplitName(splitPoint->name()), false);
        copyPoint->setLatLong(splitPoint->latitude(), splitPoint->longitude());
        copyPoint->copyMetadata(splitPoint);
        copySegment->addChildItem(copyPoint);
    }

    Q_ASSERT(mNewSegmentContainer->childCount()==1);
    TrackDataItem *newSegment = mNewSegmentContainer->takeFirstChildItem();
    Q_ASSERT(newSegment!=nullptr);

    int takeFrom = mSplitIndex+1;
    qDebug() << "from" << mParentSegment->name() << "start" << takeFrom << "->" << newSegment->name();

    // Move child items following the split index to the receiving item
    while (mParentSegment->childCount()>takeFrom)
    {
        TrackDataItem *movedItem = mParentSegment->takeChildItem(takeFrom);
        newSegment->addChildItem(movedItem);
    }

    // Adopt the receiving item as the next sibling of the split item
    TrackDataItem *parentItem = mParentSegment->parent();
    int parentIndex = (parentItem->childIndex(mParentSegment)+1);
    Q_ASSERT(parentItem!=nullptr);
    qDebug() << "add" << newSegment->name() << "to" << parentItem->name() << "as index" << parentIndex;
    parentItem->addChildItem(newSegment, parentIndex);

    model()->endLayoutChange();
    controller()->view()->selectItem(mParentSegment);
    controller()->view()->selectItem(newSegment, true);
    controller()->doUpdateMap();
}


void SplitSegmentCommand::undo()
{
    Q_ASSERT(mParentSegment!=nullptr);
    Q_ASSERT(mNewSegmentContainer->childCount()==0);

    controller()->view()->clearSelection();
    model()->startLayoutChange();

    // mParentSegment is the original segment that the split items are to be
    // merged back in to.  The added segment will be its next sibling.

    TrackDataItem *parentItem = mParentSegment->parent();
    Q_ASSERT(parentItem!=nullptr);
    const int parentIndex = parentItem->childIndex(mParentSegment);
    TrackDataItem *newSegment = parentItem->childAt(parentIndex+1);

    const int startIndex = 1;				// all apart from first point
    qDebug() << "from" << newSegment->name() << "count" << newSegment->childCount()
             << "->" << mParentSegment->name();

    // Append all the added segment's child items, apart from the first,
    // to the original parent item
    while (newSegment->childCount()>startIndex)
    {
        TrackDataItem *movedItem = newSegment->takeChildItem(startIndex);
        mParentSegment->addChildItem(movedItem);
    }

    // Remove and reclaim the now (effectively) empty source item
    qDebug() << "remove" << newSegment->name() << "from" << parentItem->name();
    parentItem->removeChildItem(newSegment);
    mNewSegmentContainer->addChildItem(newSegment);
    Q_ASSERT(mNewSegmentContainer->childCount()==1);

    model()->endLayoutChange();
    controller()->view()->selectItem(mParentSegment);
    controller()->doUpdateMap();
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
//  Again, despite the historical naming, this works on routes also.	//
//									//
//////////////////////////////////////////////////////////////////////////

MergeSegmentsCommand::MergeSegmentsCommand(FilesController *fc, QUndoCommand *parent)
    : FilesCommandBase(fc, parent)
{
    mMasterSegment = nullptr;
    mSavedSegmentContainer = nullptr;
}


MergeSegmentsCommand::~MergeSegmentsCommand()
{
    delete mSavedSegmentContainer;
}


void MergeSegmentsCommand::setData(TrackDataItem *master, const QList<TrackDataItem *> &others)
{
    mMasterSegment = master;
    mSourceSegments = others;
}


void MergeSegmentsCommand::redo()
{
    Q_ASSERT(mMasterSegment!=nullptr);
    Q_ASSERT(!mSourceSegments.isEmpty());

    controller()->view()->clearSelection();
    model()->startLayoutChange();

    if (mSavedSegmentContainer==nullptr) mSavedSegmentContainer = new ItemContainer;
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
        Q_ASSERT(parent!=nullptr);
        mSourceParents[i] = parent;
        mSourceIndexes[i] = parent->childIndex(item);

        qDebug() << "from" << item->name() << "count" << item->childCount()
                 << "->" << mMasterSegment->name();

        // Append all the source segment's child items to the master segment
        while (item->childCount()>0)
        {
            TrackDataItem *movedItem = item->takeFirstChildItem();
            mMasterSegment->addChildItem(movedItem);
        }

        // Remove and adopt the now empty source segment
        TrackDataItem *parentItem = item->parent();
        Q_ASSERT(parentItem!=nullptr);
        qDebug() << "remove" << item->name() << "from" << parentItem->name();
        parentItem->removeChildItem(item);
        mSavedSegmentContainer->addChildItem(item);
    }
    Q_ASSERT(mSavedSegmentContainer->childCount()==num);

    model()->endLayoutChange();
    controller()->view()->selectItem(mMasterSegment);
    controller()->doUpdateMap();
}


void MergeSegmentsCommand::undo()
{
    Q_ASSERT(mMasterSegment!=nullptr);
    Q_ASSERT(mSavedSegmentContainer!=nullptr);

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
        qDebug() << "from" << mMasterSegment->name()  << "start" << takeFrom
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
        Q_ASSERT(parent!=nullptr);
        qDebug() << "add to" << parent->name() << "as index" << idx;
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
    controller()->doUpdateMap();
}

//////////////////////////////////////////////////////////////////////////
//									//
//  Add Container (track, route or folder)				//
//									//
//  We create the new container, and store it when required.  The	//
//  parent container is only referred to, and can be nullptr meaning	//
//  the top level.							//
//									//
//////////////////////////////////////////////////////////////////////////

// TODO: not thread safe!
static TrackDataFolder *lastCreatedFolder = nullptr;


AddContainerCommand::AddContainerCommand(FilesController *fc, QUndoCommand *parent)
    : FilesCommandBase(fc, parent)
{
    mNewItemContainer = nullptr;
    mParent = nullptr;
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

    if (mNewItemContainer==nullptr)			// need to create new container
    {
        mNewItemContainer = new ItemContainer;

        TrackDataItem *addedItem = nullptr;
        if (mType==TrackData::Track) addedItem = new TrackDataTrack;
        else if (mType==TrackData::Route) addedItem = new TrackDataRoute;
        else if (mType==TrackData::Folder)
        {
            lastCreatedFolder = new TrackDataFolder;
            addedItem = lastCreatedFolder;
        }

        Q_ASSERT(addedItem!=nullptr);
        if (!mAddName.isEmpty()) addedItem->setName(mAddName, true);
        addedItem->setMetadata("creator", QApplication::applicationDisplayName());

        qDebug() << "created" << addedItem->name();
        mNewItemContainer->addChildItem(addedItem);
    }

    Q_ASSERT(mNewItemContainer->childCount()==1);
    TrackDataItem *newItem = mNewItemContainer->takeFirstChildItem();

    if (mParent==nullptr) mParent = model()->rootFileItem();
    Q_ASSERT(mParent!=nullptr);
    mParent->addChildItem(newItem);

    model()->endLayoutChange();
    controller()->view()->selectItem(newItem);
}


void AddContainerCommand::undo()
{
    Q_ASSERT(mNewItemContainer!=nullptr);
    Q_ASSERT(mNewItemContainer->childCount()==0);
    Q_ASSERT(mParent!=nullptr);

    controller()->view()->clearSelection();
    model()->startLayoutChange();

    TrackDataItem *newItem = mParent->takeLastChildItem();
    mNewItemContainer->addChildItem(newItem);

    model()->endLayoutChange();
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
    mNewPointContainer = nullptr;
    mAtPoint = nullptr;
}


AddPointCommand::~AddPointCommand()
{
    delete mNewPointContainer;
}


void AddPointCommand::setData(TrackDataItem *item)
{
    mAtPoint = dynamic_cast<TrackDataTrackpoint *>(item);
    Q_ASSERT(mAtPoint!=nullptr);
}


void AddPointCommand::redo()
{
    Q_ASSERT(mAtPoint!=nullptr);

    controller()->view()->clearSelection();
    model()->startLayoutChange();

    TrackDataItem *parent = mAtPoint->parent();
    Q_ASSERT(parent!=nullptr);

    if (mNewPointContainer==nullptr)			// need to create new point
    {
        mNewPointContainer = new ItemContainer;

        TrackDataTrackpoint *copyPoint = new TrackDataTrackpoint;

        const int idx = parent->childIndex(mAtPoint);
        Q_ASSERT(idx>0);				// not allowed at first point
        const TrackDataTrackpoint *prevPoint = dynamic_cast<const TrackDataTrackpoint *>(parent->childAt(idx-1));
        Q_ASSERT(prevPoint!=nullptr);

        double lat = (mAtPoint->latitude()+prevPoint->latitude())/2;
        double lon = (mAtPoint->longitude()+prevPoint->longitude())/2;
        copyPoint->setLatLong(lat, lon);		// interpolate position

        qDebug() << "created" << copyPoint->name();
        mNewPointContainer->addChildItem(copyPoint);
    }

    Q_ASSERT(mNewPointContainer->childCount()==1);
    TrackDataItem *newPoint = mNewPointContainer->takeFirstChildItem();
    parent->addChildItem(newPoint, parent->childIndex(mAtPoint));

    model()->endLayoutChange();
    controller()->view()->selectItem(newPoint);
    controller()->doUpdateMap();
}


void AddPointCommand::undo()
{
    Q_ASSERT(mAtPoint!=nullptr);
    Q_ASSERT(mNewPointContainer->childCount()==0);

    controller()->view()->clearSelection();
    model()->startLayoutChange();

    TrackDataItem *parent = mAtPoint->parent();
    Q_ASSERT(parent!=nullptr);
    const int idx = parent->childIndex(mAtPoint);
    Q_ASSERT(idx>0);

    TrackDataItem *newPoint = parent->childAt(idx-1);	// the one we added
    parent->removeChildItem(newPoint);
    mNewPointContainer->addChildItem(newPoint);
    Q_ASSERT(mNewPointContainer->childCount()==1);

    model()->endLayoutChange();
    controller()->view()->selectItem(mAtPoint);
    controller()->doUpdateMap();
}

//////////////////////////////////////////////////////////////////////////
//									//
//  Move Item(s)							//
//									//
//  The source items and destination are only referred to, and so	//
//  can be simple pointers.						//
//									//
//////////////////////////////////////////////////////////////////////////

MoveItemCommand::MoveItemCommand(FilesController *fc, QUndoCommand *parent)
    : FilesCommandBase(fc, parent)
{
    mDestinationParent = nullptr;
}


MoveItemCommand::~MoveItemCommand()
{
}


void MoveItemCommand::setData(const QList<TrackDataItem *> &items, TrackDataItem *dest, int row)
{
    mItems = items;
    mDestinationParent = dest;
    mDestinationRow = row;
}


void MoveItemCommand::redo()
{
    Q_ASSERT(mDestinationParent!=nullptr);
    Q_ASSERT(!mItems.isEmpty());

    controller()->view()->clearSelection();
    model()->startLayoutChange();

    const int num = mItems.count();
    mParentItems.resize(num);
    mParentIndexes.resize(num);

    // The 'mDestinationRow' (if it is not the default) specifies where the
    // destination items are to be inserted.  However, if the move is within
    // the same container the row will change as the items to be moved are
    // removed.  To account for this we find the item that the moved items
    // are to be inserted at (before), then remove all of the items to be
    // moved, then recalculate the destination row as that of the insertion
    // point item.

    TrackDataItem *destItem = nullptr;
    int destRow = mDestinationRow;

    // If the insertion point is at the end, this is the same as the "default"
    // case.
    if (destRow>=mDestinationParent->childCount()) destRow = -1;

    // Find the 'destItem' that corresponds to the insertion point.
    if (destRow!=-1) destItem = mDestinationParent->childAt(destRow);

    // First pass:  Calculate the indexes within their parent item of all of
    // the source items.  Do this before anything is removed fom the source
    // parent.
    for (int i = 0; i<num; ++i)
    {
        TrackDataItem *item = mItems[i];
        TrackDataItem *par = item->parent();
        Q_ASSERT(par!=nullptr);
        mParentItems[i] = par;
        const int idx = par->childIndex(item);
        mParentIndexes[i] = idx;
        qDebug() << "move" << item->name() << "from" << par->name() << "index" << idx;
    }

    // Second pass:  Remove all of the source items from their current parent,
    // in reverse order so that the previously calculated 'mParentIndexes' will
    // still be correct.  There is no need to keep a list of what is removed,
    // because it will be the same as 'mItems'.
    for (int i = num-1; i>=0; --i)
    {
        TrackDataItem *par = mParentItems[i];
        const int idx = mParentIndexes[i];
        par->takeChildItem(idx);
    }

    // Recalculate the insertion point within the destination parent.
    // If that parent (at this stage unchanged) is not the same as the
    // source parent, then this will of course produce the same 'destRow'
    // as before.
    if (destItem!=nullptr)
    {
        Q_ASSERT(destRow!=-1);
        const int row = mDestinationParent->childIndex(destItem);
        if (row!=-1) destRow = row;
    }

    // Third pass:  Add all of the items to the destination parent.  If
    // the insertion point is not the default (at the end), then increment
    // the 'destRow' after each one so as to preserve the original order.
    // Select each item as it is added.
    for (int i = 0; i<num; ++i)
    {
        TrackDataItem *item = mItems[i];
        qDebug() << "  ->" << mDestinationParent->name() << "index" << destRow;
        mDestinationParent->addChildItem(item, destRow);
        if (destRow!=-1) ++destRow;
        controller()->view()->selectItem(item, true);
    }

    model()->endLayoutChange();
    controller()->doUpdateMap();
}


void MoveItemCommand::undo()
{
    Q_ASSERT(mDestinationParent!=nullptr);
    Q_ASSERT(!mItems.isEmpty());
    const int num = mItems.count();
    Q_ASSERT(mParentItems.count()==num);
    Q_ASSERT(mParentIndexes.count()==num);

    // As with redo(), moving the items back to their original locations
    // is done in three passes so as to handle the case where they are being
    // moved within a single parent correctly.

    controller()->view()->clearSelection();
    model()->startLayoutChange();

    // First pass:  Remove all of the items from their current parent.
    for (int i = 0; i<num; ++i)
    {
        TrackDataItem *item = mItems[i];
        Q_ASSERT(item->parent()==mDestinationParent);
        qDebug() << "move" << item->name() << "from" << mDestinationParent->name();
        mDestinationParent->removeChildItem(item);
    }

    // Second pass:  Put all of the items back to where they originally came
    // from, under their original 'mParentItems' at their 'mParentIndexes'
    // position.  Do this in forward order so that the previously calculated
    // indexes will be correct.
    for (int i = 0; i<num; ++i)
    {
        TrackDataItem *item = mItems[i];
        TrackDataItem *par = mParentItems[i];
        const int idx = mParentIndexes[i];
        qDebug() << "  ->" << par->name() << "index" << idx;
        par->addChildItem(item, idx);
    }

    // Third pass:  Select all of them.  This avoids the selection not being
    // correct if any subsequently inserted items then reorder the earlier ones.
    for (int i = 0; i<num; ++i)
    {
        TrackDataItem *item = mItems[i];
        controller()->view()->selectItem(item, true);
    }

    model()->endLayoutChange();
    controller()->doUpdateMap();
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
    mDeletedItemsContainer = nullptr;
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

    if (mDeletedItemsContainer==nullptr) mDeletedItemsContainer = new ItemContainer;
    Q_ASSERT(mDeletedItemsContainer->childCount()==0);

    const int num = mItems.count();
    mParentIndexes.resize(num);
    mParentItems.resize(num);

    for (int i = 0; i<num; ++i)
    {
        TrackDataItem *item = mItems[i];
        TrackDataItem *parent = item->parent();
        Q_ASSERT(parent!=nullptr);
        mParentItems[i] = parent;
        mParentIndexes[i] = parent->childIndex(item);

        parent->removeChildItem(item);
        mDeletedItemsContainer->addChildItem(item);
    }

    model()->endLayoutChange();
    controller()->doUpdateMap();
}


void DeleteItemsCommand::undo()
{
    Q_ASSERT(!mItems.isEmpty());
    const int num = mItems.count();
    Q_ASSERT(mParentItems.count()==num);
    Q_ASSERT(mParentIndexes.count()==num);

    controller()->view()->clearSelection();
    model()->startLayoutChange();

    for (int i = num-1; i>=0; --i)
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
    controller()->doUpdateMap();
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
        if (item==nullptr) continue;
        item->setLatLong(item->latitude()+mLatOff, item->longitude()+mLonOff);
        model()->changedItem(item);
    }

    controller()->doUpdateMap();
}


void MovePointsCommand::undo()
{
    Q_ASSERT(!mItems.isEmpty());
    for (int i = 0; i<mItems.count(); ++i)
    {
        TrackDataAbstractPoint *item = dynamic_cast<TrackDataAbstractPoint *>(mItems[i]);
        if (item==nullptr) continue;
        item->setLatLong(item->latitude()-mLatOff, item->longitude()-mLonOff);
        model()->changedItem(item);
    }

    controller()->doUpdateMap();
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
    mWaypointFolder = nullptr;
    mSourcePoint = nullptr;
    mNewWaypointContainer = nullptr;
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
    Q_ASSERT(mWaypointFolder!=nullptr);

    controller()->view()->clearSelection();
    model()->startLayoutChange();

    if (mNewWaypointContainer==nullptr)			// need to create new waypoint
    {
        mNewWaypointContainer = new ItemContainer;

        TrackDataWaypoint *newWaypoint = new TrackDataWaypoint;
        if (!mWaypointName.isEmpty()) newWaypoint->setName(mWaypointName, true);
        newWaypoint->setLatLong(mLatitude, mLongitude);
        if (mSourcePoint!=nullptr)
        {
            int idx = DataIndexer::index("ele");
            newWaypoint->setMetadata(idx, mSourcePoint->metadata(idx));
            idx = DataIndexer::index("time");
            newWaypoint->setMetadata(idx, mSourcePoint->metadata(idx));
            newWaypoint->setMetadata("source", mSourcePoint->name());
            const QVariant stopData = mSourcePoint->metadata("stop");
            if (!stopData.isNull()) newWaypoint->setMetadata("stop", stopData);
        }

        mNewWaypointContainer->addChildItem(newWaypoint);
    }

    Q_ASSERT(mNewWaypointContainer->childCount()==1);
    TrackDataItem *newPoint = mNewWaypointContainer->takeFirstChildItem();
    mWaypointFolder->addChildItem(newPoint);

    model()->endLayoutChange();
    controller()->view()->selectItem(newPoint);
    controller()->doUpdateMap();
}


void AddWaypointCommand::undo()
{
    Q_ASSERT(mNewWaypointContainer!=nullptr);
    Q_ASSERT(mNewWaypointContainer->childCount()==0);
    Q_ASSERT(mWaypointFolder!=nullptr);

    controller()->view()->clearSelection();
    model()->startLayoutChange();

    TrackDataItem *newPoint = mWaypointFolder->takeLastChildItem();
    mNewWaypointContainer->addChildItem(newPoint);
    Q_ASSERT(mNewWaypointContainer->childCount()==1);

    model()->endLayoutChange();
    controller()->view()->selectItem(mWaypointFolder);
    controller()->doUpdateMap();
}

//////////////////////////////////////////////////////////////////////////
//									//
//  Add Routepoint							//
//									//
//  We create the new point and store it.  We only refer to the		//
//  input folder to identify where to create it.			//
//									//
//////////////////////////////////////////////////////////////////////////

AddRoutepointCommand::AddRoutepointCommand(FilesController *fc, QUndoCommand *parent)
    : FilesCommandBase(fc, parent)
{
    mRoutepointRoute = nullptr;
    mSourcePoint = nullptr;
    mNewRoutepointContainer = nullptr;
}


AddRoutepointCommand::~AddRoutepointCommand()
{
    delete mNewRoutepointContainer;
}


void AddRoutepointCommand::setData(const QString &name, qreal lat, qreal lon,
                                 TrackDataRoute *route,
                                 const TrackDataAbstractPoint *sourcePoint)
{
    mRoutepointName = name;
    mRoutepointRoute = route;
    mLatitude = lat;
    mLongitude = lon;
    mSourcePoint = sourcePoint;
}


void AddRoutepointCommand::redo()
{
    Q_ASSERT(mRoutepointRoute!=nullptr);

    controller()->view()->clearSelection();
    model()->startLayoutChange();

    if (mNewRoutepointContainer==nullptr)		// need to create new routepoint
    {
        mNewRoutepointContainer = new ItemContainer;

        TrackDataRoutepoint *newRoutepoint = new TrackDataRoutepoint;
        if (!mRoutepointName.isEmpty()) newRoutepoint->setName(mRoutepointName, true);
        newRoutepoint->setLatLong(mLatitude, mLongitude);
        if (mSourcePoint!=nullptr) newRoutepoint->setMetadata("source", mSourcePoint->name());

        mNewRoutepointContainer->addChildItem(newRoutepoint);
    }

    Q_ASSERT(mNewRoutepointContainer->childCount()==1);
    TrackDataItem *newPoint = mNewRoutepointContainer->takeFirstChildItem();
    mRoutepointRoute->addChildItem(newPoint);

    model()->endLayoutChange();
    controller()->view()->selectItem(newPoint);
    controller()->doUpdateMap();
}


void AddRoutepointCommand::undo()
{
    Q_ASSERT(mNewRoutepointContainer!=nullptr);
    Q_ASSERT(mNewRoutepointContainer->childCount()==0);
    Q_ASSERT(mRoutepointRoute!=nullptr);

    controller()->view()->clearSelection();
    model()->startLayoutChange();

    TrackDataItem *newPoint = mRoutepointRoute->takeLastChildItem();
    mNewRoutepointContainer->addChildItem(newPoint);
    Q_ASSERT(mNewRoutepointContainer->childCount()==1);

    model()->endLayoutChange();
    controller()->view()->selectItem(mRoutepointRoute);
    controller()->doUpdateMap();
}

//////////////////////////////////////////////////////////////////////////
//									//
//  Add Photo								//
//									//
//  Just like creating a waypoint, except that there may be some more	//
//  information to fill in.						//
//									//
//////////////////////////////////////////////////////////////////////////

void AddPhotoCommand::redo()
{
    if (mWaypointFolder==nullptr)			// no destination folder set
    {
        mWaypointFolder = lastCreatedFolder;		// use the one just created
        Q_ASSERT(mWaypointFolder!=nullptr);
    }

    AddWaypointCommand::redo();				// add the basic waypoint

    TrackDataWaypoint *tdw = dynamic_cast<TrackDataWaypoint *>(mWaypointFolder->childAt(mWaypointFolder->childCount()-1));
    Q_ASSERT(tdw!=nullptr);				// retrieve the just added point
    if (mLinkUrl.isValid()) tdw->setMetadata("link", mLinkUrl.toDisplayString());
    if (mDateTime.isValid()) tdw->setMetadata("time", mDateTime);
}


void AddPhotoCommand::undo()
{
    AddWaypointCommand::undo();
}
