// -*-mode:c++ -*-

#ifndef PROFILEWIDGET_H
#define PROFILEWIDGET_H
 
#include <dialogbase.h>
#include <dialogstatesaver.h>

#include <qvector.h>

#include "mainwindowinterface.h"


class QCheckBox;
class QRadioButton;
class QTimeZone;
class TrackDataItem;
class TrackDataTrackpoint;
class VariableUnitCombo;
class QCustomPlot;
class KConfigGroup;


class ProfileWidgetStateSaver : public DialogStateSaver
{
    Q_OBJECT

public:
    ProfileWidgetStateSaver(QDialog *pnt);
    virtual ~ProfileWidgetStateSaver() = default;

protected:
    void saveConfig(QDialog *dialog, KConfigGroup &grp) const;
    void restoreConfig(QDialog *dialog, const KConfigGroup &grp);
};


class ProfileWidget : public DialogBase, public MainWindowInterface
{
    Q_OBJECT

public:
    explicit ProfileWidget(QWidget *pnt = nullptr);
    virtual ~ProfileWidget();

    void saveConfig(KConfigGroup &grp) const;
    void restoreConfig(const KConfigGroup &grp);

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
    QTimeZone *mTimeZone;
};

#endif							// PROFILEWIDGET_H
