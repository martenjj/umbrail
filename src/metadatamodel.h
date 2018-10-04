// -*-mode:c++ -*-

#ifndef METADATAMODEL_H
#define METADATAMODEL_H
 
#include <QAbstractTableModel>


class TrackDataItem;


class MetadataModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    MetadataModel(const TrackDataItem *item, QObject *pnt = nullptr);
    virtual ~MetadataModel() = default;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &idx, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

private:
    const TrackDataItem *mItem;
};
 
#endif							// METADATAMODEL_H
