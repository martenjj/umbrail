
#include "profilewidget.h"

#include <math.h>

#include <qgridlayout.h>
#include <qvector.h>
#include <qcheckbox.h>
#include <qradiobutton.h>
#include <qbuttongroup.h>
#include <qlabel.h>

#include <kdebug.h>
#include <klocale.h>
#include <kconfiggroup.h>
#include <kglobal.h>

#include "qcustomplot.h"
#include "mainwindow.h"
#include "filescontroller.h"
#include "filesview.h"
#include "trackdata.h"


ProfileWidget::ProfileWidget(QWidget *pnt)
    : KDialog(pnt)
{
    kDebug();

    setObjectName("ProfileWidget");
    setButtons(KDialog::Close);
    showButtonSeparator(true);

    mMainWindow = qobject_cast<MainWindow *>(pnt);

    QWidget *w = new QWidget(this);
    QGridLayout *gl = new QGridLayout(w);

    mPlot = new QCustomPlot(this);
    mPlot->addGraph(mPlot->xAxis, mPlot->yAxis);
    mPlot->addGraph(mPlot->xAxis, mPlot->yAxis2);
    gl->addWidget(mPlot, 0, 0, 1, -1);

    gl->setRowStretch(0, 1);
    gl->setRowMinimumHeight(1, KDialog::spacingHint());

    QLabel *l = new QLabel(i18n("Show:"), this);
    gl->addWidget(l, 2, 0, Qt::AlignRight);

    mElevationCheck = new QCheckBox(i18n("Elevation"), this);
    connect(mElevationCheck, SIGNAL(toggled(bool)), SLOT(slotUpdatePlot()));
    gl->addWidget(mElevationCheck, 2, 1);

    mSpeedCheck = new QCheckBox(i18n("Speed"), this);
    connect(mSpeedCheck, SIGNAL(toggled(bool)), SLOT(slotUpdatePlot()));
    gl->addWidget(mSpeedCheck, 3, 1);

    gl->setColumnMinimumWidth(2, KDialog::spacingHint());

    l = new QLabel(i18n("Speed source:"), this);
    gl->addWidget(l, 2, 3, Qt::AlignRight);

    QButtonGroup *bg = new QButtonGroup(this);

    mSpeedGpsRadio = new QRadioButton(i18n("GPS"), this);
    bg->addButton(mSpeedGpsRadio);
    connect(mSpeedGpsRadio, SIGNAL(toggled(bool)), SLOT(slotUpdatePlot()));
    gl->addWidget(mSpeedGpsRadio, 2, 4);

    mSpeedTrackRadio = new QRadioButton(i18n("Track"), this);
    bg->addButton(mSpeedTrackRadio);
    connect(mSpeedTrackRadio, SIGNAL(toggled(bool)), SLOT(slotUpdatePlot()));
    gl->addWidget(mSpeedTrackRadio, 3, 4);

    gl->setColumnMinimumWidth(5, KDialog::spacingHint());

    l = new QLabel(i18n("Reference:"), this);
    gl->addWidget(l, 2, 6, Qt::AlignRight);

    bg = new QButtonGroup(this);

    mReferenceTimeRadio = new QRadioButton(i18n("Time"), this);
    bg->addButton(mReferenceTimeRadio);
    connect(mReferenceTimeRadio, SIGNAL(toggled(bool)), SLOT(slotUpdatePlot()));
    gl->addWidget(mReferenceTimeRadio, 2, 7);

    mReferenceDistRadio = new QRadioButton(i18n("Distance"), this);
    bg->addButton(mReferenceDistRadio);
    connect(mReferenceDistRadio, SIGNAL(toggled(bool)), SLOT(slotUpdatePlot()));
    gl->addWidget(mReferenceDistRadio, 3, 7);

    gl->setColumnMinimumWidth(8, KDialog::spacingHint());

    l = new QLabel(i18n("Scaling:"), this);
    gl->addWidget(l, 2, 9, Qt::AlignRight);

    bg = new QButtonGroup(this);

    mScaleAutoRadio = new QRadioButton(i18n("Auto Range"), this);
    bg->addButton(mScaleAutoRadio);
    connect(mScaleAutoRadio, SIGNAL(toggled(bool)), SLOT(slotUpdatePlot()));
    gl->addWidget(mScaleAutoRadio, 2, 10);

    mScaleZeroRadio = new QRadioButton(i18n("Zero Origin"), this);
    bg->addButton(mScaleZeroRadio);
    connect(mScaleZeroRadio, SIGNAL(toggled(bool)), SLOT(slotUpdatePlot()));
    gl->addWidget(mScaleZeroRadio, 3, 10);

    gl->setColumnStretch(11, 1);

    setMainWidget(w);

    KConfigGroup grp = KGlobal::config()->group(objectName());
    restoreDialogSize(grp);

    mElevationCheck->setChecked(grp.readEntry("ShowElevation", true));
    mSpeedCheck->setChecked(grp.readEntry("ShowSpeed", true));
    mSpeedGpsRadio->setChecked(grp.readEntry("SpeedGps", false));
    mSpeedTrackRadio->setChecked(grp.readEntry("SpeedTrack", true));
    mReferenceTimeRadio->setChecked(grp.readEntry("ReferenceTime", true));
    mReferenceDistRadio->setChecked(grp.readEntry("ReferenceDist", false));
    mScaleAutoRadio->setChecked(grp.readEntry("ScaleAuto", true));
    mScaleZeroRadio->setChecked(grp.readEntry("ScaleZero", false));

    if (!mElevationCheck->isChecked() && !mSpeedCheck->isChecked())
    {
        mElevationCheck->setChecked(true);
        mSpeedCheck->setChecked(true);
    }

    slotUpdatePlot();
}


