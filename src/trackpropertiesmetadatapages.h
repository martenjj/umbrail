
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
    virtual ~TrackItemMetadataPage() = default;
    void refreshData() override;

protected:
    TrackItemMetadataPage(const QList<TrackDataItem *> *items, QWidget *pnt);

private:
    QTableView *mView;
};


class TrackFileMetadataPage : public TrackItemMetadataPage
{
    Q_OBJECT

public:
    TrackFileMetadataPage(const QList<TrackDataItem *> *items, QWidget *pnt);
    virtual ~TrackFileMetadataPage() = default;
};


class TrackTrackMetadataPage : public TrackItemMetadataPage
{
    Q_OBJECT

public:
    TrackTrackMetadataPage(const QList<TrackDataItem *> *items, QWidget *pnt);
    virtual ~TrackTrackMetadataPage() = default;
};


class TrackSegmentMetadataPage : public TrackItemMetadataPage
{
    Q_OBJECT

public:
    TrackSegmentMetadataPage(const QList<TrackDataItem *> *items, QWidget *pnt);
    virtual ~TrackSegmentMetadataPage() = default;
};


class TrackTrackpointMetadataPage : public TrackItemMetadataPage
{
    Q_OBJECT

public:
    TrackTrackpointMetadataPage(const QList<TrackDataItem *> *items, QWidget *pnt);
    virtual ~TrackTrackpointMetadataPage() = default;
};


class TrackFolderMetadataPage : public TrackItemMetadataPage
{
    Q_OBJECT

public:
    TrackFolderMetadataPage(const QList<TrackDataItem *> *items, QWidget *pnt);
    virtual ~TrackFolderMetadataPage() = default;
};


class TrackWaypointMetadataPage : public TrackItemMetadataPage
{
    Q_OBJECT

public:
    TrackWaypointMetadataPage(const QList<TrackDataItem *> *items, QWidget *pnt);
    virtual ~TrackWaypointMetadataPage() = default;
};


class TrackRouteMetadataPage : public TrackItemMetadataPage
{
    Q_OBJECT

public:
    TrackRouteMetadataPage(const QList<TrackDataItem *> *items, QWidget *pnt);
    virtual ~TrackRouteMetadataPage() = default;
};


class TrackRoutepointMetadataPage : public TrackItemMetadataPage
{
    Q_OBJECT

public:
    TrackRoutepointMetadataPage(const QList<TrackDataItem *> *items, QWidget *pnt);
    virtual ~TrackRoutepointMetadataPage() = default;
};

#endif							// TRACKPROPERTIESMETADATAPAGES_H
