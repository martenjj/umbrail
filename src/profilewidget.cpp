
#include "profilewidget.h"

#include <time.h>

#include <qgridlayout.h>
#include <qvector.h>
#include <qcheckbox.h>
#include <qradiobutton.h>
#include <qbuttongroup.h>
#include <qlabel.h>
#include <qtimer.h>
#include <qdebug.h>

#include <klocalizedstring.h>
#include <kconfiggroup.h>

#include <dialogstatewatcher.h>

#include "qcustomplot.h"
#include "mainwindow.h"
#include "filescontroller.h"
#include "filesview.h"
#include "filesmodel.h"
#include "trackdata.h"
#include "variableunitcombo.h"


ProfileWidget::ProfileWidget(QWidget *pnt)
    : DialogBase(pnt),
      DialogStateSaver(this),
      MainWindowInterface(pnt)
{
    qDebug();

    setObjectName("ProfileWidget");
    setButtons(QDialogButtonBox::Close);

    mTimeZone = NULL;

    mUpdateTimer = new QTimer(this);
    mUpdateTimer->setSingleShot(true);
    mUpdateTimer->setInterval(50);
    connect(mUpdateTimer, SIGNAL(timeout()), SLOT(slotUpdatePlot()));

    QWidget *w = new QWidget(this);
    QGridLayout *gl = new QGridLayout(w);

    int col = 0;

    mPlot = new QCustomPlot(this);
    mPlot->addGraph(mPlot->xAxis, mPlot->yAxis);
    mPlot->addGraph(mPlot->xAxis, mPlot->yAxis2);
    gl->addWidget(mPlot, 0, col, 1, -1);

    gl->setRowStretch(0, 1);
    gl->setRowMinimumHeight(1, DialogBase::verticalSpacing());
							// First column: elevation/speed label
    QLabel *l = new QLabel(i18n("Show:"), this);
    gl->addWidget(l, 2, 0, Qt::AlignRight);

    ++col;						// New column: elevation/speed checks

    mElevationCheck = new QCheckBox(i18n("Elevation"), this);
    connect(mElevationCheck, SIGNAL(toggled(bool)), mUpdateTimer, SLOT(start()));
    gl->addWidget(mElevationCheck, 2, col);

    mSpeedCheck = new QCheckBox(i18n("Speed"), this);
    connect(mSpeedCheck, SIGNAL(toggled(bool)), mUpdateTimer, SLOT(start()));
    gl->addWidget(mSpeedCheck, 3, col);

    ++col;						// New column: elevation/speed units

    mElevationUnit = new VariableUnitCombo(VariableUnitCombo::Elevation);
    connect(mElevationUnit, SIGNAL(currentIndexChanged(int)), mUpdateTimer, SLOT(start()));
    gl->addWidget(mElevationUnit, 2, col);

    mSpeedUnit = new VariableUnitCombo(VariableUnitCombo::Speed);
    connect(mSpeedUnit, SIGNAL(currentIndexChanged(int)), mUpdateTimer, SLOT(start()));
    gl->addWidget(mSpeedUnit, 3, col);

    ++col;						// New column: spacer
    gl->setColumnMinimumWidth(col, DialogBase::horizontalSpacing());

    ++col;						// New column: speed source label
    l = new QLabel(i18n("Speed source:"), this);
    gl->addWidget(l, 2, col, Qt::AlignRight);

    ++col;						// New column: speed source radio
    QButtonGroup *bg = new QButtonGroup(this);

    mSpeedGpsRadio = new QRadioButton(i18n("GPS"), this);
    connect(mSpeedGpsRadio, SIGNAL(toggled(bool)), mUpdateTimer, SLOT(start()));
    bg->addButton(mSpeedGpsRadio);
    gl->addWidget(mSpeedGpsRadio, 2, col);

    mSpeedTrackRadio = new QRadioButton(i18n("Track"), this);
    connect(mSpeedTrackRadio, SIGNAL(toggled(bool)), mUpdateTimer, SLOT(start()));
    bg->addButton(mSpeedTrackRadio);
    gl->addWidget(mSpeedTrackRadio, 3, col);

    ++col;						// New column: spacer
    gl->setColumnMinimumWidth(col, DialogBase::horizontalSpacing());

    ++col;						// New column: reference label
    l = new QLabel(i18n("Reference:"), this);
    gl->addWidget(l, 2, col, Qt::AlignRight);

    ++col;						// New column: reference radio
    bg = new QButtonGroup(this);

    mReferenceTimeRadio = new QRadioButton(i18n("Time"), this);
    connect(mReferenceTimeRadio, SIGNAL(toggled(bool)), mUpdateTimer, SLOT(start()));
    bg->addButton(mReferenceTimeRadio);
    gl->addWidget(mReferenceTimeRadio, 2, col);

    mReferenceDistRadio = new QRadioButton(i18n("Distance"), this);
    connect(mReferenceDistRadio, SIGNAL(toggled(bool)), mUpdateTimer, SLOT(start()));
    bg->addButton(mReferenceDistRadio);
    gl->addWidget(mReferenceDistRadio, 3, col);

    ++col;						// New column: distance/time units

    mTimeUnit = new VariableUnitCombo(VariableUnitCombo::Time);
    connect(mTimeUnit, SIGNAL(currentIndexChanged(int)), mUpdateTimer, SLOT(start()));
    gl->addWidget(mTimeUnit, 2, col);

    mDistanceUnit = new VariableUnitCombo(VariableUnitCombo::Distance);
    connect(mDistanceUnit, SIGNAL(currentIndexChanged(int)), mUpdateTimer, SLOT(start()));
    gl->addWidget(mDistanceUnit, 3, col);

    ++col;						// New column: spacer
    gl->setColumnMinimumWidth(col, DialogBase::horizontalSpacing());

    ++col;						// New column: scaling label

    l = new QLabel(i18n("Scaling:"), this);
    gl->addWidget(l, 2, col, Qt::AlignRight);

    ++col;						// New column: scaling radio
    bg = new QButtonGroup(this);

    mScaleAutoRadio = new QRadioButton(i18n("Auto Range"), this);
    connect(mScaleAutoRadio, SIGNAL(toggled(bool)), mUpdateTimer, SLOT(start()));
    bg->addButton(mScaleAutoRadio);
    gl->addWidget(mScaleAutoRadio, 2, col);

    mScaleZeroRadio = new QRadioButton(i18n("Zero Origin"), this);
    connect(mScaleZeroRadio, SIGNAL(toggled(bool)), mUpdateTimer, SLOT(start()));
    bg->addButton(mScaleZeroRadio);
    gl->addWidget(mScaleZeroRadio, 3, col);

    ++col;						// New column: stretch
    gl->setColumnStretch(col, 1);

    setMainWidget(w);
    setStateSaver(this);
    stateWatcher()->setSaveOnButton(buttonBox()->button(QDialogButtonBox::Close));

    if (!mElevationCheck->isChecked() && !mSpeedCheck->isChecked())
    {							// if nothing on, set both on
        mElevationCheck->setChecked(true);
        mSpeedCheck->setChecked(true);
    }

    mUpdateTimer->start();				// do the first plot update
}


