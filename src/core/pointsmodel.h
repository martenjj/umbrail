// -*-mode:c++ -*-

#ifndef POINTSMODEL_H
#define POINTSMODEL_H
 
#include <qabstractitemmodel.h>


class TrackDataItem;


class PointsModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    PointsModel(QObject *pnt = nullptr);
    virtual ~PointsModel() = default;

    virtual QModelIndex index(int row, int col, const QModelIndex &pnt = QModelIndex()) const override;
    virtual QModelIndex parent(const QModelIndex &idx) const override;
    virtual int rowCount(const QModelIndex &pnt = QModelIndex()) const override;
    virtual int columnCount(const QModelIndex &pnt = QModelIndex()) const override;
    virtual QVariant data(const QModelIndex &idx, int role) const override;
    virtual Qt::ItemFlags flags(const QModelIndex &idx) const override;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    void setSourceModel(QAbstractItemModel *srcModel);

private:
    void buildPointsList(const TrackDataItem *item);

private slots:
    void slotRebuildPointsList();

private:
    QAbstractItemModel *mSourceModel;
    QVector<const TrackDataItem *> mPoints;
};
 
#endif							// POINTSMODEL_H
