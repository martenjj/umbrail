// -*-mode:c++ -*-

#ifndef POINTSDATAMODEL_H
#define POINTSDATAMODEL_H

#include <qidentityproxymodel.h>


class TrackDataItem;


/**
 * @short A model to generate the display data for the points list view.
 *
 * The underlying FilesModel does most of the formatting work; this model
 * just needs to generate data specific to this view.  It expects its
 * source model to be a WaypointsFilterModel which presents it with a list
 * of waypoints only.
 */
class PointsDataModel : public QIdentityProxyModel
{
    Q_OBJECT

public:
    PointsDataModel(QObject *pnt = nullptr);
    virtual ~PointsDataModel() = default;

    QVariant data(const QModelIndex &idx, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &idx) const override;

    TrackDataItem *itemForIndex(const QModelIndex &idx) const;
};

#endif							// POINTSDATAMODEL_H
