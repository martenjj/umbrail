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
    TrackFilterModel(QObject *pnt = nullptr);
    virtual ~TrackFilterModel()				{}

    virtual bool filterAcceptsRow(int row, const QModelIndex &pnt) const override;
    virtual Qt::ItemFlags flags(const QModelIndex &idx) const override;

    virtual QVariant data(const QModelIndex &idx, int role = Qt::DisplayRole) const override;
    void setSource(const QList<TrackDataItem *> *items);
    void setMode(TrackData::Type mode);

private:
    const QList<TrackDataItem *> *mSourceItems;
    TrackData::Type mMode;
};

 
#endif							// TRACKFILTERMODEL_H