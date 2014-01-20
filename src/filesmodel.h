// -*-mode:c++ -*-

#ifndef FILESMODEL_H
#define FILESMODEL_H
 
#include <qabstractitemmodel.h>

//#include "pointdata.h"


class FilesController;
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

    int fileCount() const;
    const TrackDataFile *fileAt(int i) const;
    void addFile(TrackDataFile *tdf);
    TrackDataFile *removeFile();

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
//    // row 'row' in place
//    void changePoint(const PointData *pnt, int row);

private:
    TrackDataItem *dataPointer(const QModelIndex &idx) const;
    QModelIndex indexForData(const TrackDataItem *tdi) const;
//    FilesController *controller() const		{ return (mController); }

private:
//    FilesList mFiles;
//    FilesController *mController;
    TrackDataItem *mRootItem;

};
 
#endif							// FILESMODEL_H
