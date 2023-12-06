
#include "itemindexinterface.h"

#include <qabstractproxymodel.h>
#include <qdebug.h>


ItemIndexInterface::ItemIndexInterface(QAbstractProxyModel *pnt)
{
    //qDebug() << "############ on" << pnt;
    mSourceModel = pnt;
}


TrackDataItem *ItemIndexInterface::itemForIndex(const QModelIndex &idx) const
{
    //qDebug() << "#######################################################";
    //qDebug() << "sm" << mSourceModel;

    const ItemIndexInterface *iii = dynamic_cast<const ItemIndexInterface *>(mSourceModel->sourceModel());
    //qDebug() << "ii" << iii;

    Q_ASSERT(iii!=nullptr);
    return (iii->itemForIndex(mSourceModel->mapToSource(idx)));
}
