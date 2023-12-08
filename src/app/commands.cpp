//////////////////////////////////////////////////////////////////////////
//									//
//  Project:	Umbrail - GPX track viewer and editor			//
//									//
//////////////////////////////////////////////////////////////////////////
//									//
//  Copyright (c) 2014-2022 Jonathan Marten <jjm@keelhaul.me.uk>	//
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
#include "category.h"


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
//     the child list of an internal TrackDataContainer;  this is simply a
//     TrackDataItem that is not abstract so can be allocated.  Items must
//     be added or removed to/from this container using the functions
//     provided by TrackDataItem.
//
//     This is so that the parent of the item can be tracked as required.
//     When the TrackDataContainer is destroyed, any items that still belong
//     to it will finally be deleted.

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


// Locate any folders (at any depth) below that item.  If there are
// subfolders then they and the parent folder are returned separately
// with the parent folder first.
static void findFolders(TrackDataItem *item, QVector<TrackDataFolder *> *res)
{
    // See if this item is a folder.  If so, record it in the result list.
    TrackDataFolder *tdf = dynamic_cast<TrackDataFolder *>(item);
    if (tdf!=nullptr)
    {
        qDebug() << "found" << tdf->path();
        res->append(tdf);
    }

    // Recurse into child items to find any folders below here.
    const int num = item->childCount();
    for (int i = 0; i<num; ++i) findFolders(item->childAt(i), res);
}


