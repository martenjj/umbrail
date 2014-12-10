
#include "filesmodel.h"

#include <qfont.h>
#include <qitemselectionmodel.h>

#include <kdebug.h>
#include <klocale.h>
#include <kicon.h>

#include "trackdata.h"






enum COLUMN
{
    COL_NAME,						// icon/description
    COL_COUNT						// how many - must be last
};


#define SIZE_ICON		QSize(16, 16)
#define SIZE_HINT		QSize(18, 18)




FilesModel::FilesModel(QObject *pnt)
    : QAbstractItemModel(pnt)
{
    kDebug();

    mRootFileItem = NULL;
    clear();
}


FilesModel::~FilesModel()
{
    delete mRootFileItem;
    kDebug() << "done";
}












TrackDataItem *FilesModel::itemForIndex(const QModelIndex &idx) const
{
    return (static_cast<TrackDataItem *>(idx.internalPointer()));
}



QModelIndex FilesModel::indexForItem(const TrackDataItem *tdi) const
{
    Q_ASSERT(tdi!=NULL);
    const TrackDataItem *pnt = tdi->parent();
    int row = (pnt==NULL ? 0 : pnt->childIndex(tdi));
    // static_cast will not work here due to const'ness
    return (row==-1 ? QModelIndex() : createIndex(row, 0, (void *) tdi));
}



QModelIndex FilesModel::index(int row, int col, const QModelIndex &pnt) const
{
    const TrackDataItem *tdi = itemForIndex(pnt);
    if (tdi==NULL)
    {
        if (isEmpty()) return (QModelIndex());
        if (row>0) return (QModelIndex());
        return (createIndex(row, col, mRootFileItem));
    }

    if (row>=tdi->childCount())				// only during initialisation
    {							// without SORTABLE_VIEW
        kDebug() << "requested index for nonexistent row" << row << "of" << tdi->childCount();
        return (QModelIndex());
    }

    return (createIndex(row, col, tdi->childAt(row)));
}



QModelIndex FilesModel::parent(const QModelIndex &idx) const
{
    const TrackDataItem *tdi = itemForIndex(idx);
    if (tdi->parent()==NULL) return (QModelIndex());
    return (indexForItem(tdi->parent()));
}


int FilesModel::rowCount(const QModelIndex &pnt) const
{
   if (pnt==QModelIndex()) return (!isEmpty() ? 1 : 0);
   const TrackDataItem *tdi = itemForIndex(pnt);
   Q_ASSERT(tdi!=NULL);
   return (tdi->childCount());
}


int FilesModel::columnCount(const QModelIndex &pnt) const
{
    return (COL_COUNT);
}




QVariant FilesModel::data(const QModelIndex &idx, int role) const
{
    const TrackDataItem *tdi = itemForIndex(idx);

    switch (role)
    {
case Qt::DisplayRole:
        switch (idx.column())
        {
case COL_NAME:     return (tdi->name());
        }
        break;

case Qt::DecorationRole:
        switch (idx.column())
        {
case COL_NAME:     return (KIcon(tdi->iconName()));
        }
        break;

case Qt::UserRole:					// data for sorting
        switch (idx.column())
        {
default:           return (data(idx, Qt::DisplayRole));
        }
        break;

case Qt::ToolTipRole:
        switch (idx.column())
        {
case COL_NAME:
            {
                const TrackDataFile *tdf = dynamic_cast<const TrackDataFile *>(tdi);
                if (tdf!=NULL) return (i18np("File %2 with %1 item", "File %2 with %1 items", tdf->childCount(), tdf->fileName().pathOrUrl()));
            }
            {
                const TrackDataTrack *tdt = dynamic_cast<const TrackDataTrack *>(tdi);
                if (tdt!=NULL) return (i18np("Track with %1 segment", "Track with %1 segments", tdt->childCount()));
            }
            {
                const TrackDataSegment *tds = dynamic_cast<const TrackDataSegment *>(tdi);
                if (tds!=NULL) return (i18np("Segment with %1 point", "Segment with %1 points", tds->childCount()));
            }
            {
                const TrackDataTrackpoint *tdp = dynamic_cast<const TrackDataTrackpoint *>(tdi);
                if (tdp!=NULL) return (i18n("Point at %1, elevation %2",
                                            tdp->formattedTime(true), tdp->formattedElevation()));
            }
            {
                const TrackDataFolder *tdf = dynamic_cast<const TrackDataFolder *>(tdi);
                if (tdf!=NULL) return (i18np("Folder with %1 item", "Folder with %1 items", tdf->childCount()));
            }
            {
                const TrackDataWaypoint *tdw = dynamic_cast<const TrackDataWaypoint *>(tdi);
                if (tdw!=NULL) return (i18n("Waypoint, elevation %1", tdw->formattedElevation()));
            }
            break;
        }
        break;

//case Qt::SizeHintRole:
//        switch (idx.column())
//        {
//case COL_SYM:      return (SIZE_HINT);
//        }
//        break;
    }

    return (QVariant());
}


