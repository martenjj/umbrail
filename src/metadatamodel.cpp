
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
							// copy existing item data
    const int num = DataIndexer::self()->count();
    for (int i = 0; i<num; ++i) mItemData.append(item->metadata(i));

    mTimeZone = item->timeZone();			// record for use by pages

//     mUpdatesIgnored = false;

    // Copy and record data which is not stored by item metadata.



    // Allocate these indexes for internal data;  they are not used anywhere
    // outside of this model.  If any are added here then they also need to
    // be ignored in FilesController::slotTrackProperties() and in
    // isInternalTag() in gpxexporter.cpp
    // const int nameIdx = DataIndexer::self()->index("name");
    // const int latIdx = DataIndexer::self()->index("latitude");
    // const int lonIdx = DataIndexer::self()->index("longitude");

    setData(DataIndexer::self()->index("name"), item->name());
    // mItemData[nameIdx] = item->name();
    const TrackDataAbstractPoint *tdp = dynamic_cast<const TrackDataAbstractPoint *>(item);
    if (tdp!=NULL)
    {
        setData(DataIndexer::self()->index("latitude"), tdp->latitude());
        setData(DataIndexer::self()->index("longitude"), tdp->longitude());
        // mItemData[latIdx] = tdp->latitude();
        // mItemData[lonIdx] = tdp->longitude();
    }

}


int MetadataModel::rowCount(const QModelIndex &pnt) const
{
//    qDebug() << "returning" << mItemData.count();
    return (mItemData.size());
}


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
    if (idx>=mItemData.size()) return (QVariant());	// nothing stored for that
    return (mItemData[idx]);
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
//     if (mUpdatesIgnored) return;			// ignore this change

    const int oldSize = mItemData.size();		// current size of array
    const int newSize = idx+1;				// size that is required

    bool doReset = false;				// not needed yet, anyway
    if (newSize>oldSize)				// needs to be extended
    {
        qDebug() << "need to extend to size" << newSize;
        mItemData.resize(newSize);
        doReset = true;
        beginResetModel();
    }

    mItemData[idx] = value;
    if (doReset) endResetModel();
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


// void MetadataModel::setName(const QString &value)
// {
    // if (mUpdatesIgnored) return;
    // mName = value;
// ///////////////////////////////////////////////////////////////////////// TODO: temp
    // emit metadataChanged(0);
// }


// void MetadataModel::ignoreUpdates(bool ignore)
// {
//     qDebug() << "ignore?" << ignore;
//     mUpdatesIgnored = ignore;
// }






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
