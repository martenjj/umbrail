// -*-mode:c++ -*-

#ifndef HOMEPOINTSDATAMODEL_H
#define HOMEPOINTSDATAMODEL_H

#include <kextracolumnsproxymodel.h>


class TrackDataItem;


/**
 * @short A model to generate the display data for the home points list view.
 *
 * The source model is expected to be a HomePointsListModel which presents
 * a filtered list of home waypoints only.
 */
class HomePointsDataModel : public KExtraColumnsProxyModel
{
    Q_OBJECT

public:
    HomePointsDataModel(QObject *pnt = nullptr);
    virtual ~HomePointsDataModel() = default;

    QVariant data(const QModelIndex &idx, int role) const override;
    bool setData(const QModelIndex &idx, const QVariant &value, int role) override;
    Qt::ItemFlags flags(const QModelIndex &idx) const override;
    QVariant headerData(int section, Qt::Orientation orient, int role) const override;
    QVariant extraColumnData(const QModelIndex &pnt, int row, int col, int role) const override;

    TrackDataItem *itemForIndex(const QModelIndex &idx) const;

    QString homePoint() const;
    QString workPoint() const;

private:
    int mHomeIndex;
    int mWorkIndex;
};
 
#endif							// HOMEPOINTSDATAMODEL_H