void ImportFileCommand::redo()
{
    Q_ASSERT(mImportData!=nullptr);
    mSavedCount = mImportData->childCount();		// how many tracks contained
    qDebug() << "from" << mImportData->name() << "count" << mSavedCount << "opts" << mOptions.flags();

    // The "Ignore Home/Work" option and the NewlyImported flag
    // will already have been actioned by the importer, so there is
    // no need to take any account of those here.

    TrackDataFile *root = model()->rootFileItem();
    if (root==nullptr)					// no data in model yet
    {
        // This option should be disabled by the GUI if the model
        // is empty, so it should never be seen here.
        if (mOptions.hasFlag(ImporterExporterOptions::MergeWaypoints)) qWarning() << "Ignoring merge option into empty model";

        // Set the top level imported file item as the model file root.
        model()->setRootFileItem(mImportData);		// use this as root item
        mImportData = nullptr;				// now owned by model
    }
    else
    {
        model()->startLayoutChange();

        if (mOptions.hasFlag(ImporterExporterOptions::MergeWaypoints))
        {
            // When merging in this mode, the undo stack will be cleared
            // after the merge is complete.  This means that it is not
            // necessary to record everything that is done so that it can
            // be undone if requested.

            // Recursively look through the imported data for any folders,
            // and if the corresponding folders already exist then merge
            // them.
            //
            // Merging means first comparing all of the imported waypoints
            // with those in the existing folder.  Any that can be automatically
            // merged with an existing waypoint are merged and then removed from
            // the imported data.  Any that cannot be automatically merged are
            // added to the existing folder and again removed from the imported
            // data.  Any subfolders remaining after doing this are also added
            // to the existing folder.  The imported folder should then be empty
            // and is removed.
            QVector<TrackDataFolder *> importedFolders;
            findFolders(mImportData, &importedFolders);
            const int num = importedFolders.count();
            qDebug() << "found" << num << "folders";

            for (int i = 0; i<num; ++i)
            {
                TrackDataFolder *importFolder = importedFolders[i];
                const QString path = importFolder->path();
                qDebug() << "trying folder" << path;

                TrackDataFolder *existingFolder = TrackData::findFolderByPath(path, root);
                if (existingFolder!=nullptr)
                {
                    qDebug() << "already existing folder, count" << existingFolder->childCount() << "import" << importFolder->childCount();

                    // Look at all of the import points in this folder in order.
                    for (int j = 0; ; ++j)
                    {
                        // Do the loop check here, for the case where an item
                        // has been removed and we are looking again at the
                        // same position in the list.
again:                  if (j>=importFolder->childCount()) break;

                        // The import waypoint to potentially be merged.
                        // It may not be a waypoint (if not, most likely
                        // a subfolder), in which case just ignore it.
                        TrackDataWaypoint *importWpt = dynamic_cast<TrackDataWaypoint *>(importFolder->childAt(j));
                        if (importWpt==nullptr) continue;

                        // Compare it against all of the existing points in this folder.
                        for (int k = 0; k<existingFolder->childCount(); ++k)
                        {
                            // The existing waypoint to potentially be merged into.
                            // Again it may not be a waypoint, in which case
                            // just ignore it.
                            TrackDataWaypoint *existingWpt = dynamic_cast<TrackDataWaypoint *>(existingFolder->childAt(k));
                            if (existingWpt==nullptr) continue;

                            // See if the two waypoints can be automatically merged.
                            if (existingWpt->canMerge(importWpt))
                            {
                                existingWpt->mergeWith(importWpt);
                                importFolder->removeChildItem(importWpt);
                                goto again;		// continue checks with next
                            }				// (also exits this nested loop)
                        }

                        // If the waypoint could not be automatically merged,
                        // then simply move it to the existing folder.
                        qDebug() << "could not be merged" << importWpt->name();
                        importFolder->removeChildItem(importWpt);
                        existingFolder->addChildItem(importWpt);
                        goto again;			// continue checks with next
                    }

                    qDebug() << "after merge, import count" << importFolder->childCount();
                }
            }

            // Remove any import folders which have been emptied by
            // the merging above.  The list of folders originally found
            // is traversed in reverse order so that subfolders are removed
            // before their parent folders.  After this has been done, the
            // pointers in 'importedFolders' may no longer be valid.
            for (int i = num-1; i>=0; --i)
            {
                TrackDataFolder *tdf = importedFolders[i];
                if (tdf->childCount()==0)		// is the folder empty?
                {
                    TrackDataItem *pnt = tdf->parent();	// parent containing this folder
                    if (pnt==nullptr) continue;		// shouldn't happen at top level
                    qDebug() << "remove empty imported" << tdf->path();
                    pnt->removeChildItem(tdf);
                    delete tdf;				// don't need this any more
                }
            }
        }

        // Now, all remaining items (expected to be tracks, or folders if
        // a waypoint merge is not being done) contained in the new file
        // are set as children of the file root.
        while (mImportData->childCount()>0)
        {
            TrackDataItem *tdi = mImportData->takeFirstChildItem();
            if (tdi!=nullptr) root->addChildItem(tdi);
        }

        // Merge any categories defined in the import data with the existing
        // categories on the root file item.  Do not supersede any already
        // existing categories.
        CategoryList *newMap = mImportData->categories();
        if (newMap!=nullptr)				// import data has categories
        {
            CategoryList *catMap = root->categories();
            if (catMap==nullptr)			// but root does not so far
            {
                qDebug() << "adopting imported categories";
                root->setCategories(newMap);		// use those as categories
                mImportData->setCategories(nullptr);	// have now taken ownership
                catMap = newMap;			// update pointer to as set
            }
            else
            {
                qDebug() << "merging" << newMap->count() << "imported categories";
                catMap->addCategories(newMap, false);	// merge with current, no overwrite
            }

            qDebug() << "have" << catMap->count() << "categories";
        }

        model()->endLayoutChange();
        Q_ASSERT(mImportData->childCount()==0);		// should have taken everything
    }

    controller()->filesView()->clearSelection();
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

        // TODO: should undo the merge of the category maps?  And if so, how?

        model()->endLayoutChange();
        Q_ASSERT(mImportData->childCount()==mSavedCount);
    }

    qDebug() << "saved" << mImportData->name() << "children" << mImportData->childCount();

    controller()->filesView()->clearSelection();
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
    for (TrackDataItem *item : qAsConst(mDataItems))
    {
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
    for (TrackDataItem *item : qAsConst(mDataItems))
    {
        Q_ASSERT(item!=nullptr);
        QVariant savedValue = mSavedValues.takeFirst();
        qDebug() << "item" << item->name() << "data" << mKey << "back to" << savedValue;
        item->setMetadata(idx, savedValue);
        model()->changedItem(item);
    }

    Q_ASSERT(mSavedValues.isEmpty());
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

    controller()->filesView()->clearSelection();
    model()->startLayoutChange();

    TrackDataAbstractPoint *splitPoint = dynamic_cast<TrackDataAbstractPoint *>(mParentSegment->childAt(mSplitIndex));
    Q_ASSERT(splitPoint!=nullptr);

    if (mNewSegmentContainer==nullptr)
    {
        mNewSegmentContainer = new TrackDataContainer;

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
    controller()->filesView()->selectItem(mParentSegment);
    controller()->filesView()->selectItem(newSegment, true);
    controller()->doUpdateMap();
}


void SplitSegmentCommand::undo()
{
    Q_ASSERT(mParentSegment!=nullptr);
    Q_ASSERT(mNewSegmentContainer->childCount()==0);

    controller()->filesView()->clearSelection();
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
    controller()->filesView()->selectItem(mParentSegment);
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

    controller()->filesView()->clearSelection();
    model()->startLayoutChange();

    if (mSavedSegmentContainer==nullptr) mSavedSegmentContainer = new TrackDataContainer;
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
    controller()->filesView()->selectItem(mMasterSegment);
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

    controller()->filesView()->selectItem(mMasterSegment);
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
        controller()->filesView()->selectItem(item, true);
    }
    controller()->doUpdateMap();
}

