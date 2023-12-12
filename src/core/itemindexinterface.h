// -*-mode:c++ -*-

#ifndef ITEMINDEXINTERFACE_H
#define ITEMINDEXINTERFACE_H


class QAbstractItemModel;
class QAbstractProxyModel;
class QModelIndex;
class TrackDataItem;


/**
 * @short An interface to implement itemForIndex() for a proxy model.
 *
 * Most models in the model tree need to access the @c TrackDataItem
 * corresponding to a model index.  They do not know which or how many
 * models eventually leading to the FilesModel are below them, so this
 * interface passes such a request down the chain by calling the
 * corresponding function in the source model.  The root FilesModel
 * implements its own itemForIndex() function which actually returns
 * the requested item.
 **/
class ItemIndexInterface
{
public:
    explicit ItemIndexInterface(QAbstractProxyModel *mod);
    virtual ~ItemIndexInterface() = default;

    virtual TrackDataItem *itemForIndex(const QModelIndex &idx) const;
    virtual TrackDataItem *itemForSourceIndex(const QModelIndex &idx) const;
    virtual QModelIndex indexForItem(const TrackDataItem *item) const;

    static const ItemIndexInterface *of(QAbstractItemModel *mod);

private:
    QAbstractProxyModel *mSourceModel;
};

#endif							// ITEMINDEXINTERFACE_H
