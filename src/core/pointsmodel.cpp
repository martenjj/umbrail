
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
    : QSortFilterProxyModel(pnt)
{
    qDebug();

    //    setRecursiveFilteringEnabled(true);
}


PointsModel::~PointsModel()
{
    qDebug() << "done";
}


// QModelIndex PointsModel::index(int row, int col, const QModelIndex &pnt) const
// {
//     return (createIndex(row, col, row));
// }
// 
// 
// QModelIndex PointsModel::parent(const QModelIndex &idx) const
// {
//     return (QModelIndex());
// }
// 
// 
// int PointsModel::rowCount(const QModelIndex &pnt) const
// {
// 
//     int c = sourceModel()->rowCount(mapToSource(pnt));
//     qDebug() << "########## idx" << pnt << "=" << c;
//     return (c);
// 
// 
// 
// 
// }


int PointsModel::columnCount(const QModelIndex &pnt) const
{
    return (COL_COUNT);
}


QVariant PointsModel::data(const QModelIndex &idx, int role) const
{
    return (sourceModel()->data(mapToSource(idx), role));

//     const PointData *p = &mPoints.at(idx.row());
// 
//     switch (role)
//     {
// case Qt::DisplayRole:
//         switch (idx.column())
//         {
// case COL_NAME:     return (p->displayName());
// case COL_SOURCE:   return (p->sources()->join(", "));
// case COL_COORDS:   return (p->displayLatLong());
// case COL_ADDRESS:  return (p->displayAddress(", "));
// case COL_CATS:     return (p->categories()->join(", "));
//         }
//         break;
// 
// case Qt::DecorationRole:
//         switch (idx.column())
//         {
// case COL_SYM:      const QImage *img = controller()->iconsManager()->iconForName(p->symbol());
//                    if (img!=NULL) return (img->scaled(SIZE_ICON));
//                    break;
//         }
//         break;
// 
// case Qt::FontRole:
//         switch (idx.column())
//         {
// case COL_COORDS:   return (QFontDatabase::systemFont(QFontDatabase::FixedFont));
//         }
//         break;
// 
// case Qt::ForegroundRole:
//         switch (idx.column())
//         {
// case COL_NAME:     if (p->flags() & PointData::NewlyImported) return (QColor(Qt::green));
//                    if (p->flags() & PointData::NoExport) return (QColor(Qt::red));
//         };
//         break;
// 
// case Qt::ToolTipRole:
//         switch (idx.column())
//         {
// case COL_SYM:      return (p->symbol());
// case COL_SOURCE:   return (p->sources()->join("<br/>"));
//         }
//         break;
// 
// case Qt::UserRole:					// data for sorting
//         switch (idx.column())
//         {
// case COL_SYM:      return (p->symbol());
// default:           return (data(idx, Qt::DisplayRole));
//         }
//         break;
// 
// case Qt::SizeHintRole:
//         switch (idx.column())
//         {
// case COL_SYM:      return (SIZE_HINT);
//         }
//         break;
//     }
// 
//     return (QVariant());
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
    return (Qt::ItemIsSelectable|Qt::ItemIsEnabled);
}


bool PointsModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    QModelIndex sourceIndex = sourceModel()->index(sourceRow, 0, sourceParent);
//     qDebug() << "---------------- si" << sourceIndex;

    QAbstractProxyModel *sm = qobject_cast<QAbstractProxyModel *>(sourceModel());
//     qDebug() << "---------------- sm" << sm;

    QModelIndex si2 = sm->mapToSource(sourceIndex);
//     qDebug() << "---------------- si2" << si2;

    const TrackDataItem *item = FilesModel::itemForIndex(si2);
    Q_ASSERT(item!=nullptr);

    return (dynamic_cast<const TrackDataWaypoint *>(item)!=nullptr);
}
