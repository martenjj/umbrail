// -*-mode:c++ -*-

#ifndef STATISTICSWIDGET_H
#define STATISTICSWIDGET_H
 
#include <kdialog.h>
#include "mainwindowinterface.h"


class QGridLayout;
class TrackDataItem;


class StatisticsWidget : public KDialog, public MainWindowInterface
{
    Q_OBJECT

public:
    explicit StatisticsWidget(QWidget *pnt = NULL);
    virtual ~StatisticsWidget();

private:
    void getPointData(const TrackDataItem *item);
    void addRow(const QString &text, int num, bool withPercent = true);

private:
    QWidget *mWidget;
    QGridLayout *mLayout;

    int mTotalPoints;
    int mWithTime;
    int mWithElevation;
    int mWithGpsSpeed;
    int mWithGpsHdop;
};

#endif							// STATISTICSWIDGET_H
