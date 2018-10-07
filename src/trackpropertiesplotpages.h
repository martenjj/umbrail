
#ifndef TRACKPROPERTIESPLOTPAGES_H
#define TRACKPROPERTIESPLOTPAGES_H

#include "trackpropertiespage.h"


class PlotEditWidget;
class TrackDataItem;


class TrackItemPlotPage : public TrackPropertiesPage
{
    Q_OBJECT

public:
    virtual ~TrackItemPlotPage() = default;

protected:
    TrackItemPlotPage(const QList<TrackDataItem *> *items, QWidget *pnt);
};


class TrackWaypointPlotPage : public TrackItemPlotPage
{
    Q_OBJECT

public:
    TrackWaypointPlotPage(const QList<TrackDataItem *> *items, QWidget *pnt);
    virtual ~TrackWaypointPlotPage() = default;
    void refreshData() override;

protected slots:
    void slotPlotDataChanged();

private:
    PlotEditWidget *mBearingEdit;
    PlotEditWidget *mRangeEdit;
};

#endif							// TRACKPROPERTIESPLOTPAGES_H
