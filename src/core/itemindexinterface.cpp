
#include "itemindexinterface.h"

#include <qabstractproxymodel.h>
#include <qdebug.h>


ItemIndexInterface::ItemIndexInterface(QAbstractProxyModel *pnt)
{
    mSourceModel = pnt;
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
    const ItemIndexInterface *iii = dynamic_cast<const ItemIndexInterface *>(mSourceModel->sourceModel());
    Q_ASSERT(iii!=nullptr);
    return (iii->itemForIndex(idx));
}
