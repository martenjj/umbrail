
#include "filesmodel.h"

#include <qfont.h>

#include <kdebug.h>
#include <klocale.h>
//#include <kglobalsettings.h>
#include <kicon.h>

#include "trackdata.h"
//#include "filescontroller.h"
//#include "iconsmanager.h"






enum COLUMN
{
    COL_NAME,						// icon/description
//    COL_SYM,						// symbol
//    COL_SOURCE,						// data source ID
//    COL_COORDS,						// lat/long coordinates
//    COL_ADDRESS,					// street address
//    COL_CATS,						// catgeories
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
//case COL_SOURCE:   return (pnt->sources()->join(", "));
//case COL_COORDS:   return (pnt->displayLatLong());
//case COL_ADDRESS:  return (pnt->displayAddress(", "));
//case COL_CATS:     return (pnt->categories()->join(", "));
        }
        break;

case Qt::DecorationRole:
        switch (idx.column())
        {
case COL_NAME:     return (KIcon(tdi->iconName()));
                   break;
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
//case COL_SYM:		return (i18n("Sym"));
//case COL_SOURCE:	return (i18n("Source"));
//case COL_COORDS:	return (i18n("Lat/Long"));
//case COL_ADDRESS:	return (i18n("Address"));
//case COL_CATS:		return (i18n("Categories"));

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
//
//
//
//void FilesModel::addFiles(const FilesList *files, bool clear)
//{
//    emit layoutAboutToBeChanged();
//    kDebug() << "clear" << clear << "appending" << files->count() << "files";
//
//    if (clear) mFiles.clear();
//    for (FilesList::const_iterator it = files->constBegin();
//         it!=files->constEnd(); ++it)
//    {
//        PointData pnt = (*it);
//        mFiles.append(pnt);
//    }
//
//    emit layoutChanged();
//}
//
//
//
void FilesModel::addFile(TrackDataFile *tdf)
{
    emit layoutAboutToBeChanged();
    mRootItem->addChildItem(tdf);			// files always at top level
    emit layoutChanged();
}


TrackDataFile *FilesModel::removeFile()
{
    emit layoutAboutToBeChanged();
    TrackDataFile *tdf = static_cast<TrackDataFile *>(mRootItem->removeLastChildItem());
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




int FilesModel::fileCount() const
{
    return (mRootItem->childCount());
}


TrackDataFile *FilesModel::fileAt(int i) const
{
    return (static_cast<TrackDataFile *>(mRootItem->childAt(i)));
}