ProfileWidget::~ProfileWidget()
{
    delete mTimeZone;
}


void ProfileWidget::restoreConfig(QDialog *dialog, const KConfigGroup &grp)
{
    mElevationCheck->setChecked(grp.readEntry("ShowElevation", true));
    mSpeedCheck->setChecked(grp.readEntry("ShowSpeed", true));
    mElevationUnit->setCurrentIndex(grp.readEntry("UnitElevation", 0));
    mSpeedUnit->setCurrentIndex(grp.readEntry("UnitSpeed", 0));
    mSpeedGpsRadio->setChecked(grp.readEntry("SpeedGps", false));
    mSpeedTrackRadio->setChecked(grp.readEntry("SpeedTrack", true));
    mReferenceTimeRadio->setChecked(grp.readEntry("ReferenceTime", true));
    mReferenceDistRadio->setChecked(grp.readEntry("ReferenceDist", false));
    mTimeUnit->setCurrentIndex(grp.readEntry("UnitTime", 0));
    mDistanceUnit->setCurrentIndex(grp.readEntry("UnitDistance", 0));
    mScaleAutoRadio->setChecked(grp.readEntry("ScaleAuto", true));
    mScaleZeroRadio->setChecked(grp.readEntry("ScaleZero", false));

    DialogStateSaver::restoreConfig(dialog, grp);
}


