

#include "pointsdatamodel.h"

#include <qdebug.h>

#include <klocalizedstring.h>
#include <kcolorscheme.h>

#include "trackdata.h"
#include "waypointsfiltermodel.h"
#include "filesmodel.h"


PointsDataModel::PointsDataModel(QObject *pnt)
    : QIdentityProxyModel(pnt)
{
    qDebug();
}


TrackDataItem *PointsDataModel::itemForIndex(const QModelIndex &idx) const
{
    const WaypointsFilterModel *wlm = qobject_cast<const WaypointsFilterModel *>(sourceModel());
    Q_ASSERT(wlm!=nullptr);
    return (wlm->itemForIndex(mapToSource(idx)));
}


QVariant PointsDataModel::data(const QModelIndex &idx, int role) const
{
    const TrackDataItem *item = itemForIndex(idx);
    if (item==nullptr) return (QVariant());
    const int col = idx.column();

    if (role==Qt::DecorationRole && col==ColumnName)
    {
        return (QVariant());
    }

    if (role==Qt::ForegroundRole && col==ColumnName)
    {
        const TrackData::WaypointFlags flags = static_cast<TrackData::WaypointFlags>(item->metadata("flags").toInt());
        const KColorScheme sch;
        if (flags & TrackData::NewlyImported) return (sch.foreground(KColorScheme::PositiveText));
        if (flags & TrackData::NoExport) return (sch.foreground(KColorScheme::NegativeText));
    }

    return (QIdentityProxyModel::data(idx, role));
}


Qt::ItemFlags PointsDataModel::flags(const QModelIndex &idx) const
{
    return (QIdentityProxyModel::flags(idx)|Qt::ItemNeverHasChildren);
}
