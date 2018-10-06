
#include "metadatamodel.h"

#include <qdebug.h>

#include <klocalizedstring.h>

#include "trackdata.h"
#include "dataindexer.h"


enum COLUMN
{
    COL_NAME,						// key name
    COL_VALUE,						// value
    COL_COUNT						// how many - must be last
};


MetadataModel::MetadataModel(const TrackDataItem *item, QObject *pnt)
    : QAbstractTableModel(pnt)
{
    qDebug() << "for" << item->name();

    // Copy the existing item metadata.

    const int num = DataIndexer::self()->count();
    for (int i = 0; i<num; ++i)
    {
        QVariant v = item->metadata(i);
        if (!v.isNull()) mItemData[i] = v;		// only if present and meaningful
    }

    // TODO: a static function to access internal index list

    // Copy and record data which is not stored by item metadata.
    //
    // These names are only used here for internal data;  they are not
    // used anywhere outside of this model.  If any are added here then
    // they also need to be ignored in FilesController::slotTrackProperties()
    // and in isInternalTag() in gpxexporter.cpp
    mItemData[DataIndexer::self()->index("name")] = item->name();
    const TrackDataAbstractPoint *tdp = dynamic_cast<const TrackDataAbstractPoint *>(item);
    if (tdp!=NULL)
    {
        mItemData[DataIndexer::self()->index("latitude")] = tdp->latitude();
        mItemData[DataIndexer::self()->index("longitude")] = tdp->longitude();
    }


//     mTimeZone = item->timeZone();			// record for use by pages

}


int MetadataModel::rowCount(const QModelIndex &pnt) const
{
    return (DataIndexer::self()->count());		// this must be the size of all,
}							// not just what we have stored


int MetadataModel::columnCount(const QModelIndex &pnt) const
{
    return (COL_COUNT);
}


QVariant MetadataModel::data(const QModelIndex &idx, int role) const
{
    const int row = idx.row();				// metadata index
    const QVariant &v = data(row);			// reference to data

    switch (role)
    {
case Qt::UserRole:					// raw unformatted data
        return (v);

case Qt::DisplayRole:					// formatted display data
        switch (idx.column())
        {
case COL_NAME:
            return (DataIndexer::self()->name(row));
case COL_VALUE:
            switch (v.type())
            {
case QMetaType::QDateTime:
                return (v.toDateTime().toString(Qt::ISODate));

default:        return (v);
            }
        }						// fall through for other roles

default:
        return (QVariant());
    }
}


const QVariant MetadataModel::data(int idx) const
{
    return (mItemData.value(idx));
}


const QVariant MetadataModel::data(const QString &nm) const
{
    return (data(DataIndexer::self()->index(nm)));
}


bool MetadataModel::setData(const QModelIndex &idx, const QVariant &value, int role)
{
    if (role!=Qt::EditRole) return (false);

    mItemData[idx.row()] = value;
    // QAbstractItemModel::setData() API documentation says that we have to
    // emit this signal explicitly.
    emit dataChanged(idx, idx);
    return (true);
}


void MetadataModel::setData(int idx, const QVariant &value)
{
    mItemData[idx] = value;
    emit metadataChanged(idx);
}


double MetadataModel::latitude() const
{
    const QVariant &v = data(DataIndexer::self()->index("latitude"));
    return (!v.isNull() ? v.toDouble() : NAN);
}


double MetadataModel::longitude() const
{
    const QVariant &v = data(DataIndexer::self()->index("longitude"));
    return (!v.isNull() ? v.toDouble() : NAN);
}


QVariant MetadataModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role!=Qt::DisplayRole) return (QVariant());

    if (orientation==Qt::Horizontal)
    {
        switch (section)
        {
case COL_NAME:	return (i18nc("@title:column", "Name"));
case COL_VALUE:	return (i18nc("@title:column", "Value"));
        }
    }
    else if (orientation==Qt::Vertical)
    {
        return (section);
    }

    return (QVariant());
}