void ProfileWidget::saveConfig(QDialog *dialog, KConfigGroup &grp) const
{
    grp.writeEntry("ShowElevation", mElevationCheck->isChecked());
    grp.writeEntry("ShowSpeed", mSpeedCheck->isChecked());
    grp.writeEntry("UnitElevation", mElevationUnit->currentIndex());
    grp.writeEntry("UnitSpeed", mSpeedUnit->currentIndex());
    grp.writeEntry("SpeedGps", mSpeedGpsRadio->isChecked());
    grp.writeEntry("SpeedTrack", mSpeedTrackRadio->isChecked());
    grp.writeEntry("ReferenceTime", mReferenceTimeRadio->isChecked());
    grp.writeEntry("ReferenceDist", mReferenceDistRadio->isChecked());
    grp.writeEntry("UnitTime", mTimeUnit->currentIndex());
    grp.writeEntry("UnitDistance", mDistanceUnit->currentIndex());
    grp.writeEntry("ScaleAuto", mScaleAutoRadio->isChecked());
    grp.writeEntry("ScaleZero", mScaleZeroRadio->isChecked());

    DialogStateSaver::saveConfig(dialog, grp);
}


void ProfileWidget::getPlotData(const TrackDataItem *item)
{
    const TrackDataTrackpoint *tdp = dynamic_cast<const TrackDataTrackpoint *>(item);
    if (tdp!=NULL)					// is this a point?
    {
        double distStep = 0;
        double timeStep = 0;

        QDateTime dt = tdp->time();
        // do time zone conversion
        if (mTimeZone!=NULL) dt = dt.toUTC().toTimeZone(*mTimeZone);

        if (mPrevPoint!=NULL)
        {
            distStep = mPrevPoint->distanceTo(tdp, true);
							// distance travelled this step
            timeStep = mPrevPoint->timeTo(tdp);		// time interval this step
        }

        mCumulativeTravel += distStep*mDistanceUnit->factor();
        mPrevPoint = tdp;

        // Reference: either cumulative travel distance, or GPS time (if valid)
        if (mUseTravelDistance)				// plot against distance
        {
            mRefData.append(mCumulativeTravel);
        }
        else						// plot against time
        {
            if (!dt.isValid()) return;			// no time available this point

            time_t tm = dt.toTime_t();

            if (mTimeZone!=NULL)			// file time zone available
            {
                // Axis times to be displayed by QCustomPlot are passed to it
                // as double values representing a time_t.  They may either be
                // local time or UTC, and the corresponding setting passed to
                // QCPAxis::setDateTimeSpec() gives the correct display.
                //
                // However, it is not possible to display the value converted to
                // a time zone time, which is what is required.  So that is done
                // manually here instead, using the offset from UTC to the file
                // time zone, and setting the axis to display UTC values.  The
                // offset can vary depending on the time of year (i.e. whether
                // DST is in operation), so it is calculated here at the time of
                // the data point.  This hopefully means that the displayed time
                // values will be correct even if they span DST transitions.

                tm += mTimeZone->offsetFromUtc(dt);
            }

            if (mBaseTime==0)				// this is the first point
            {
                mBaseTime = tm;				// use this as base time
                qDebug() << "time zone offset" << mTimeZone->offsetFromUtc(dt);
            }

            if (mTimeUnit->factor()==VariableUnitCombo::TimeRelative)
            {						// to make times start at zero
                tm = static_cast<time_t>(difftime(tm, mBaseTime));
            }
            mRefData.append(tm);
        }

        // Elevation, data always in metres
        const double ele = tdp->elevation()*mElevationUnit->factor();
        mElevData.append(ele);

        // Speed, either from GPS or calculated from track
        double spd;
        if (mUseGpsSpeed)				// GPS speed
        {
            const QString speedMeta = tdp->metadata("speed");
            if (speedMeta.isEmpty()) spd = 0;
            else
            {
                spd = VariableUnitCombo::distanceFromMetres(speedMeta.toDouble())*mSpeedUnit->factor()*3600;
            }
        }
        else						// speed from track
        {
            if (timeStep==0) spd = 0;
            else spd = (distStep*mSpeedUnit->factor()*3600)/timeStep;
        }
        mSpeedData.append(spd);
    }
    else						// not a point, recurse for children
    {
        const int num = item->childCount(); 
        for (int i = 0; i<num; ++i) getPlotData(item->childAt(i));
    }
}


