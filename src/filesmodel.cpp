
#include "filesmodel.h"

#include <qfont.h>
#include <qicon.h>
#include <qitemselectionmodel.h>
#include <qdebug.h>

#include <klocalizedstring.h>
#include <kcolorscheme.h>

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
    qDebug();
    mRootFileItem = NULL;
}


FilesModel::~FilesModel()
{
    delete mRootFileItem;
    qDebug() << "done";
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
        qDebug() << "requested index for nonexistent row" << row << "of" << tdi->childCount();
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
case COL_NAME:     return (QIcon::fromTheme(tdi->iconName()));
        }
        break;

case Qt::ForegroundRole:
        switch (idx.column())
        {
case COL_NAME:
            QString status = tdi->metadata("status");
            if (!status.isEmpty())
            {
                TrackData::WaypointStatus s = static_cast<TrackData::WaypointStatus>(status.toInt());
                KColorScheme sch(QPalette::Normal);
                if (s==TrackData::StatusTodo) return (sch.foreground(KColorScheme::NegativeText));
                if (s==TrackData::StatusDone) return (sch.foreground(KColorScheme::PositiveText));
                if (s==TrackData::StatusQuestion) return (sch.foreground(KColorScheme::NeutralText));
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
                if (tdf!=NULL) return (i18np("File %2 with %1 item", "File %2 with %1 items", tdf->childCount(), tdf->fileName().toDisplayString()));
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


TrackDataFile *FilesModel::takeRootFileItem()
{
    TrackDataFile *root = mRootFileItem;
    Q_ASSERT(root!=NULL);
    beginResetModel();
    mRootFileItem = NULL;
    endResetModel();
    qDebug() << "removing root" << root->name();
    return (root);
}


void FilesModel::setRootFileItem(TrackDataFile *root)
{
    Q_ASSERT(mRootFileItem==NULL);
    beginResetModel();
    qDebug() << "setting root" << root->name();
    mRootFileItem = root;
    endResetModel();
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

    qDebug() << "click for" << indexForItem(tdp) << "flags" << selFlags;
    emit clickedItem(indexForItem(tdp), static_cast<unsigned int>(selFlags));
}
