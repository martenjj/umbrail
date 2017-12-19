// -*-mode:c++ -*-

#ifndef PROFILEWIDGET_H
#define PROFILEWIDGET_H
 
#include <dialogbase.h>
#include <dialogstatesaver.h>

#include <qvector.h>

#include "mainwindowinterface.h"


class QTimer;
class QCheckBox;
class QComboBox;
class QRadioButton;
class QTimeZone;
class TrackDataItem;
class TrackDataTrackpoint;
class VariableUnitCombo;
class QCustomPlot;
class QCPAxisTicker;
class KConfigGroup;
class ElevationTile;


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
    void slotElevationTileReady(const ElevationTile *tile);

private:
    void getPlotData(const TrackDataItem *item);

private:
    QCheckBox *mElevationCheck;
    QCheckBox *mSpeedCheck;
    QComboBox *mElevationSourceCombo;
    QComboBox *mSpeedSourceCombo;
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

    ProfileWidget::ElevationSource mElevationSource;
    ProfileWidget::SpeedSource mSpeedSource;

    bool mUseTravelDistance;
    double mCumulativeTravel;
    const TrackDataTrackpoint *mPrevPoint;

    time_t mBaseTime;
    QTimeZone *mTimeZone;

    QSharedPointer<QCPAxisTicker> mNumberTicker;
    QSharedPointer<QCPAxisTicker> mTimeTicker;
};

#endif							// PROFILEWIDGET_H
