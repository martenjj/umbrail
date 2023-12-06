// -*-mode:c++ -*-

#ifndef ITEMINDEXINTERFACE_H
#define ITEMINDEXINTERFACE_H


class QAbstractProxyModel;
class QModelIndex;
class TrackDataItem;


class ItemIndexInterface
{
public:
    explicit ItemIndexInterface(QAbstractProxyModel *pnt);
    virtual ~ItemIndexInterface() = default;

    virtual TrackDataItem *itemForIndex(const QModelIndex &idx) const;

private:
    QAbstractProxyModel *mSourceModel;
};

#endif							// ITEMINDEXINTERFACE_H