//////////////////////////////////////////////////////////////////////////
//									//
//  Add Container (track, route or folder)				//
//									//
//  We create the new container, and store it when required.  The	//
//  parent container is only referred to, and can be NULL meaning	//
//  the top level.							//
//									//
//////////////////////////////////////////////////////////////////////////

AddContainerCommand::AddContainerCommand(FilesController *fc, QUndoCommand *parent)
    : FilesCommandBase(fc, parent)
{
    mNewItemContainer = nullptr;
    mParent = nullptr;
    mType = TrackData::None;
    mAddedItem = nullptr;
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
    controller()->filesView()->clearSelection();
    model()->startLayoutChange();

    if (mNewItemContainer==nullptr)			// need to create new container
    {
        mNewItemContainer = new TrackDataContainer;

        if (mType==TrackData::Track) mAddedItem = new TrackDataTrack;
        else if (mType==TrackData::Route) mAddedItem = new TrackDataRoute;
        else if (mType==TrackData::Folder) mAddedItem = new TrackDataFolder;

        Q_ASSERT(mAddedItem!=nullptr);
        if (!mAddName.isEmpty()) mAddedItem->setName(mAddName, true);
        mAddedItem->setMetadata("creator", QApplication::applicationDisplayName());

        qDebug() << "created" << mAddedItem->name();
        mNewItemContainer->addChildItem(mAddedItem);
    }

    Q_ASSERT(mNewItemContainer->childCount()==1);
    TrackDataItem *newItem = mNewItemContainer->takeFirstChildItem();

    if (mParent==nullptr) mParent = model()->rootFileItem();
    Q_ASSERT(mParent!=nullptr);
    mParent->addChildItem(newItem);

    model()->endLayoutChange();
    controller()->filesView()->selectItem(newItem);
}


void AddContainerCommand::undo()
{
    Q_ASSERT(mNewItemContainer!=nullptr);
    Q_ASSERT(mNewItemContainer->childCount()==0);
    Q_ASSERT(mParent!=nullptr);

    controller()->filesView()->clearSelection();
    model()->startLayoutChange();

    TrackDataItem *newItem = mParent->takeLastChildItem();
    mNewItemContainer->addChildItem(newItem);

    model()->endLayoutChange();
}


// This is a direct pointer to the item, so it must not be reparented
// or retained outside of the lifetime of this command object.  However,
// undo/redo will work even if this item is immediately modified after
// the command is first executed.
TrackDataItem *AddContainerCommand::addedItem() const
{
    return (mAddedItem);
}

