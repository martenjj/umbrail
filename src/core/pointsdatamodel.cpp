

#include "pointsdatamodel.h"

#include <qfontdatabase.h>
#include <qdebug.h>
#include <qimage.h>
#include <qicon.h>

#include <klocalizedstring.h>
#include <kcolorscheme.h>

#include "trackdata.h"
#include "pointicon.h"
#include "waypointsfiltermodel.h"


enum COLUMN
{
    COL_NAME,						// name
    COL_SYM,						// symbol
    COL_ORIGIN,						// data source ID
    COL_COORDS,						// lat/long coordinates
    COL_ADDRESS,					// street address
    COL_CATS,						// catgeories
    COL_COUNT						// how many - must be last
};


#define SIZE_ICON		QSize(16, 16)
#define SIZE_HINT		QSize(18, 18)


PointsDataModel::PointsDataModel(QObject *pnt)
    : KExtraColumnsProxyModel(pnt)
{
    qDebug();
    for (int i = 1; i<COL_COUNT; ++i) appendColumn();
}



TrackDataItem *PointsDataModel::itemForIndex(const QModelIndex &idx) const
{
    const WaypointsFilterModel *wlm = qobject_cast<const WaypointsFilterModel *>(sourceModel());
    Q_ASSERT(wlm!=nullptr);

    // Do not use mapToSource(), because the source model will generate
    // an invalid index for the extra columns.  We know that the row
    // numbers are the same in the two models.
    return (wlm->itemForIndex(wlm->index(idx.row(), 0, idx.parent())));
}


QVariant PointsDataModel::extraColumnData(const QModelIndex &pnt, int row, int col, int role) const
{
    // This should never actually be called, because we override data()
    // and headerData() and return results from those for all columns.
    return (QVariant());
}


// TODO: can use TrackData::formattedLatLong()
static QVariant formatCoordinates(const TrackDataItem *item)
{
    const TrackDataAbstractPoint *tdp = dynamic_cast<const TrackDataAbstractPoint *>(item);
    if (tdp==nullptr) return (QVariant());
    return (tdp->formattedPosition());
}


// TODO: can use TrackData::formattedAddress()
// Based on NavMarks PointData::displayAddress()
static QVariant formatAddress(const TrackDataItem *item)
{
    QStringList result;

    // "StreetAddress", which may be multiple lines
    const QString street = item->metadata("StreetAddress").toString();
    if (!street.isEmpty()) result.append(street.split("\n", Qt::SkipEmptyParts));

    // "City"
    const QString city = item->metadata("City").toString();
    if (!city.isEmpty()) result.append(city);

    // "State", if not the same as "City"
    // and not the same as the first two of "PostalCode" (France departement)
    const QString state = item->metadata("State").toString();
    const QString pcode = item->metadata("PostalCode").toString();
    if (!state.isEmpty() && state!=city &&
        !(state.length()==2 && state==pcode.left(2))) result.append(state);

    const QString cntry =  item->metadata("Country").toString();
    if (!pcode.isEmpty() && !cntry.isEmpty())
    {
        // "PostalCode - Country" if both are present
        result.append(pcode+" - "+cntry);
    }
    else
    {
        // "PostalCode" or "Country"
        if (!pcode.isEmpty()) result.append(pcode);
        if (!cntry.isEmpty()) result.append(cntry);
    }

    return (result.join(", "));
}


QVariant PointsDataModel::data(const QModelIndex &idx, int role) const
{
    const TrackDataItem *item = itemForIndex(idx);
    if (item==nullptr) return (QVariant());
    const int col = idx.column();

    switch (role)
    {
case Qt::DisplayRole:
        switch (col)
        {
case COL_NAME:     return (item->name());
case COL_ORIGIN:   return (item->metadata("origin").toStringList().join(", "));
case COL_COORDS:   return (formatCoordinates(item));
case COL_ADDRESS:  return (formatAddress(item));
case COL_CATS:     return (item->metadata("category").toStringList().join(", "));
        }
        break;

case Qt::DecorationRole:
        if (col==COL_SYM) return (item->icon()->icon());
        break;

case Qt::FontRole:
        if (col==COL_COORDS) return (QFontDatabase::systemFont(QFontDatabase::FixedFont));
        break;

case Qt::ForegroundRole:
        if (col==COL_NAME)
        {
            const KColorScheme sch;
            const TrackData::WaypointFlags flags = static_cast<TrackData::WaypointFlags>(item->metadata("flags").toInt());

            if (flags & TrackData::NewlyImported) return (sch.foreground(KColorScheme::PositiveText));
            if (flags & TrackData::NoExport) return (sch.foreground(KColorScheme::NegativeText));
        }
        break;

case Qt::ToolTipRole:
        switch (col)
        {
case COL_SYM:      return (item->icon()->name());
case COL_ORIGIN:   return (item->metadata("origin").toStringList().join("<br/>"));
        }
        break;

case Qt::UserRole:					// data for sorting
        if (col==COL_SYM) return (item->icon()->name());
        else return (data(idx, Qt::DisplayRole));
        break;

case Qt::SizeHintRole:
        if (col==COL_SYM) return (SIZE_HINT);
        break;
    }

    return (QVariant());
}


QVariant PointsDataModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role!=Qt::DisplayRole) return (QVariant());
    if (orientation!=Qt::Horizontal) return (QVariant());

    switch (section)
    {
case COL_NAME:		return (i18n("Name"));
case COL_SYM:		return (i18n("Sym"));
case COL_ORIGIN:	return (i18n("Origin"));
case COL_COORDS:	return (i18n("Lat/Long"));
case COL_ADDRESS:	return (i18n("Address"));
case COL_CATS:		return (i18n("Categories"));

default:		return (QVariant());
    }
}


Qt::ItemFlags PointsDataModel::flags(const QModelIndex &idx) const
{
    return (Qt::ItemIsSelectable|Qt::ItemIsEnabled|Qt::ItemNeverHasChildren);
}
