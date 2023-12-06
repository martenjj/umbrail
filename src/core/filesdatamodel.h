// -*-mode:c++ -*-

#ifndef FILESDATAMODEL_H
#define FILESDATAMODEL_H

#include <qidentityproxymodel.h>


class TrackDataItem;


/**
 * @short A model to produce the display data for the main tree view.
 *
 * The underlying FilesModel does most of the work; this model just needs
 * to trim the columns down to one and generate data specific to this
 * view.
 */
class FilesDataModel : public QIdentityProxyModel
{
    Q_OBJECT

public:
    FilesDataModel(QObject *pnt = nullptr);
    virtual ~FilesDataModel() = default;

    QVariant data(const QModelIndex &idx, int role) const override;
    int columnCount(const QModelIndex &pnt) const override;

    TrackDataItem *itemForIndex(const QModelIndex &idx) const;
};

#endif							// FILESDATAMODEL_H
