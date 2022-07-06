// -*-mode:c++ -*-

#ifndef POINTSMODEL_H
#define POINTSMODEL_H
 
#include <qsortfilterproxymodel.h>


class PointsModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    PointsModel(QObject *pnt = NULL);
    virtual ~PointsModel();

//     virtual QModelIndex index(int row, int col, const QModelIndex &pnt = QModelIndex()) const override;
//     virtual QModelIndex parent(const QModelIndex &idx) const override;
//     virtual int rowCount(const QModelIndex &pnt = QModelIndex()) const override;
    virtual int columnCount(const QModelIndex &pnt = QModelIndex()) const override;
    virtual QVariant data(const QModelIndex &idx, int role) const override;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    Qt::ItemFlags flags(const QModelIndex &idx) const override;

    virtual bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
};
 
#endif							// POINTSMODEL_H
