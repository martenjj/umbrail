
#ifndef TRACKPROPERTIESDETAILPAGES_H
#define TRACKPROPERTIESDETAILPAGES_H

#include "trackpropertiespage.h"

class QLabel;

class TrackDataItem;


class TrackItemDetailPage : public TrackPropertiesPage
{
    Q_OBJECT

public:
    virtual ~TrackItemDetailPage() = default;

    QString newItemName() const;

    enum DisplayItem
    {
        DisplayPosition = 0x001,			// position or bounding area
        DisplayTime = 0x002,				// time or time span
        DisplayElevation = 0x004,			// elevation or difference
        DisplayTravelDistance = 0x008,			// travel distance
        DisplayAverageSpeed = 0x010,			// average speed
        DisplayRelativeBearing = 0x020,			// relative bearing
        DisplayStraightLine = 0x040,			// straight line distance
        DisplayTravelTime = 0x080,			// total travel time
        DisplayRouteLength = 0x100,			// total route length
    };

    Q_DECLARE_FLAGS(DisplayItems, DisplayItem)

protected:
    TrackItemDetailPage(const QList<TrackDataItem *> *items, QWidget *pnt);

    void addDisplayFields(const QList<TrackDataItem *> *items, DisplayItems disp);
    void addChildCountField(const QList<TrackDataItem *> *items, const QString &labelText);
    void addMetadataField(const TrackDataItem *tdi, const QString &key, const QString &label);
};

Q_DECLARE_OPERATORS_FOR_FLAGS(TrackItemDetailPage::DisplayItems)



class TrackFileDetailPage : public TrackItemDetailPage
{
    Q_OBJECT

public:
    TrackFileDetailPage(const QList<TrackDataItem *> *items, QWidget *pnt);
    virtual ~TrackFileDetailPage() = default;
};



class TrackTrackDetailPage : public TrackItemDetailPage
{
    Q_OBJECT

public:
    TrackTrackDetailPage(const QList<TrackDataItem *> *items, QWidget *pnt);
    virtual ~TrackTrackDetailPage() = default;
};



class TrackRouteDetailPage : public TrackItemDetailPage
{
    Q_OBJECT

public:
    TrackRouteDetailPage(const QList<TrackDataItem *> *items, QWidget *pnt);
    virtual ~TrackRouteDetailPage() = default;
};



class TrackSegmentDetailPage : public TrackItemDetailPage
{
    Q_OBJECT

public:
    TrackSegmentDetailPage(const QList<TrackDataItem *> *items, QWidget *pnt);
    virtual ~TrackSegmentDetailPage() = default;
};



class TrackTrackpointDetailPage : public TrackItemDetailPage
{
    Q_OBJECT

public:
    TrackTrackpointDetailPage(const QList<TrackDataItem *> *items, QWidget *pnt);
    virtual ~TrackTrackpointDetailPage() = default;
};



class TrackFolderDetailPage : public TrackItemDetailPage
{
    Q_OBJECT

public:
    TrackFolderDetailPage(const QList<TrackDataItem *> *items, QWidget *pnt);
    virtual ~TrackFolderDetailPage() = default;
};



class TrackWaypointDetailPage : public TrackItemDetailPage
{
    Q_OBJECT

public:
    TrackWaypointDetailPage(const QList<TrackDataItem *> *items, QWidget *pnt);
    virtual ~TrackWaypointDetailPage() = default;
};


class TrackRoutepointDetailPage : public TrackItemDetailPage
{
    Q_OBJECT

public:
    TrackRoutepointDetailPage(const QList<TrackDataItem *> *items, QWidget *pnt);
    virtual ~TrackRoutepointDetailPage() = default;
};


#endif							// TRACKPROPERTIESDETAILPAGES_H