void ProfileWidget::slotUpdatePlot()
{
    const bool speedEnabled = mSpeedCheck->isChecked();
    mSpeedGpsRadio->setEnabled(speedEnabled);
    mSpeedTrackRadio->setEnabled(speedEnabled);
    mSpeedUnit->setEnabled(speedEnabled);

    const bool elevationEnabled = mElevationCheck->isChecked();
    mElevationUnit->setEnabled(elevationEnabled);

    mUseGpsSpeed = mSpeedGpsRadio->isChecked();
    mUseTravelDistance = mReferenceDistRadio->isChecked();

    mTimeUnit->setEnabled(!mUseTravelDistance);
    mDistanceUnit->setEnabled(mUseTravelDistance);

    mCumulativeTravel = 0;
    mPrevPoint = NULL;

    const QList<TrackDataItem *> items = filesController()->view()->selectedItems();

    mRefData.clear();
    mElevData.clear();
    mSpeedData.clear();
    mBaseTime = 0;

    delete mTimeZone;
    mTimeZone = NULL;					// no time zone available yet

    // Resolve the file time zone
    QString zoneName = filesController()->model()->rootFileItem()->metadata("timezone");
    if (!zoneName.isEmpty())
    {
        QTimeZone *tz = new QTimeZone(zoneName.toLatin1());
        if (tz->isValid()) mTimeZone = tz;		// use as time zone
        else qWarning() << "unknown time zone" << zoneName;
    }

    for (int i = 0; i<items.count(); ++i) getPlotData(items[i]);
    qDebug() << "got" << mRefData.count() << "data points";

    QCPGraph *graph = mPlot->graph(0);			// elevation graph
    graph->setData(mRefData, mElevData);
    graph->setVisible(elevationEnabled);
    graph->setPen(QPen(Qt::red));
    graph->rescaleValueAxis();

    graph = mPlot->graph(1);				// speed graph
    graph->setData(mRefData, mSpeedData);
    graph->setVisible(speedEnabled);
    graph->setPen(QPen(Qt::blue));
    graph->rescaleValueAxis();

    if (mReferenceTimeRadio->isChecked())		// reference axis
    {
        mPlot->xAxis->setLabel(i18n("Time (%1)", mTimeUnit->currentText()));
        mPlot->xAxis->setTickLabelType(QCPAxis::ltDateTime);
        mPlot->xAxis->setDateTimeFormat("hh:mm");
        // See the comment in getPlotData() above
        mPlot->xAxis->setDateTimeSpec(Qt::UTC);
    }
    else
    {
        mPlot->xAxis->setLabel(i18n("Travel (%1)", mDistanceUnit->currentText()));
        mPlot->xAxis->setTickLabelType(QCPAxis::ltNumber);
        mPlot->xAxis->setNumberFormat("g");
    }
    mPlot->xAxis->rescale();
							// elevation axis
    mPlot->yAxis->setLabel(i18n("Elevation (%1)", mElevationUnit->currentText()));
    mPlot->yAxis->setLabelColor(Qt::red);
    mPlot->yAxis->setVisible(true);

    if (mUseGpsSpeed)					// speed axis
    {
        mPlot->yAxis2->setLabel(i18n("Speed (GPS %1)", mSpeedUnit->currentText()));
    }
    else
    {
        mPlot->yAxis2->setLabel(i18n("Speed (%1)", mSpeedUnit->currentText()));
    }
    mPlot->yAxis2->setLabelColor(Qt::blue);
    mPlot->yAxis2->setVisible(true);

    if (mScaleZeroRadio->isChecked())			// zero origin
    {
        mPlot->yAxis->setRangeLower(0);
        mPlot->yAxis2->setRangeLower(0);
    }

    mPlot->replot();
}
