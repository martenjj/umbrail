
#ifndef TRACKPROPERTIESDETAILPAGES_H
#define TRACKPROPERTIESDETAILPAGES_H

#include "trackpropertiespage.h"

class QLabel;

class TrackDataItem;






class TrackItemDetailPage : public TrackPropertiesPage
{
    Q_OBJECT

public:
    virtual ~TrackItemDetailPage()				{}

    QString newItemName() const;

protected:
    TrackItemDetailPage(const QList<TrackDataItem *> *items, QWidget *pnt);

    void addTimeDistanceSpeedFields(const QList<TrackDataItem *> *items, bool bothTimes = true, bool tracksOnly = true);
    void addBoundingAreaField(const QList<TrackDataItem *> *items);
    void addChildCountField(const QList<TrackDataItem *> *items, const QString &labelText);
    void addMetadataField(const TrackDataItem *tdi, const QString &key, const QString &label);
};




class TrackFileDetailPage : public TrackItemDetailPage
{
    Q_OBJECT

public:
    TrackFileDetailPage(const QList<TrackDataItem *> *items, QWidget *pnt);
    virtual ~TrackFileDetailPage()				{}

protected:
};



class TrackTrackDetailPage : public TrackItemDetailPage
{
    Q_OBJECT

public:
    TrackTrackDetailPage(const QList<TrackDataItem *> *items, QWidget *pnt);
    virtual ~TrackTrackDetailPage()				{}
};



class TrackSegmentDetailPage : public TrackItemDetailPage
{
    Q_OBJECT

public:
    TrackSegmentDetailPage(const QList<TrackDataItem *> *items, QWidget *pnt);
    virtual ~TrackSegmentDetailPage()				{}
};



class TrackTrackpointDetailPage : public TrackItemDetailPage
{
    Q_OBJECT

public:
    TrackTrackpointDetailPage(const QList<TrackDataItem *> *items, QWidget *pnt);
    virtual ~TrackTrackpointDetailPage()			{}
};



class TrackFolderDetailPage : public TrackItemDetailPage
{
    Q_OBJECT

public:
    TrackFolderDetailPage(const QList<TrackDataItem *> *items, QWidget *pnt);
    virtual ~TrackFolderDetailPage()				{}
};



class TrackWaypointDetailPage : public TrackItemDetailPage
{
    Q_OBJECT

public:
    TrackWaypointDetailPage(const QList<TrackDataItem *> *items, QWidget *pnt);
    virtual ~TrackWaypointDetailPage()				{}
};


#endif							// TRACKPROPERTIESDETAILPAGES_H
