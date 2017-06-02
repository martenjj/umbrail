// -*-mode:c++ -*-

#ifndef METADATAMODEL_H
#define METADATAMODEL_H
 
#include <qabstractitemmodel.h>


class TrackDataItem;


class MetadataModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    MetadataModel(const TrackDataItem *item, QObject *pnt = NULL);
    virtual ~MetadataModel();

    QModelIndex index(int row, int col, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &idx) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &idx, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

private:
    const TrackDataItem *mItem;
};
 
#endif							// METADATAMODEL_H
