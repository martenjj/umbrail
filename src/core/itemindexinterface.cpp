
#include "itemindexinterface.h"

#include <qabstractproxymodel.h>
#include <qdebug.h>


ItemIndexInterface::ItemIndexInterface(QAbstractProxyModel *pnt)
{
    mSourceModel = pnt;
}


TrackDataItem *ItemIndexInterface::itemForIndex(const QModelIndex &idx) const
{
    return (itemForSourceIndex(mSourceModel->mapToSource(idx)));
}


TrackDataItem *ItemIndexInterface::itemForSourceIndex(const QModelIndex &idx) const
{
    const ItemIndexInterface *iii = dynamic_cast<const ItemIndexInterface *>(mSourceModel->sourceModel());
    Q_ASSERT(iii!=nullptr);
    return (iii->itemForIndex(idx));
}
