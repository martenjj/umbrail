// -*-mode:c++ -*-

#ifndef FILESMODEL_H
#define FILESMODEL_H
 
#include <qabstractitemmodel.h>


class FilesController;
class TrackDataDisplayable;
class TrackDataItem;
class TrackDataFile;

class FilesModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    FilesModel(QObject *parent = NULL);
    virtual ~FilesModel();

    virtual QModelIndex index(int row, int col, const QModelIndex &parent = QModelIndex()) const;
    virtual QModelIndex parent(const QModelIndex &idx) const;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex &idx, int role) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;

    void clear();
    bool isEmpty() const;

    void addFile(TrackDataFile *tdf);
    TrackDataItem *removeLast();


//    void addPoint(const PointData *point);
//    void addFiles(const FilesList *files, bool clear = false);
//
//    void removePoint(int row);
//    // inclusive
//    void removeFiles(int fromRow, int toRow);
//
//    // the new point becomes row 'row'
//    void insertPoint(const PointData *pnt, int row);
//
    // in place
    void changedItem(const TrackDataItem *item);

    TrackDataFile *rootFileItem() const;

private:
    TrackDataItem *dataPointer(const QModelIndex &idx) const;
    QModelIndex indexForData(const TrackDataItem *tdi) const;

private:
    TrackDataItem *mRootItem;
//    int mTrackId;
};
 
#endif							// FILESMODEL_H
