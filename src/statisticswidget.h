// -*-mode:c++ -*-

#ifndef STATISTICSWIDGET_H
#define STATISTICSWIDGET_H
 
#include <dialogbase.h>
#include "mainwindowinterface.h"


class QGridLayout;
class TrackDataItem;


class StatisticsWidget : public DialogBase, public MainWindowInterface
{
    Q_OBJECT

public:
    explicit StatisticsWidget(QWidget *pnt = nullptr);
    virtual ~StatisticsWidget() = default;

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
