
#ifndef TRACKPROPERTIESSTYLEPAGES_H
#define TRACKPROPERTIESSTYLEPAGES_H

#include "trackpropertiespage.h"


class QCheckBox;
class KColorButton;
class TrackDataItem;


class TrackItemStylePage : public TrackPropertiesPage
{
    Q_OBJECT

public:
    virtual ~TrackItemStylePage() = default;
    void refreshData() override;

protected:
    TrackItemStylePage(const QList<TrackDataItem *> *items, QWidget *pnt);

protected:
    KColorButton *mLineColourButton;
    QCheckBox *mLineInheritCheck;
    KColorButton *mPointColourButton;
    QCheckBox *mPointInheritCheck;

protected slots:
    void slotColourChanged(const QColor &col);
    void slotInheritChanged(bool on);
};


class TrackFileStylePage : public TrackItemStylePage
{
    Q_OBJECT

public:
    TrackFileStylePage(const QList<TrackDataItem *> *items, QWidget *pnt);
    virtual ~TrackFileStylePage() = default;
};


class TrackTrackStylePage : public TrackItemStylePage
{
    Q_OBJECT

public:
    TrackTrackStylePage(const QList<TrackDataItem *> *items, QWidget *pnt);
    virtual ~TrackTrackStylePage() = default;
};


class TrackSegmentStylePage : public TrackItemStylePage
{
    Q_OBJECT

public:
    TrackSegmentStylePage(const QList<TrackDataItem *> *items, QWidget *pnt);
    virtual ~TrackSegmentStylePage() = default;
};


class TrackWaypointStylePage : public TrackItemStylePage
{
    Q_OBJECT

public:
    TrackWaypointStylePage(const QList<TrackDataItem *> *items, QWidget *pnt);
    virtual ~TrackWaypointStylePage() = default;
};


class TrackRouteStylePage : public TrackItemStylePage
{
    Q_OBJECT

public:
    TrackRouteStylePage(const QList<TrackDataItem *> *items, QWidget *pnt);
    virtual ~TrackRouteStylePage() = default;
};

#endif							// TRACKPROPERTIESSTYLEPAGES_H
