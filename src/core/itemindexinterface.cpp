
#include "itemindexinterface.h"

#include <qabstractproxymodel.h>


ItemIndexInterface::ItemIndexInterface(QAbstractProxyModel *mod)
{
    // If the 'mod' model is NULL, this model must implement the interface
    // itself.  This means that it must implement itemForIndex() and
    // indexForItem() and never use itemForSourceIndex().
    mSourceModel = mod;
}


TrackDataItem *ItemIndexInterface::itemForIndex(const QModelIndex &idx) const
{
    // The 'idx' here refers to the current model, so it needs to be
    // mapped to the source model.
    return (itemForSourceIndex(mSourceModel->mapToSource(idx)));
}


TrackDataItem *ItemIndexInterface::itemForSourceIndex(const QModelIndex &idx) const
{
    // The 'idx' here already refers to the source model, so there is
    // therefore no need to map it as above.
    return (of(mSourceModel->sourceModel())->itemForIndex(idx));
}


QModelIndex ItemIndexInterface::indexForItem(const TrackDataItem *item) const
{
    return (mSourceModel->mapFromSource(of(mSourceModel->sourceModel())->indexForItem(item)));
}


TrackDataContainer *ItemIndexInterface::rootItem() const
{
    return (of(mSourceModel->sourceModel())->rootItem());
}


/* static */ const ItemIndexInterface *ItemIndexInterface::of(QAbstractItemModel *mod)
{
    // Although in this application all views' source models are proxy models,
    // the parameter here is a QAbstractItemModel so that it can be applied
    // directly to the model returned by a view's model() function.  This will
    // therefore work even if the view's source model is the master tree model.

    const ItemIndexInterface *iii = dynamic_cast<const ItemIndexInterface *>(mod);
    Q_ASSERT(iii!=nullptr);
    return (iii);
}
