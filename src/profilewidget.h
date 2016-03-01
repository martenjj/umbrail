// -*-mode:c++ -*-

#ifndef PROFILEWIDGET_H
#define PROFILEWIDGET_H
 
#include <kdialog.h>
#include "mainwindowinterface.h"

#include <qvector.h>

class QCheckBox;
class QRadioButton;
class TrackDataItem;
class TrackDataTrackpoint;
class VariableUnitCombo;
class QCustomPlot;


class ProfileWidget : public KDialog, public MainWindowInterface
{
    Q_OBJECT

public:
    explicit ProfileWidget(QWidget *pnt = NULL);
    virtual ~ProfileWidget();

private slots:
    void slotUpdatePlot();

private:
    void getPlotData(const TrackDataItem *item);

private:
    QCheckBox *mElevationCheck;
    QCheckBox *mSpeedCheck;
    QRadioButton *mSpeedGpsRadio;
    QRadioButton *mSpeedTrackRadio;
    QRadioButton *mReferenceTimeRadio;
    QRadioButton *mReferenceDistRadio;
    QRadioButton *mScaleAutoRadio;
    QRadioButton *mScaleZeroRadio;
    VariableUnitCombo *mElevationUnit;
    VariableUnitCombo *mSpeedUnit;
    VariableUnitCombo *mDistanceUnit;
    VariableUnitCombo *mTimeUnit;

    QVector<double> mRefData;
    QVector<double> mElevData;
    QVector<double> mSpeedData;

    QCustomPlot *mPlot;

    bool mUseGpsSpeed;
    bool mUseTravelDistance;
    double mCumulativeTravel;
    const TrackDataTrackpoint *mPrevPoint;
    time_t mBaseTime;
};

#endif							// PROFILEWIDGET_H
