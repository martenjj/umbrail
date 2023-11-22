

#include "homepointsdatamodel.h"

#include <qdebug.h>
#include <qicon.h>

#include <klocalizedstring.h>

#include "trackdata.h"
#include "pointicon.h"
#include "homepointsfiltermodel.h"


enum COLUMN
{
    COL_HOME,						// "Home" check box
    COL_WORK,						// "Work" check box
    COL_NAME,						// name
    COL_COUNT						// how many - must be last
};


HomePointsDataModel::HomePointsDataModel(QObject *pnt)
    : KExtraColumnsProxyModel(pnt)
{
    qDebug();
    for (int i = 1; i<COL_COUNT; ++i) appendColumn();

    // TODO: eliminate, use QAbstractItemModel::match()
    mHomeIndex = -1;					// nothing selected yet
    mWorkIndex = -1;
}


TrackDataItem *HomePointsDataModel::itemForIndex(const QModelIndex &idx) const
{
    const HomePointsFilterModel *hfm = qobject_cast<const HomePointsFilterModel *>(sourceModel());
    Q_ASSERT(hfm!=nullptr);
    return (hfm->itemForIndex(hfm->index(idx.row(), 0, idx.parent())));
}


QVariant HomePointsDataModel::extraColumnData(const QModelIndex &pnt, int row, int col, int role) const
{
    // This should never actually be called, because we override data()
    // and headerData() and return results from those for all columns.
    return (QVariant());
}


QVariant HomePointsDataModel::data(const QModelIndex &idx, int role) const
{
    // Because this generates data for all columns of the model, the rearranged
    // column order does not matter.
    const int col = idx.column();
    const TrackDataItem *item = itemForIndex(idx);

    switch (role)
    {
case Qt::DisplayRole:
        if (col==COL_NAME) return (item==nullptr ? i18nc("No point selected", "(none)") : item->name());
        break;

case Qt::DecorationRole:
        if (col==COL_NAME) return (item==nullptr ? QIcon::fromTheme("edit-delete") : item->icon()->icon());
        break;

case Qt::CheckStateRole:
        if (col==COL_HOME) return (idx.row()==mHomeIndex ? Qt::Checked : Qt::Unchecked);
        if (col==COL_WORK) return (idx.row()==mWorkIndex ? Qt::Checked : Qt::Unchecked);
        break;
    }

    return (QVariant());
}


QVariant HomePointsDataModel::headerData(int section, Qt::Orientation orient, int role) const
{
    if (orient!=Qt::Horizontal) return (QVariant());

    if (role==Qt::DisplayRole)
    {
        if (section==COL_NAME) return (i18n("Point"));
    }

    if (role==Qt::DecorationRole)
    {
        if (section==COL_HOME) return (QIcon::fromTheme("go-home"));
        if (section==COL_WORK) return (QIcon::fromTheme("meeting-participant"));
    }

    return (QVariant());
}


bool HomePointsDataModel::setData(const QModelIndex &idx, const QVariant &value, int role)
{
    const int row = idx.row();
    const int col = idx.column();

    qDebug() << "row" << row << "col" << idx.column() << "role" << role << "value" << value;

    if (role==Qt::CheckStateRole)
    {
        if (col==COL_HOME) mHomeIndex = (value==Qt::Checked ? row : -1);
        if (col==COL_WORK) mWorkIndex = (value==Qt::Checked ? row : -1);

        // The check boxes are exclusive, so the entire column needs
        // to be marked as changed when one is toggled.
        //
        // The Qt API documentation does not explicitly say that the
        // 'topLeft' and 'bottomRight' signal parameters can be a null
        // QModelIndex, but it seems to work.  Using what would seem to
        // be the obvious way of generating indexes to span the entire
        // column:
        //
        //   emit dataChanged(createIndex(0, col), createIndex(rowCount(QModelIndex())-1, col));
        //
        // causes an assert within QSortFilterProxyModel.
        emit dataChanged(QModelIndex(), QModelIndex());
    }

    return (true);
}


Qt::ItemFlags HomePointsDataModel::flags(const QModelIndex &idx) const
{
    Qt::ItemFlags f = Qt::ItemIsEnabled|Qt::ItemNeverHasChildren;
    if (idx.column()==COL_HOME || idx.column()==COL_WORK) f |= Qt::ItemIsUserCheckable;
    return (f);
}


QString HomePointsDataModel::homePoint() const
{
    return (mHomeIndex<0 ? QString() : itemForIndex(index(mHomeIndex, 0))->name());
}


QString HomePointsDataModel::workPoint() const
{
    return (mWorkIndex<0 ? QString() : itemForIndex(index(mWorkIndex, 0))->name());
}
