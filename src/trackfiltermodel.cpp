
#include "trackfiltermodel.h"

#include <qfont.h>

#include <kdebug.h>

#include "filesmodel.h"
#include "trackdata.h"



TrackFilterModel::TrackFilterModel(QObject *pnt)
    : QSortFilterProxyModel(pnt)
{
    mSourceSegment = NULL;
}



void TrackFilterModel::setSourceSegment(const TrackDataSegment *tds)
{
    mSourceSegment = tds;
}



bool TrackFilterModel::filterAcceptsRow(int row, const QModelIndex &pnt) const
{
    FilesModel *filesModel = qobject_cast<FilesModel *>(sourceModel());
    QModelIndex idx = filesModel->index(row, 0, pnt);
    const TrackDataItem *item = filesModel->itemForIndex(idx);

    if (dynamic_cast<const TrackDataFile *>(item)!=NULL) return (true);
    if (dynamic_cast<const TrackDataTrack *>(item)!=NULL) return (true);
    if (dynamic_cast<const TrackDataSegment *>(item)!=NULL) return (true);
    return (false);
}



Qt::ItemFlags TrackFilterModel::flags(const QModelIndex &idx) const
{
    FilesModel *filesModel = qobject_cast<FilesModel *>(sourceModel());
    const TrackDataItem *item = filesModel->itemForIndex(mapToSource(idx));

    if (dynamic_cast<const TrackDataFile *>(item)!=NULL) return (Qt::ItemIsEnabled);
    if (dynamic_cast<const TrackDataTrack *>(item)!=NULL) return (Qt::ItemIsSelectable|Qt::ItemIsEnabled);
    return (Qt::NoItemFlags);
}



QVariant TrackFilterModel::data(const QModelIndex &idx, int role) const
{
    if (role!=Qt::FontRole) return (QSortFilterProxyModel::data(idx, role));

    FilesModel *filesModel = qobject_cast<FilesModel *>(sourceModel());
    const TrackDataItem *item = filesModel->itemForIndex(mapToSource(idx));
    const TrackDataSegment *tds = dynamic_cast<const TrackDataSegment *>(item);
    if (tds==NULL || tds!=mSourceSegment) return (QSortFilterProxyModel::data(idx, role));

    QFont f = QSortFilterProxyModel::data(idx,role).value<QFont>();
    f.setBold(true);
    return (f);
}
