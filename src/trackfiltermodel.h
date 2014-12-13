// -*-mode:c++ -*-

#ifndef TRACKFILTERMODEL_H
#define TRACKFILTERMODEL_H
 
#include <qsortfilterproxymodel.h>

#include "trackdata.h"


class TrackDataItem;


class TrackFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    TrackFilterModel(QObject *pnt = NULL);
    virtual ~TrackFilterModel()				{}

    virtual bool filterAcceptsRow(int row, const QModelIndex &pnt) const;
    virtual Qt::ItemFlags flags(const QModelIndex &idx) const;

    virtual QVariant data(const QModelIndex &idx, int role = Qt::DisplayRole) const;
    void setSource(const QList<TrackDataItem *> *items);

private:
    const QList<TrackDataItem *> *mSourceItems;
    TrackData::Type mMode;
};

 
#endif							// TRACKFILTERMODEL_H
