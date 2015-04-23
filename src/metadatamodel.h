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

    virtual QModelIndex index(int row, int col, const QModelIndex &parent = QModelIndex()) const;
    virtual QModelIndex parent(const QModelIndex &idx) const;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex &idx, int role) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;

private:
    const TrackDataItem *mItem;
};
 
#endif							// METADATAMODEL_H
