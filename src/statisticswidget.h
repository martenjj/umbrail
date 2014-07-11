// -*-mode:c++ -*-

#ifndef STATISTICSWIDGET_H
#define STATISTICSWIDGET_H
 
#include <kdialog.h>


class QGridLayout;
class MainWindow;
class TrackDataItem;


class StatisticsWidget : public KDialog
{
    Q_OBJECT

public:
    explicit StatisticsWidget(QWidget *pnt = NULL);
    virtual ~StatisticsWidget();

private:
    MainWindow *mainWindow() const		{ return (mMainWindow); }

    void getPointData(const TrackDataItem *item);
    void addRow(const QString &text, int num, bool withPercent = true);

private:
    MainWindow *mMainWindow;
    QWidget *mWidget;
    QGridLayout *mLayout;

    int mTotalPoints;
    int mWithTime;
    int mWithElevation;
    int mWithGpsSpeed;
    int mWithGpsHdop;
};

#endif							// STATISTICSWIDGET_H