//////////////////////////////////////////////////////////////////////////
//									//
//  Add Track Point							//
//									//
//  We create the new point and store it.  We only refer to the point	//
//  identifying the add position.					//
//									//
//////////////////////////////////////////////////////////////////////////

AddTrackpointCommand::AddTrackpointCommand(FilesController *fc, QUndoCommand *parent)
    : FilesCommandBase(fc, parent)
{
    mNewPointContainer = nullptr;
    mAtPoint = nullptr;
}


AddTrackpointCommand::~AddTrackpointCommand()
{
    delete mNewPointContainer;
}


void AddTrackpointCommand::setData(TrackDataItem *item)
{
    mAtPoint = dynamic_cast<TrackDataTrackpoint *>(item);
    Q_ASSERT(mAtPoint!=nullptr);
}


void AddTrackpointCommand::redo()
{
    Q_ASSERT(mAtPoint!=nullptr);

    controller()->filesView()->clearSelection();
    model()->startLayoutChange();

    TrackDataItem *parent = mAtPoint->parent();
    Q_ASSERT(parent!=nullptr);

    if (mNewPointContainer==nullptr)			// need to create new point
    {
        mNewPointContainer = new TrackDataContainer;

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
    controller()->filesView()->selectItem(newPoint);
    controller()->doUpdateMap();
}


void AddTrackpointCommand::undo()
{
    Q_ASSERT(mAtPoint!=nullptr);
    Q_ASSERT(mNewPointContainer->childCount()==0);

    controller()->filesView()->clearSelection();
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
    controller()->filesView()->selectItem(mAtPoint);
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

    controller()->filesView()->clearSelection();
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
    for (int i = 0; i<num; ++i)
    {
        TrackDataItem *item = mItems[i];
        qDebug() << "  ->" << mDestinationParent->name() << "index" << destRow;
        mDestinationParent->addChildItem(item, destRow);
        if (destRow!=-1) ++destRow;
    }

    model()->endLayoutChange();

    // Fourth pass:  Select each item that has been added.
    for (int i = 0; i<num; ++i)
    {
        TrackDataItem *item = mItems[i];
        controller()->filesView()->selectItem(item, true);
    }

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

    controller()->filesView()->clearSelection();
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

    model()->endLayoutChange();

    // Third pass:  Select all of them.  This avoids the selection not being
    // correct if any subsequently inserted items then reorder the earlier ones.
    for (int i = 0; i<num; ++i)
    {
        TrackDataItem *item = mItems[i];
        controller()->filesView()->selectItem(item, true);
    }

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

    controller()->filesView()->clearSelection();
    model()->startLayoutChange();

    if (mDeletedItemsContainer==nullptr) mDeletedItemsContainer = new TrackDataContainer;
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

    controller()->filesView()->clearSelection();
    model()->startLayoutChange();

    for (int i = num-1; i>=0; --i)
    {
        TrackDataItem *item = mDeletedItemsContainer->takeLastChildItem();
        TrackDataItem *parent = mParentItems[i];
        parent->addChildItem(item, mParentIndexes[i]);
    }
    Q_ASSERT(mDeletedItemsContainer->childCount()==0);

    model()->endLayoutChange();

    for (int i = 0; i<num; ++i)
    {
        TrackDataItem *parent = mParentItems[i];
        controller()->filesView()->selectItem(parent->childAt(mParentIndexes[i]), true);
    }

    mParentItems.clear();
    mParentIndexes.clear();

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
//  Create the new point and store it.  The input folder is only used	//
//  as a reference to identify where to create it.			//
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

    controller()->filesView()->clearSelection();
    model()->startLayoutChange();

    if (mNewWaypointContainer==nullptr)			// need to create new waypoint
    {
        mNewWaypointContainer = new TrackDataContainer;

        mAddedWaypoint = new TrackDataWaypoint;
        if (!mWaypointName.isEmpty()) mAddedWaypoint->setName(mWaypointName, true);
        mAddedWaypoint->setLatLong(mLatitude, mLongitude);
        if (mSourcePoint!=nullptr)
        {
            int idx = DataIndexer::index("ele");
            mAddedWaypoint->setMetadata(idx, mSourcePoint->metadata(idx));
            idx = DataIndexer::index("time");
            mAddedWaypoint->setMetadata(idx, mSourcePoint->metadata(idx));

            // Only set the "source" metadata if the mSourcePoint point has
            // a name.  See StopDetectDialogue::slotCommitResults() for the
            // situation where it may not.
            const QString sourceName = mSourcePoint->name();
            if (!sourceName.isEmpty()) mAddedWaypoint->setMetadata("source", sourceName);

            const QVariant stopData = mSourcePoint->metadata("stop");
            if (!stopData.isNull()) mAddedWaypoint->setMetadata("stop", stopData);
        }

        // Always set the "origin" metadata of the added point to reflect that
        // it has been created manually.
        //
        // String format from NavMarks PointsController::slotNewPoint()
        const QString orgData = "manual_"+QDateTime::currentDateTime().toString(Qt::ISODate);
        mAddedWaypoint->setMetadata("origin", orgData);

        mNewWaypointContainer->addChildItem(mAddedWaypoint);
    }

    Q_ASSERT(mNewWaypointContainer->childCount()==1);
    TrackDataItem *newPoint = mNewWaypointContainer->takeFirstChildItem();
    mWaypointFolder->addChildItem(newPoint);

    model()->endLayoutChange();
    controller()->filesView()->selectItem(newPoint);
    controller()->doUpdateMap();
}


void AddWaypointCommand::undo()
{
    Q_ASSERT(mNewWaypointContainer!=nullptr);
    Q_ASSERT(mNewWaypointContainer->childCount()==0);
    Q_ASSERT(mWaypointFolder!=nullptr);

    controller()->filesView()->clearSelection();
    model()->startLayoutChange();

    TrackDataItem *newPoint = mWaypointFolder->takeLastChildItem();
    mNewWaypointContainer->addChildItem(newPoint);
    Q_ASSERT(mNewWaypointContainer->childCount()==1);

    model()->endLayoutChange();
    controller()->filesView()->selectItem(mWaypointFolder);
    controller()->doUpdateMap();
}


// See AddContainerCommand::addedItem() for more information.
TrackDataWaypoint *AddWaypointCommand::addedItem() const
{
    return (mAddedWaypoint);
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

    controller()->filesView()->clearSelection();
    model()->startLayoutChange();

    if (mNewRoutepointContainer==nullptr)		// need to create new routepoint
    {
        mNewRoutepointContainer = new TrackDataContainer;

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
    controller()->filesView()->selectItem(newPoint);
    controller()->doUpdateMap();
}


void AddRoutepointCommand::undo()
{
    Q_ASSERT(mNewRoutepointContainer!=nullptr);
    Q_ASSERT(mNewRoutepointContainer->childCount()==0);
    Q_ASSERT(mRoutepointRoute!=nullptr);

    controller()->filesView()->clearSelection();
    model()->startLayoutChange();

    TrackDataItem *newPoint = mRoutepointRoute->takeLastChildItem();
    mNewRoutepointContainer->addChildItem(newPoint);
    Q_ASSERT(mNewRoutepointContainer->childCount()==1);

    model()->endLayoutChange();
    controller()->filesView()->selectItem(mRoutepointRoute);
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
    AddWaypointCommand::redo();				// add the basic waypoint

    TrackDataWaypoint *tdw = AddWaypointCommand::addedItem();
    Q_ASSERT(tdw!=nullptr);				// retrieve the just added point
    if (mLinkUrl.isValid()) tdw->setMetadata("link", mLinkUrl.toDisplayString());
    if (mDateTime.isValid()) tdw->setMetadata("time", mDateTime);
							// note added as a photo
    const QString orgData = "photo_"+QDateTime::currentDateTime().toString(Qt::ISODate);
    tdw->setMetadata("origin", orgData);
}


void AddPhotoCommand::undo()
{
    AddWaypointCommand::undo();
}

//////////////////////////////////////////////////////////////////////////
//									//
//  Replace Items							//
//									//
//  Remove the specified 'items' from their parent container, which	//
//  is assumed to be the same for all of them but need not be, and	//
//  then put the replacement 'item' where the first removed item was.	//
//									//
//////////////////////////////////////////////////////////////////////////

// TODO: equivalent to DeleteItemsCommand with no replacement

ReplaceItemsCommand::ReplaceItemsCommand(FilesController *fc, QUndoCommand *parent)
    : FilesCommandBase(fc, parent)
{
    mDeletedItemsContainer = nullptr;
    mAddedItem = nullptr;
    // Need this flag to distinguish between "no replacement specified"
    // and "replacement has been added to and is owned by main data tree".
    mWasAdded = false;
}


ReplaceItemsCommand::~ReplaceItemsCommand()
{
    delete mDeletedItemsContainer;
    delete mAddedItem;
}


void ReplaceItemsCommand::setData(const QList<TrackDataItem *> &removeItems, TrackDataItem *addItem)
{
     mRemoveItems = removeItems;
     mAddedItem = addItem;
}


void ReplaceItemsCommand::redo()
{
    Q_ASSERT(!mRemoveItems.isEmpty());
    controller()->filesView()->clearSelection();
    model()->startLayoutChange();

    if (mDeletedItemsContainer==nullptr) mDeletedItemsContainer = new TrackDataContainer;
    Q_ASSERT(mDeletedItemsContainer->childCount()==0);

    const int num = mRemoveItems.count();
    mParentIndexes.resize(num);
    mParentItems.resize(num);

    for (int i = 0; i<num; ++i)
    {
        TrackDataItem *item = mRemoveItems[i];
        TrackDataItem *parent = item->parent();
        Q_ASSERT(parent!=nullptr);
        mParentItems[i] = parent;
        mParentIndexes[i] = parent->childIndex(item);

        parent->removeChildItem(item);
        mDeletedItemsContainer->addChildItem(item);
    }

    if (mAddedItem!=nullptr)
    {
        int addedIndex = mParentIndexes[0];		// index of first removed item
        TrackDataItem *addedParent = mParentItems[0];	// parent of first removed item
        addedParent->addChildItem(mAddedItem, addedIndex);
							// add new child to it
        controller()->filesView()->selectItem(mAddedItem);
							// and select in view
        mAddedItem = nullptr;				// now claimed by data tree
        mWasAdded = true;				// note item was added
    }

    model()->endLayoutChange();
    controller()->doUpdateMap();
}


void ReplaceItemsCommand::undo()
{
    Q_ASSERT(!mRemoveItems.isEmpty());
    const int num = mRemoveItems.count();
    Q_ASSERT(mParentItems.count()==num);
    Q_ASSERT(mParentIndexes.count()==num);

    controller()->filesView()->clearSelection();
    model()->startLayoutChange();

    if (mWasAdded)					// a replacement was added,
    {							// remove it again
        int addedIndex = mParentIndexes[0];		// index of added item
        TrackDataItem *addedParent = mParentItems[0];	// parent of added item
        mAddedItem = addedParent->takeChildItem(addedIndex);
    }							// remove replacement from tree

    for (int i = num-1; i>=0; --i)			// now add back those deleted
    {
        TrackDataItem *item = mDeletedItemsContainer->takeLastChildItem();
        TrackDataItem *parent = mParentItems[i];
        parent->addChildItem(item, mParentIndexes[i]);
        controller()->filesView()->selectItem(item, true);
    }
    Q_ASSERT(mDeletedItemsContainer->childCount()==0);

    mParentItems.clear();
    mParentIndexes.clear();
    mWasAdded = false;

    model()->endLayoutChange();
    controller()->doUpdateMap();
}
