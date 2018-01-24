
#ifndef TRACKPROPERTIESPLOTPAGES_H
#define TRACKPROPERTIESPLOTPAGES_H

#include "trackpropertiespage.h"


class QCheckBox;
class QSpinBox;

class TrackDataItem;


class TrackItemPlotPage : public TrackPropertiesPage
{
    Q_OBJECT

public:
    virtual ~TrackItemPlotPage()				{}

    QString newBearingLine() const;

protected:
    TrackItemPlotPage(const QList<TrackDataItem *> *items, QWidget *pnt);

protected:
    QCheckBox *mBearingLineCheck;
    QSpinBox *mBearingEntry;

protected slots:
    void slotUpdateButtons();

};


class TrackWaypointPlotPage : public TrackItemPlotPage
{
    Q_OBJECT

public:
    TrackWaypointPlotPage(const QList<TrackDataItem *> *items, QWidget *pnt);
    virtual ~TrackWaypointPlotPage()				{}


};


#endif							// TRACKPROPERTIESPLOTPAGES_H
