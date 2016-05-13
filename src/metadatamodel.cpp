
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
    : QAbstractItemModel(pnt)
{
    qDebug() << "for" << item->name();
    mItem = item;
}


MetadataModel::~MetadataModel()
{
}


QModelIndex MetadataModel::index(int row, int col, const QModelIndex &pnt) const
{
    return (createIndex(row, col));
}


QModelIndex MetadataModel::parent(const QModelIndex &idx) const
{
    return (QModelIndex());
}


int MetadataModel::rowCount(const QModelIndex &pnt) const
{
    return (DataIndexer::self()->count());
}


int MetadataModel::columnCount(const QModelIndex &pnt) const
{
    return (COL_COUNT);
}


QVariant MetadataModel::data(const QModelIndex &idx, int role) const
{
    if (role!=Qt::DisplayRole) return (QVariant());

    switch (idx.column())
    {
case COL_NAME:     return (DataIndexer::self()->name(idx.row()));
case COL_VALUE:    return (mItem->metadata(idx.row()));
    }

    return (QVariant());
}


QVariant MetadataModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role!=Qt::DisplayRole) return (QVariant());

    if (orientation==Qt::Horizontal)
    {
        switch (section)
        {
case COL_NAME:	return (i18n("Name"));
case COL_VALUE:	return (i18n("Value"));
        }
    }
    else if (orientation==Qt::Vertical)
    {
        return (section);
    }

    return (QVariant());
}
