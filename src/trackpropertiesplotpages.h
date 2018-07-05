
#ifndef TRACKPROPERTIESPLOTPAGES_H
#define TRACKPROPERTIESPLOTPAGES_H

#include "trackpropertiespage.h"


//class QCheckBox;
//class QSpinBox;
class PlotEditWidget;
class TrackDataItem;


class TrackItemPlotPage : public TrackPropertiesPage
{
    Q_OBJECT

public:
    virtual ~TrackItemPlotPage() = default;

    QString newBearingData() const;
    QString newRangeData() const;

protected:
    TrackItemPlotPage(const QList<TrackDataItem *> *items, QWidget *pnt);

private:
    PlotEditWidget *mBearingEdit;
    PlotEditWidget *mRangeEdit;
};


class TrackWaypointPlotPage : public TrackItemPlotPage
{
    Q_OBJECT

public:
    TrackWaypointPlotPage(const QList<TrackDataItem *> *items, QWidget *pnt);
    virtual ~TrackWaypointPlotPage() = default;
};


#endif							// TRACKPROPERTIESPLOTPAGES_H
