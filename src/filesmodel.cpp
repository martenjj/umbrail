
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




FilesModel::FilesModel(QObject *parent)
    : QAbstractItemModel(parent)
{
    kDebug();

    mRootItem = NULL;
    clear();
}


FilesModel::~FilesModel()
{
    delete mRootItem;
    kDebug() << "done";
}





// Actually, the first (and normally only) file item child of the root item.
TrackDataFile *FilesModel::rootFileItem() const
{
    if (isEmpty()) return (NULL);
    return (dynamic_cast<TrackDataFile *>(mRootItem->childAt(0)));
}


bool FilesModel::isEmpty() const
{
    return (mRootItem==NULL || mRootItem->childCount()==0);
}









TrackDataItem *FilesModel::dataPointer(const QModelIndex &idx) const
{
    TrackDataItem *tdi = static_cast<TrackDataItem *>(idx.internalPointer());
    if (tdi==NULL) tdi = mRootItem;
    return (tdi);
}



QModelIndex FilesModel::indexForData(const TrackDataItem *tdi) const
{
    /////////////////////// Sometimes happens with crash after undo of import
    Q_ASSERT(tdi!=NULL);

    if (tdi->parent()==NULL) return (QModelIndex());
    int row = tdi->parent()->childIndex(tdi);
    // static_cast will not work here due to const'ness
    return (row==-1 ? QModelIndex() : createIndex(row, 0, (void *) tdi));
}



QModelIndex FilesModel::index(int row, int col, const QModelIndex &pnt) const
{
    const TrackDataItem *tdi = dataPointer(pnt);
    if (row>=tdi->childCount())				// only during initialisation
    {							// without SORTABLE_VIEW
        kDebug() << "requested index for nonexistent row!";
        return (QModelIndex());
    }
    return (createIndex(row, col, tdi->childAt(row)));
}



QModelIndex FilesModel::parent(const QModelIndex &idx) const
{
    const TrackDataItem *tdi = dataPointer(idx);
    if (tdi->parent()==NULL) return (QModelIndex());
    return (indexForData(tdi->parent()));
}


int FilesModel::rowCount(const QModelIndex &pnt) const
{
   const TrackDataItem *tdi = dataPointer(pnt);
   return (tdi->childCount());
}


int FilesModel::columnCount(const QModelIndex &pnt) const
{
    return (COL_COUNT);
}




QVariant FilesModel::data(const QModelIndex &idx, int role) const
{
    const TrackDataItem *tdi = dataPointer(idx);
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
case COL_NAME:     
            {
                const TrackDataDisplayable *tdd = dynamic_cast<const TrackDataDisplayable *>(tdi);
                if (tdd!=NULL) return (KIcon(tdd->iconName()));
            }
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
                if (tdf!=NULL) return (i18np("File %2 with %1 track", "File %2 with %1 tracks", tdf->childCount(), tdf->fileName().pathOrUrl()));
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
                const TrackDataPoint *tdp = dynamic_cast<const TrackDataPoint *>(tdi);
                if (tdp!=NULL) return (i18n("Point at %1, elevation %2",
                                            tdp->formattedTime(), tdp->formattedElevation()));
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
    delete mRootItem;					// delete any existing
    mRootItem = new TrackDataRoot("ROOT");		// create new root item
    emit layoutChanged();
}


void FilesModel::addFile(TrackDataFile *tdf)
{
    emit layoutAboutToBeChanged();

    TrackDataFile *fileRoot = NULL;

    if (isEmpty())
    {
        // If the model is currently empty, then first we add a file item
        // immediately under the (invisible) root item.  The input tracks
        // will then be adopted to become children of this.
        //
        // The original metadata from the file will have been copied to the
        // contained tracks by the importer.

        kDebug() << "adding root file item" << tdf->name();
        fileRoot = new TrackDataFile(tdf->name());
        fileRoot->setFileName(tdf->fileName());
        fileRoot->copyMetadata(tdf);

        mRootItem->addChildItem(fileRoot);
    }
    else
    {
        // Get the file root, the first (and only) child of the root item.
        fileRoot = dynamic_cast<TrackDataFile *>(mRootItem->childAt(0));
    }
    Q_ASSERT(fileRoot!=NULL);

    // Now, the tracks contained in the new file are adopted as children
    // of the file root.
    kDebug() << "adding from" << tdf->name() << tdf->childCount() << "tracks";
    while (tdf->childCount()>0)
    {
        TrackDataTrack *tdt = dynamic_cast<TrackDataTrack *>(tdf->takeFirstChildItem());
        if (tdt!=NULL) fileRoot->addChildItem(tdt);
    }

    emit layoutChanged();

    // The file root passed in is now empty, as all its children have been
    // taken away.  Leave it owned by the caller.
}






TrackDataItem *FilesModel::removeLast()
{
    // Remove and return the last track item child of the root file item.
    // TODO: need to make this work for removing the root file item?
    if (rootFileItem()->childCount()==0)
    {
        kDebug() << "!!!!!!!!!!!!!!!!!!!!!!! no tracks to remove";
        return (NULL);
    }

    emit layoutAboutToBeChanged();
    TrackDataFile *tdf = static_cast<TrackDataFile *>(rootFileItem()->takeLastChildItem());
    emit layoutChanged();
    return (tdf);
}


//
//
//
//void FilesModel::removePoint(int row)
//{
//    emit layoutAboutToBeChanged();
//    kDebug() << "removing" << row;
//    mFiles.removeAt(row);
//    emit layoutChanged();
//}
//
//
//void FilesModel::removeFiles(int fromRow, int toRow)
//{
//    if (toRow==-1) toRow = mFiles.count()-1;
//    emit layoutAboutToBeChanged();
//    kDebug() << "removing" << fromRow << "-" << toRow;
//    for (int i = toRow; i>=fromRow; --i) mFiles.removeAt(i);
//    emit layoutChanged();
//}
//
//
//
//
//// the new point becomes row 'row'
//void FilesModel::insertPoint(const PointData *pnt, int row)
//{
//    emit layoutAboutToBeChanged();
//    kDebug() << "inserting" << row;
//    mFiles.insert(row, *pnt);
//    emit layoutChanged();
//}


void FilesModel::changedItem(const TrackDataItem *item)
{
    QModelIndex idx = indexForData(item);
    emit dataChanged(idx, idx);
}


void FilesModel::clickedPoint(const TrackDataPoint *tdp, Qt::KeyboardModifiers mods)
{
    QItemSelectionModel::SelectionFlags selFlags;
    if (mods==Qt::NoModifier) selFlags = QItemSelectionModel::ClearAndSelect;
    else if (mods==Qt::ControlModifier) selFlags = QItemSelectionModel::Toggle;
    else return;

    kDebug() << "click for" << indexForData(tdp) << "flags" << selFlags;
    emit clickedItem(indexForData(tdp), static_cast<unsigned int>(selFlags));
}
