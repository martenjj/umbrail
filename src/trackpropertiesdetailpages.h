
#ifndef TRACKPROPERTIESDETAILPAGES_H
#define TRACKPROPERTIESDETAILPAGES_H

#include <kurl.h>

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
    TrackItemDetailPage(const QList<TrackDataItem *> items, QWidget *pnt);

    void addTimeDistanceSpeedFields(const QList<TrackDataItem *> &items, bool bothTimes = true);
    void addBoundingAreaField(const QList<TrackDataItem *> &items);
    void addChildCountField(const QList<TrackDataItem *> &items, const QString &labelText);
    void addMetadataField(const TrackDataItem *tdi, const QString &key, const QString &label);

protected:
    bool mIsEmpty;
};




class TrackFileDetailPage : public TrackItemDetailPage
{
    Q_OBJECT

public:
    TrackFileDetailPage(const QList<TrackDataItem *> items, QWidget *pnt);
    virtual ~TrackFileDetailPage()				{}

protected:
};



class TrackTrackDetailPage : public TrackItemDetailPage
{
    Q_OBJECT

public:
    TrackTrackDetailPage(const QList<TrackDataItem *> items, QWidget *pnt);
    virtual ~TrackTrackDetailPage()				{}
};



class TrackSegmentDetailPage : public TrackItemDetailPage
{
    Q_OBJECT

public:
    TrackSegmentDetailPage(const QList<TrackDataItem *> items, QWidget *pnt);
    virtual ~TrackSegmentDetailPage()				{}
};



class TrackPointDetailPage : public TrackItemDetailPage
{
    Q_OBJECT

public:
    TrackPointDetailPage(const QList<TrackDataItem *> items, QWidget *pnt);
    virtual ~TrackPointDetailPage()				{}
};




#endif							// TRACKPROPERTIESDETAILPAGES_H
