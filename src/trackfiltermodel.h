// -*-mode:c++ -*-

#ifndef TRACKFILTERMODEL_H
#define TRACKFILTERMODEL_H
 
#include <qsortfilterproxymodel.h>


class TrackFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    TrackFilterModel(QObject *pnt = NULL);
    virtual ~TrackFilterModel()				{}

    virtual bool filterAcceptsRow(int row, const QModelIndex &pnt) const;
    virtual Qt::ItemFlags flags(const QModelIndex &idx) const;

private:

};

 
#endif							// TRACKFILTERMODEL_H
