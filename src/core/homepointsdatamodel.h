// -*-mode:c++ -*-

#ifndef HOMEPOINTSDATAMODEL_H
#define HOMEPOINTSDATAMODEL_H

#include <qidentityproxymodel.h>
#include "itemindexinterface.h"


/**
 * @short A model to generate the display data for the home points list view.
 *
 * The source model is expected to be a HomePointsListModel which presents
 * a flat filtered list of home waypoints only.
 */
class HomePointsDataModel : public QIdentityProxyModel, public ItemIndexInterface
{
    Q_OBJECT

public:
    HomePointsDataModel(QObject *pnt = nullptr);
    virtual ~HomePointsDataModel() = default;

    int columnCount(const QModelIndex &pnt) const override;
    QVariant data(const QModelIndex &idx, int role) const override;
    bool setData(const QModelIndex &idx, const QVariant &value, int role) override;
    Qt::ItemFlags flags(const QModelIndex &idx) const override;
    QVariant headerData(int section, Qt::Orientation orient, int role) const override;

    QString homePoint() const;
    QString workPoint() const;

private:
    int mHomeIndex;
    int mWorkIndex;
};
 
#endif							// HOMEPOINTSDATAMODEL_H
