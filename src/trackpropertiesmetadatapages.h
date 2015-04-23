
#ifndef TRACKPROPERTIESMETADATAPAGES_H
#define TRACKPROPERTIESMETADATAPAGES_H

#include "trackpropertiespage.h"


class QTableView;

class TrackDataItem;
class MetadataModel;


class TrackItemMetadataPage : public TrackPropertiesPage
{
    Q_OBJECT

public:
    virtual ~TrackItemMetadataPage()				{}

protected:
    TrackItemMetadataPage(const QList<TrackDataItem *> *items, QWidget *pnt);

private:
    QTableView *mView;
    MetadataModel *mModel;
};




class TrackFileMetadataPage : public TrackItemMetadataPage
{
    Q_OBJECT

public:
    TrackFileMetadataPage(const QList<TrackDataItem *> *items, QWidget *pnt);
    virtual ~TrackFileMetadataPage()				{}

};



class TrackTrackMetadataPage : public TrackItemMetadataPage
{
    Q_OBJECT

public:
    TrackTrackMetadataPage(const QList<TrackDataItem *> *items, QWidget *pnt);
    virtual ~TrackTrackMetadataPage()				{}


};



class TrackSegmentMetadataPage : public TrackItemMetadataPage
{
    Q_OBJECT

public:
    TrackSegmentMetadataPage(const QList<TrackDataItem *> *items, QWidget *pnt);
    virtual ~TrackSegmentMetadataPage()				{}


};



class TrackTrackpointMetadataPage : public TrackItemMetadataPage
{
    Q_OBJECT

public:
    TrackTrackpointMetadataPage(const QList<TrackDataItem *> *items, QWidget *pnt);
    virtual ~TrackTrackpointMetadataPage()				{}


};



class TrackFolderMetadataPage : public TrackItemMetadataPage
{
    Q_OBJECT

public:
    TrackFolderMetadataPage(const QList<TrackDataItem *> *items, QWidget *pnt);
    virtual ~TrackFolderMetadataPage()				{}


};



class TrackWaypointMetadataPage : public TrackItemMetadataPage
{
    Q_OBJECT

public:
    TrackWaypointMetadataPage(const QList<TrackDataItem *> *items, QWidget *pnt);
    virtual ~TrackWaypointMetadataPage()				{}


};


#endif							// TRACKPROPERTIESMETADATAPAGES_H
