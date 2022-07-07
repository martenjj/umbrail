
#include "pointsmodel.h"

#include <qfontdatabase.h>
#include <qdebug.h>
#include <qimage.h>

#include <klocalizedstring.h>

#include "trackdata.h"
#include "filesmodel.h"


enum COLUMN
{
    COL_NAME,						// name
    COL_SYM,						// symbol
    COL_SOURCE,						// data source ID
    COL_COORDS,						// lat/long coordinates
    COL_ADDRESS,					// street address
    COL_CATS,						// catgeories
    COL_COUNT						// how many - must be last
};


#define SIZE_ICON		QSize(16, 16)
#define SIZE_HINT		QSize(18, 18)


PointsModel::PointsModel(QObject *pnt)
    : QAbstractItemModel(pnt)
{
    qDebug();
}


QModelIndex PointsModel::index(int row, int col, const QModelIndex &pnt) const
{
    return (createIndex(row, col, row));
}


QModelIndex PointsModel::parent(const QModelIndex &idx) const
{
    return (QModelIndex());
}


int PointsModel::rowCount(const QModelIndex &pnt) const
{
    return (mPoints.count());
}


int PointsModel::columnCount(const QModelIndex &pnt) const
{
    return (COL_COUNT);
}


QVariant PointsModel::data(const QModelIndex &idx, int role) const
{
    const TrackDataItem *item = mPoints.value(idx.row());
    if (item==nullptr) return (QVariant());

    switch (role)
    {
case Qt::DisplayRole:
        switch (idx.column())
        {
case COL_NAME:     return (item->name());
// case COL_SOURCE:   return (p->sources()->join(", "));

case COL_COORDS:   {
                       const TrackDataAbstractPoint *tdp = dynamic_cast<const TrackDataAbstractPoint *>(item);
                       if (tdp==nullptr) return (QVariant());
                       return (tdp->formattedPosition());
                   }

// case COL_ADDRESS:  return (p->displayAddress(", "));
case COL_CATS:     return (item->metadata("category"));
        }
return QString("R%1 C%2").arg(idx.row()).arg(idx.column());
        break;

case Qt::DecorationRole:
//         switch (idx.column())
//         {
// case COL_SYM:      const QImage *img = controller()->iconsManager()->iconForName(p->symbol());
//                    if (img!=NULL) return (img->scaled(SIZE_ICON));
//                    break;
//         }
        break;

case Qt::FontRole:
        switch (idx.column())
        {
case COL_COORDS:   return (QFontDatabase::systemFont(QFontDatabase::FixedFont));
        }
        break;

case Qt::ForegroundRole:
//         switch (idx.column())
//         {
// case COL_NAME:     if (p->flags() & PointData::NewlyImported) return (QColor(Qt::green));
//                    if (p->flags() & PointData::NoExport) return (QColor(Qt::red));
//         };
        break;

case Qt::ToolTipRole:
//         switch (idx.column())
//         {
// case COL_SYM:      return (p->symbol());
// case COL_SOURCE:   return (p->sources()->join("<br/>"));
//         }
        break;

case Qt::UserRole:					// data for sorting
        switch (idx.column())
        {
// case COL_SYM:      return (p->symbol());
default:           return (data(idx, Qt::DisplayRole));
        }
        break;

case Qt::SizeHintRole:
        switch (idx.column())
        {
case COL_SYM:      return (SIZE_HINT);
        }
        break;
    }

    return (QVariant());
}


QVariant PointsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role!=Qt::DisplayRole) return (QVariant());
    if (orientation!=Qt::Horizontal) return (QVariant());

    switch (section)
    {
case COL_NAME:		return (i18n("Name"));
case COL_SYM:		return (i18n("Sym"));
case COL_SOURCE:	return (i18n("Source"));
case COL_COORDS:	return (i18n("Lat/Long"));
case COL_ADDRESS:	return (i18n("Address"));
case COL_CATS:		return (i18n("Categories"));

default:		return (QVariant());
    }
}


Qt::ItemFlags PointsModel::flags(const QModelIndex &idx) const
{
    return (Qt::ItemIsSelectable|Qt::ItemIsEnabled|Qt::ItemNeverHasChildren);
}


void PointsModel::setSourceModel(QAbstractItemModel *srcModel)
{
    qDebug();

    mSourceModel = srcModel;
    connect(srcModel, &QAbstractItemModel::modelReset, this, &PointsModel::slotRebuildPointsList);
    connect(srcModel, &QAbstractItemModel::layoutChanged, this, &PointsModel::slotRebuildPointsList);

    slotRebuildPointsList();
}


void PointsModel::slotRebuildPointsList()
{
    qDebug();
    beginResetModel();

    mPoints.clear();

    FilesModel *filesModel = qobject_cast<FilesModel *>(mSourceModel);
    Q_ASSERT(filesModel!=nullptr);
    const TrackDataItem *root = filesModel->rootFileItem();
    if (root!=nullptr) buildPointsList(root);		// may not have been set yet

    endResetModel();
    qDebug() << "total points" << mPoints.count();
}


void PointsModel::buildPointsList(const TrackDataItem *item)
{
    if (dynamic_cast<const TrackDataWaypoint *>(item)!=nullptr) mPoints.append(item);
    else
    {
        const int n = item->childCount();
        for (int i = 0; i<n; ++i) buildPointsList(item->childAt(i));
    }
}


const TrackDataItem *PointsModel::itemAt(int row)
{
    return (mPoints.value(row));
}
