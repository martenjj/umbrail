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
    FilesModel(QObject *pnt = nullptr);
    virtual ~FilesModel();

    virtual QModelIndex index(int row, int col, const QModelIndex &parent = QModelIndex()) const override;
    virtual QModelIndex parent(const QModelIndex &idx) const override;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual QVariant data(const QModelIndex &idx, int role) const override;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    TrackDataFile *rootFileItem() const			{ return (mRootFileItem); }
    bool isEmpty() const				{ return (mRootFileItem==nullptr); }
    TrackDataFile *takeRootFileItem();
    void setRootFileItem(TrackDataFile *root);

    void clickedPoint(const TrackDataAbstractPoint *tdp, Qt::KeyboardModifiers mods);

    // signal changes from the model
    void changedItem(const TrackDataItem *item);
    void startLayoutChange();
    void endLayoutChange();

    QModelIndex indexForItem(const TrackDataItem *tdi) const;
    static TrackDataItem *itemForIndex(const QModelIndex &idx);

signals:
    void clickedItem(const QModelIndex &index, unsigned int flags);

private:
    TrackDataFile *mRootFileItem;
};
 
#endif							// FILESMODEL_H