ProfileWidget::~ProfileWidget()
{
    KConfigGroup grp = KGlobal::config()->group(objectName());
    saveDialogSize(grp);

    grp.writeEntry("ShowElevation", mElevationCheck->isChecked());
    grp.writeEntry("ShowSpeed", mSpeedCheck->isChecked());
    grp.writeEntry("SpeedGps", mSpeedGpsRadio->isChecked());
    grp.writeEntry("SpeedTrack", mSpeedTrackRadio->isChecked());
    grp.writeEntry("ReferenceTime", mReferenceTimeRadio->isChecked());
    grp.writeEntry("ReferenceDist", mReferenceDistRadio->isChecked());
    grp.writeEntry("ScaleAuto", mScaleAutoRadio->isChecked());
    grp.writeEntry("ScaleZero", mScaleZeroRadio->isChecked());

    kDebug() << "done";
}


void ProfileWidget::getPlotData(const TrackDataItem *item)
{
    const TrackDataPoint *tdp = dynamic_cast<const TrackDataPoint *>(item);
    if (tdp!=NULL)					// is this a point?
    {
        double distStep = 0;
        double timeStep = 0;

        const QDateTime dt = tdp->time();

        if (mPrevPoint!=NULL)
        {
            // TODO: variable units
            distStep = mPrevPoint->distanceTo(tdp, true)*6371;	// distance travelled this step
            timeStep = mPrevPoint->timeTo(tdp);			// time interval this step
        }

        mCumulativeTravel += distStep;
        mPrevPoint = tdp;

        // Reference: either cumulative travel distance, or GPS time (if valid)
        if (mUseTravelDistance)				// plot against distance
        {
            mRefData.append(mCumulativeTravel);
        }
        else						// plot against time
        {
            if (!dt.isValid()) return;			// no time available this point
            mRefData.append(dt.toTime_t());
        }

        // Elevation
        const double ele = tdp->elevation();
        mElevData.append(ele);

        // Speed, either from GPS or calculated from track
        double spd;
        if (mUseGpsSpeed)				// GPS speed
        {
            const QString speedMeta = tdp->metadata("speed");
            spd = (!speedMeta.isEmpty() ? speedMeta.toDouble() : 0);
        }
        else					// speed from track
        {
            if (timeStep==0) spd = 0;
            else spd = (distStep*3600)/timeStep;
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
    kDebug();

    const bool speedEnabled = mSpeedCheck->isChecked();
    mSpeedGpsRadio->setEnabled(speedEnabled);
    mSpeedTrackRadio->setEnabled(speedEnabled);

    mUseGpsSpeed = mSpeedGpsRadio->isChecked();
    mUseTravelDistance = mReferenceDistRadio->isChecked();

    mCumulativeTravel = 0;
    mPrevPoint = NULL;

    const QList<TrackDataItem *> items = mainWindow()->filesController()->view()->selectedItems();
    mRefData.clear();
    mElevData.clear();
    mSpeedData.clear();

    for (int i = 0; i<items.count(); ++i) getPlotData(items[i]);
    kDebug() << "got" << mRefData.count() << "data points";

    QCPGraph *graph = mPlot->graph(0);			// elevation graph
    graph->setData(mRefData, mElevData);
    graph->setVisible(mElevationCheck->isChecked());
    graph->setPen(QPen(Qt::red));
    graph->rescaleValueAxis();

    graph = mPlot->graph(1);				// speed graph
    graph->setData(mRefData, mSpeedData);
    graph->setVisible(speedEnabled);
    graph->setPen(QPen(Qt::blue));
    graph->rescaleValueAxis();

    if (mReferenceTimeRadio->isChecked())		// reference axis
    {
        mPlot->xAxis->setLabel(i18n("Time"));
        mPlot->xAxis->setTickLabelType(QCPAxis::ltDateTime);
        mPlot->xAxis->setDateTimeFormat("hh:mm");
    }
    else
    {
        mPlot->xAxis->setLabel(i18n("Travel (km)"));
        mPlot->xAxis->setTickLabelType(QCPAxis::ltNumber);
        mPlot->xAxis->setNumberFormat("g");
    }
    mPlot->xAxis->rescale();

    mPlot->yAxis->setLabel(i18n("Elevation (m)"));	// elevation axis
    mPlot->yAxis->setLabelColor(Qt::red);
    mPlot->yAxis->setVisible(true);

    if (mUseGpsSpeed)					// speed axis
    {
        mPlot->yAxis2->setLabel(i18n("Speed (GPS)"));
    }
    else
    {
        mPlot->yAxis2->setLabel(i18n("Speed (km/h)"));
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