QVariant FilesModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role!=Qt::DisplayRole) return (QVariant());
    if (orientation!=Qt::Horizontal) return (QVariant());

    switch (section)
    {
case COL_NAME:		return (i18n("Name"));
default:		return (QVariant());
    }
}


void FilesModel::clear()
{
    emit layoutAboutToBeChanged();
    delete mRootFileItem;				// delete any existing
    mRootFileItem = NULL;
    emit layoutChanged();
}


void FilesModel::addToplevelItem(TrackDataFile *tdf)
{
    emit layoutAboutToBeChanged();

    TrackDataFile *fileRoot = rootFileItem();
    if (fileRoot==NULL)
    {
        // If the model is currently empty, then first we add a file item
        // immediately under the (invisible) root item.  The input tracks
        // will then be adopted to become children of this.
        //
        // The original metadata from the file will have been copied to the
        // contained tracks by the importer.

        kDebug() << "adding root file item" << tdf->name();
        mRootFileItem = new TrackDataFile(tdf->name());
        mRootFileItem->setFileName(tdf->fileName());
        mRootFileItem->copyMetadata(tdf);
        fileRoot = mRootFileItem;
    }

    // Now, all items (expected to be tracks or folders) contained in
    // the new file are adopted as children of the file root.
    kDebug() << "adding from" << tdf->name() << tdf->childCount() << "items";
    while (tdf->childCount()>0)
    {
        TrackDataItem *tdi = tdf->takeFirstChildItem();
        if (tdi!=NULL) fileRoot->addChildItem(tdi);
    }

    emit layoutChanged();

    // The file root passed in is now empty, as all its children have been
    // taken away.  Leave it owned by the caller.
}


TrackDataItem *FilesModel::removeLastToplevelItem()
{
    // Remove and return the last track item child of the root file item.
    // TODO: need to make this work for removing the root file item?
    if (rootFileItem()->childCount()==0)
    {
        kDebug() << "nothing to remove";
        return (NULL);
    }

    emit layoutAboutToBeChanged();
    TrackDataItem *tdf = rootFileItem()->takeLastChildItem();
    emit layoutChanged();
    return (tdf);
}


void FilesModel::changedItem(const TrackDataItem *item)
{
    QModelIndex idx = indexForItem(item);
    emit dataChanged(idx, idx);
}


void FilesModel::startLayoutChange()
{
    emit layoutAboutToBeChanged();
}


void FilesModel::endLayoutChange()
{
    emit layoutChanged();
}


void FilesModel::clickedPoint(const TrackDataAbstractPoint *tdp, Qt::KeyboardModifiers mods)
{
    QItemSelectionModel::SelectionFlags selFlags;
    if (mods==Qt::NoModifier) selFlags = QItemSelectionModel::ClearAndSelect;
    else if (mods==Qt::ControlModifier) selFlags = QItemSelectionModel::Toggle;
    else return;

    kDebug() << "click for" << indexForItem(tdp) << "flags" << selFlags;
    emit clickedItem(indexForItem(tdp), static_cast<unsigned int>(selFlags));
}
