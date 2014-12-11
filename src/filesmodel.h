// -*-mode:c++ -*-

#ifndef FILESMODEL_H
#define FILESMODEL_H
 
#include <qabstractitemmodel.h>


class FilesController;
class TrackDataItem;
class TrackDataFile;
class TrackDataAbstractPoint;


class FilesModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    FilesModel(QObject *pnt = NULL);
    virtual ~FilesModel();

    virtual QModelIndex index(int row, int col, const QModelIndex &parent = QModelIndex()) const;
    virtual QModelIndex parent(const QModelIndex &idx) const;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex &idx, int role) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;

    TrackDataFile *rootFileItem() const			{ return (mRootFileItem); }
    bool isEmpty() const				{ return (mRootFileItem==NULL); }
    TrackDataFile *takeRootFileItem();
    void setRootFileItem(TrackDataFile *root);

    void clickedPoint(const TrackDataAbstractPoint *tdp, Qt::KeyboardModifiers mods);

    // signal changes from the model
    void changedItem(const TrackDataItem *item);
    void startLayoutChange();
    void endLayoutChange();

    QModelIndex indexForItem(const TrackDataItem *tdi) const;
    TrackDataItem *itemForIndex(const QModelIndex &idx) const;

signals:
    void clickedItem(const QModelIndex &index, unsigned int flags);

private:
    TrackDataFile *mRootFileItem;
};
 
#endif							// FILESMODEL_H
