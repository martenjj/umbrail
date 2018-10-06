
#ifndef TRACKPROPERTIESSTYLEPAGES_H
#define TRACKPROPERTIESSTYLEPAGES_H

#include "trackpropertiespage.h"


class QFormLayout;
class QCheckBox;

class KColorButton;

class TrackDataItem;


class TrackItemStylePage : public TrackPropertiesPage
{
    Q_OBJECT

public:
    virtual ~TrackItemStylePage() = default;

    QString newItemName() const;
    QColor newColour() const;

protected:
    TrackItemStylePage(const QList<TrackDataItem *> *items, QWidget *pnt);

protected:
    KColorButton *mLineColourButton;
    QCheckBox *mLineInheritCheck;
    KColorButton *mPointColourButton;
    QCheckBox *mPointInheritCheck;

protected slots:
    void slotColourChanged(const QColor &col);
};


class TrackFileStylePage : public TrackItemStylePage
{
    Q_OBJECT

public:
    TrackFileStylePage(const QList<TrackDataItem *> *items, QWidget *pnt);
    virtual ~TrackFileStylePage() = default;
    void refreshData() override;
};


class TrackTrackStylePage : public TrackItemStylePage
{
    Q_OBJECT

public:
    TrackTrackStylePage(const QList<TrackDataItem *> *items, QWidget *pnt);
    virtual ~TrackTrackStylePage() = default;
    void refreshData() override;
};


class TrackSegmentStylePage : public TrackItemStylePage
{
    Q_OBJECT

public:
    TrackSegmentStylePage(const QList<TrackDataItem *> *items, QWidget *pnt);
    virtual ~TrackSegmentStylePage() = default;
    void refreshData() override;
};


class TrackWaypointStylePage : public TrackItemStylePage
{
    Q_OBJECT

public:
    TrackWaypointStylePage(const QList<TrackDataItem *> *items, QWidget *pnt);
    virtual ~TrackWaypointStylePage() = default;
    void refreshData() override;
};


class TrackRouteStylePage : public TrackItemStylePage
{
    Q_OBJECT

public:
    TrackRouteStylePage(const QList<TrackDataItem *> *items, QWidget *pnt);
    virtual ~TrackRouteStylePage() = default;
    void refreshData() override;
};

#endif							// TRACKPROPERTIESSTYLEPAGES_H
