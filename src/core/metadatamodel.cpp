//////////////////////////////////////////////////////////////////////////
//									//
//  Project:	Umbrail - GPX track viewer and editor			//
//									//
//////////////////////////////////////////////////////////////////////////
//									//
//  Copyright (c) 2014-2021 Jonathan Marten <jjm@keelhaul.me.uk>	//
//  Home and download page: <http://github.com/martenjj/umbrail>	//
//									//
//  This program is free software; you can redistribute it and/or	//
//  modify it under the terms of the GNU General Public License as	//
//  published by the Free Software Foundation, either version 3 of	//
//  the License or (at your option) any later version.			//
//									//
//  It is distributed in the hope that it will be useful, but		//
//  WITHOUT ANY WARRANTY;  without even the implied warranty of		//
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the	//
//  GNU General Public License for more details.			//
//									//
//  You should have received a copy of the GNU General Public License	//
//  along with this program;  see the file COPYING for further		//
//  details.  If not, see <http://gnu.org/licenses/gpl>.      		//
//									//
//////////////////////////////////////////////////////////////////////////

#include "metadatamodel.h"

#include <qdebug.h>
#include <qtimezone.h>
#include <qcolor.h>

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
    const int num = DataIndexer::count();
    for (int i = 0; i<num; ++i)
    {
        QVariant v = item->metadata(i);
        if (!v.isNull()) mItemData[i] = v;		// only if present and meaningful
        mItemChanged[i] = false;			// nothing changed yet
    }

    // Copy and record data which is not stored by item metadata.
    //
    // These names are only used here for internal data;  they are not
    // used anywhere outside of this model.  If any are added here then
    // they also need to be ignored in isInternaltag() below.  Any checks
    // for these names elsewhere must use isInternalTag().
    mItemData[DataIndexer::index("name")] = item->name();
    const TrackDataAbstractPoint *tdp = dynamic_cast<const TrackDataAbstractPoint *>(item);
    if (tdp!=nullptr)
    {
        mItemData[DataIndexer::index("latitude")] = tdp->latitude();
        mItemData[DataIndexer::index("longitude")] = tdp->longitude();
    }

    // The default time zone to use is that appropriate for the reference item,
    // which will be obtained by a search of that item and its parents up to
    // the top level.  It may be null, meaning UTC.
    mParentTimeZone = item->timeZone();
    mUseParentTimeZone = (item->parent()!=nullptr);
    qDebug() << "parent time zone" << mParentTimeZone << "use parent?" << mUseParentTimeZone;
    mTimeZone = nullptr;
    resolveTimeZone();
}


/* static */ bool MetadataModel::isInternalTag(const QByteArray &nm)
{
    return (nm=="name" || nm=="latitude" || nm=="longitude");
}


MetadataModel::~MetadataModel()
{
    delete mTimeZone;
}


int MetadataModel::rowCount(const QModelIndex &pnt) const
{
    return (DataIndexer::count());			// this must be the size of all,
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

case Qt::ForegroundRole:
        if (isChanged(row)) return (QColor(Qt::red));
        return (QVariant());

case Qt::DisplayRole:					// formatted display data
        switch (idx.column())
        {
case COL_NAME:
            return (DataIndexer::name(row));
case COL_VALUE:
            switch (v.type())
            {
case QMetaType::QDateTime:
                return (v.toDateTime().toString(Qt::ISODate));

default:        return (v);
            }
        }						// fall through for other roles
        Q_FALLTHROUGH();

default:
        return (QVariant());
    }
}


const QVariant MetadataModel::data(int idx) const
{
    return (mItemData.value(idx));
}


bool MetadataModel::isChanged(int idx) const
{
    return (mItemChanged.value(idx));
}


const QVariant MetadataModel::data(const QByteArray &nm) const
{
    return (data(DataIndexer::index(nm)));
}


bool MetadataModel::setData(const QModelIndex &idx, const QVariant &value, int role)
{
    if (role!=Qt::EditRole) return (false);

    mItemData[idx.row()] = value;
    mItemChanged[idx.row()] = true;
    // QAbstractItemModel::setData() API documentation says that we have to
    // emit this signal explicitly.
    emit dataChanged(idx, idx);
    return (true);
}


void MetadataModel::setData(int idx, const QVariant &value)
{
    mItemData[idx] = value;
    mItemChanged[idx] = true;

    if (idx==DataIndexer::index("timezone")) resolveTimeZone();
							// update time zone data
    emit metadataChanged(idx);				// signal that data changed
}


double MetadataModel::latitude() const
{
    const QVariant &v = data(DataIndexer::index("latitude"));
    return (!v.isNull() ? v.toDouble() : NAN);
}


double MetadataModel::longitude() const
{
    const QVariant &v = data(DataIndexer::index("longitude"));
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


void MetadataModel::resolveTimeZone()
{
    QString name = data("timezone").toString();
    qDebug() << "zone from metadata" << name;

    // The time zone used is either the "timezone" from this item's metadata,
    // or that obtained by the closest ancestor that has it set in its metadata.
    // If the reference item is a top level item (currently a TrackDataFile),
    // then a blank time zone in the item's own metadata means really blank,
    // that is UTC.  If the reference item is not a top level item than the
    // resolved ancestor time zone is used.

    if (name.isEmpty() && mUseParentTimeZone)
    {
        name = mParentTimeZone;
        qDebug() << "using parent zone" << name;
    }

    delete mTimeZone;					// remove existing time zone
    if (name.isEmpty())
    {
        mTimeZone = nullptr;
        qDebug() << "set to no zone";
    }
    else
    {
        mTimeZone = new QTimeZone(name.toLatin1());
        if (!mTimeZone->isValid()) qWarning() << "unknown time zone" << name;
        else qDebug() << "set to" << mTimeZone->id() << "offset" << mTimeZone->offsetFromUtc(QDateTime::currentDateTime());
    }
}
