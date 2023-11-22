// -*-mode:c++ -*-

#ifndef POINTSMODEL_H
#define POINTSMODEL_H

#include <kextracolumnsproxymodel.h>


class TrackDataItem;


/**
 * @short A model to generate the display data for the points list view.
 *
 * Apart from doing that, the KExtraColumnsProxyModel handles most of
 * the work.  It expects its source model to be a WaypointsListModel
 * which presents it with a list of waypoints only.
 */
class PointsModel : public KExtraColumnsProxyModel
{
    Q_OBJECT

public:
    PointsModel(QObject *pnt = nullptr);
    virtual ~PointsModel() = default;

    QVariant data(const QModelIndex &idx, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &idx) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    QVariant extraColumnData(const QModelIndex &pnt, int row, int col, int role) const override;

    TrackDataItem *itemForIndex(const QModelIndex &idx) const;
};
 
#endif							// POINTSMODEL_H
