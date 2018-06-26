// -*-mode:c++ -*-

#ifndef PROFILEWIDGET_H
#define PROFILEWIDGET_H
 
#include <dialogbase.h>
#include <dialogstatesaver.h>

#include <qvector.h>
#include <qmap.h>

#include "mainwindowinterface.h"


class QTimer;
class QCheckBox;
class QComboBox;
class QRadioButton;
class QTimeZone;

class KConfigGroup;

class TrackDataItem;
class TrackDataAbstractPoint;
class VariableUnitCombo;
class ElevationTile;

class QCustomPlot;
class QCPAxisTicker;
class QCPLayerable;
class WaypointLayerable;


class ProfileWidget : public DialogBase, public DialogStateSaver, public MainWindowInterface
{
    Q_OBJECT

public:
    explicit ProfileWidget(QWidget *pnt = nullptr);
    virtual ~ProfileWidget();

    void saveConfig(QDialog *dialog, KConfigGroup &grp) const override;
    void restoreConfig(QDialog *dialog, const KConfigGroup &grp) override;

private slots:
    void slotUpdatePlot();

private:
    void getPlotData(const TrackDataAbstractPoint *point);
    void associateWaypoints(const TrackDataItem *item);

private:
    QCheckBox *mElevationCheck;
    QCheckBox *mSpeedCheck;
    QComboBox *mElevationSourceCombo;
    QComboBox *mSpeedSourceCombo;
    QComboBox *mScaleRangeCombo;
    QRadioButton *mReferenceTimeRadio;
    QRadioButton *mReferenceDistRadio;
    VariableUnitCombo *mElevationUnit;
    VariableUnitCombo *mSpeedUnit;
    VariableUnitCombo *mDistanceUnit;
    VariableUnitCombo *mTimeUnit;

    QVector<const TrackDataAbstractPoint *> mPoints;
    QVector<double> mRefData;
    QVector<double> mElevData;
    QVector<double> mSpeedData;

    QMap<const TrackDataAbstractPoint *, int> mWaypoints;

    QCustomPlot *mPlot;
    WaypointLayerable *mWaypointLayerable;

    QTimer *mUpdateTimer;

    enum ElevationSource
    {
        ElevationSourceGPS = 0,
        ElevationSourceDEM = 1
    };

    enum SpeedSource
    {
        SpeedSourceGPS = 0,
        SpeedSourceTrack = 1
    };

    enum ScaleRange
    {
        ScaleRangeAuto = 0,
        ScaleRangeZero = 1
    };

    ProfileWidget::ElevationSource mElevationSource;
    ProfileWidget::SpeedSource mSpeedSource;

    bool mUseTravelDistance;
    double mCumulativeTravel;
    const TrackDataAbstractPoint *mPrevPoint;

    time_t mBaseTime;
    QTimeZone *mTimeZone;

    QSharedPointer<QCPAxisTicker> mNumberTicker;
    QSharedPointer<QCPAxisTicker> mTimeTicker;
};

#endif							// PROFILEWIDGET_H
