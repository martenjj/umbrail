
#include "profilewidget.h"

#include <time.h>
#include <float.h>

#include <qgridlayout.h>
#include <qvector.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qradiobutton.h>
#include <qbuttongroup.h>
#include <qlabel.h>
#include <qtimer.h>
#include <qdebug.h>

#include <klocalizedstring.h>
#include <kconfiggroup.h>
#include <kiconloader.h>
#include <kcolorscheme.h>

#include <dialogstatewatcher.h>

#include "qcustomplot.h"
#include "mainwindow.h"
#include "filescontroller.h"
#include "filesview.h"
#include "filesmodel.h"
#include "trackdata.h"
#include "variableunitcombo.h"
#include "units.h"
#include "elevationmanager.h"
#include "elevationtile.h"

//////////////////////////////////////////////////////////////////////////
//									//
//  Debugging switches							//
//									//
//////////////////////////////////////////////////////////////////////////

#undef DEBUG_WAYPOINTS
#define DEBUG_WAYPOINTS

//////////////////////////////////////////////////////////////////////////
//									//
// Painting parameters							//
//									//
//////////////////////////////////////////////////////////////////////////

#define ASSOCIATE_DISTANCE	(100.0/(6371*1000))	// distance tolerance in metres
#define MARKER_SIZE		8			// if no icon image available

//////////////////////////////////////////////////////////////////////////
//									//
//  WaypointLayerable							//
//									//
//  A layerable item that can be added to the plot.  It draws the	//
//  applicable waypoints on the elevation line.				//
//									//
//////////////////////////////////////////////////////////////////////////

class WaypointLayerable : public QCPLayerable
{
public:
    explicit WaypointLayerable(QCustomPlot *plot, QString targetLayer = QString(), QCPLayerable *parentLayerable = nullptr);

    void setData(const QMap<const TrackDataAbstractPoint *, int> *waypoints,
                 const QVector<double> *refData,
                 const QVector<double> *elevData);

protected:
    void draw(QCPPainter *painter) override;
    void applyDefaultAntialiasingHint(QCPPainter *painter) const override;

private:
    const QMap<const TrackDataAbstractPoint *, int> *mWaypoints;
    const QVector<double> *mRefData;
    const QVector<double> *mElevData;
};


WaypointLayerable::WaypointLayerable(QCustomPlot *plot, QString targetLayer, QCPLayerable *parentLayerable)
    : QCPLayerable(plot, targetLayer, parentLayerable)
{
#ifdef DEBUG_WAYPOINTS
    qDebug() << "created on layer" << targetLayer;
#endif

    mWaypoints = nullptr;
    mRefData = nullptr;
    mElevData = nullptr;
}


void WaypointLayerable::setData(const QMap<const TrackDataAbstractPoint *, int> *waypoints,
                                const QVector<double> *refData,
                                const QVector<double> *elevData)
{
    mWaypoints = waypoints;
    mRefData = refData;
    mElevData = elevData;
}


void WaypointLayerable::draw(QCPPainter *painter)
{
    if (mWaypoints==nullptr || mRefData==nullptr || mElevData==nullptr) return;
    if (mRefData->isEmpty()) return;			// data not extracted yet
#ifdef DEBUG_WAYPOINTS
    qDebug() << "have" << mWaypoints->count() << "waypoints";
#endif

    QCustomPlot *plot = parentPlot();
    Q_ASSERT(plot!=nullptr);

    const QRect axisRect = plot->axisRect(0)->rect();	// plot rectangle (inside axes)
    const QCPGraph *graph = plot->graph(0);		// the elevation graph

    for (auto it = mWaypoints->constBegin(); it!=mWaypoints->constEnd(); ++it)
    {
        const TrackDataAbstractPoint *tdw = it.key();
        const int idx = it.value();

        const double ele = mElevData->at(idx);
        if (isnan(ele))
        {
            qDebug() << "no elevation for" << tdw->name();
            continue;
        }

        QPointF pos = graph->coordsToPixels(mRefData->at(idx), mElevData->at(idx));
#ifdef DEBUG_WAYPOINTS
        qDebug() << "plot" << tdw->name() << "at" << pos;
#endif

        // First draw the position/time line
        painter->setPen(QPen(Qt::green, 1));
        painter->drawLine(QPointF(pos.x(), axisRect.top()), QPointF(pos.x(), axisRect.bottom()-2));

        // Then the waypoint icon image, if available
        const QPixmap img = tdw->icon().pixmap(KIconLoader::SizeSmall);
        if (!img.isNull())				// icon image available
        {
            QPointF coord(pos.x()-(img.width()/2), pos.y()-(img.height()/2));
            painter->drawPixmap(coord, img);
        }
        else						// draw our own marker
        {
            painter->setPen(QPen(Qt::red, 2));
            painter->setBrush(Qt::yellow);
            painter->drawEllipse(pos, MARKER_SIZE, MARKER_SIZE);
        }

        // Followed by the waypoint text
        painter->save();
        painter->translate(14, 2);			// offset text from point
        painter->setPen(Qt::black);
        painter->drawText(pos, tdw->name());
        painter->restore();
    }
}


void WaypointLayerable::applyDefaultAntialiasingHint(QCPPainter *painter) const
{
}

//////////////////////////////////////////////////////////////////////////
//									//
//  ProfileWidget							//
//									//
//////////////////////////////////////////////////////////////////////////

ProfileWidget::ProfileWidget(QWidget *pnt)
    : DialogBase(pnt),
      DialogStateSaver(this),
      MainWindowInterface(pnt)
{
    qDebug();

    setObjectName("ProfileWidget");
    setButtons(QDialogButtonBox::Close);

    mTimeZone = NULL;

    filesController()->view()->selectedPoints().swap(mPoints);

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

    ++col;						// New column: source labels

    ++col;						// New column: source combos
    mElevationSourceCombo = new QComboBox(this);
    mElevationSourceCombo->addItem(i18n("GPS"), ElevationSourceGPS);
    mElevationSourceCombo->addItem(i18n("DEM"), ElevationSourceDEM);
    connect(mElevationSourceCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            mUpdateTimer, static_cast<void (QTimer::*)(void)>(&QTimer::start));
    l = new QLabel(i18n("source"), this);
    l->setBuddy(mElevationSourceCombo);
    gl->addWidget(l, 2, col-1);
    gl->addWidget(mElevationSourceCombo, 2, col);

    mSpeedSourceCombo = new QComboBox(this);
    mSpeedSourceCombo->addItem(i18n("GPS"), SpeedSourceGPS);
    mSpeedSourceCombo->addItem(i18n("Track"), SpeedSourceTrack);
    connect(mSpeedSourceCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            mUpdateTimer, static_cast<void (QTimer::*)(void)>(&QTimer::start));
    l = new QLabel(i18n("source"), this);
    l->setBuddy(mSpeedSourceCombo);
    gl->addWidget(l, 3, col-1);
    gl->addWidget(mSpeedSourceCombo, 3, col);

    ++col;						// New column: spacer
    gl->setColumnMinimumWidth(col, DialogBase::horizontalSpacing());

    ++col;						// New column: reference label
    l = new QLabel(i18n("Reference:"), this);
    gl->addWidget(l, 2, col, Qt::AlignRight);

    ++col;						// New column: reference radio
    QButtonGroup *bg = new QButtonGroup(this);

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

    // Set up a new layer on which to draw the waypoints.  Assuming that
    // getting 'overlayLayer' from the plot succeeds, the new layer will
    // be inserted just below that overlay layer, in other words second down
    // from the top.  If getting the overlay layer fails then the new layer will
    // be inserted at the top of the stack anyway, so hopefully the
    // addLayer() can never fail.
    QCPLayer *overlayLayer = mPlot->layer("overlay");

    bool added = mPlot->addLayer("waypoints", overlayLayer,  QCustomPlot::limBelow);
    Q_ASSERT(added);

    QCPLayer *waypointLayer = mPlot->layer("waypoints");
    Q_ASSERT(waypointLayer!=nullptr);
    mWaypointLayerable = new WaypointLayerable(mPlot, "waypoints");
							// only needs to be done once
    associateWaypoints(mainWindow()->filesController()->model()->rootFileItem());
    qDebug() << "found" << mWaypoints.count() << "associated waypoints";

    connect(ElevationManager::self(), &ElevationManager::tileReady,
            this, [this](const ElevationTile *tile){ mUpdateTimer->start(); });
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

    int val = grp.readEntry("ElevationSource", static_cast<int>(ElevationSourceGPS));
    int idx = mElevationSourceCombo->findData(val);
    if (idx!=-1) mElevationSourceCombo->setCurrentIndex(idx);
    val = grp.readEntry("SpeedSource", static_cast<int>(SpeedSourceGPS));
    idx = mSpeedSourceCombo->findData(val);
    if (idx!=-1) mSpeedSourceCombo->setCurrentIndex(idx);

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
    grp.writeEntry("ElevationSource", mElevationSourceCombo->currentData().toInt());
    grp.writeEntry("SpeedSource", mSpeedSourceCombo->currentData().toInt());
    grp.writeEntry("ReferenceTime", mReferenceTimeRadio->isChecked());
    grp.writeEntry("ReferenceDist", mReferenceDistRadio->isChecked());
    grp.writeEntry("UnitTime", mTimeUnit->currentIndex());
    grp.writeEntry("UnitDistance", mDistanceUnit->currentIndex());
    grp.writeEntry("ScaleAuto", mScaleAutoRadio->isChecked());
    grp.writeEntry("ScaleZero", mScaleZeroRadio->isChecked());

    DialogStateSaver::saveConfig(dialog, grp);
}


void ProfileWidget::getPlotData(const TrackDataAbstractPoint *point)
{
    const TrackDataTrackpoint *tdp = dynamic_cast<const TrackDataTrackpoint *>(point);
    if (tdp!=NULL)					// is this a track point?
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

        mCumulativeTravel += Units::internalToLength(distStep, mDistanceUnit->unit());
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
                if (mTimeZone!=NULL) qDebug() << "time zone offset" << mTimeZone->offsetFromUtc(dt);
            }

            if (mTimeUnit->unit()==Units::TimeRelative)
            {						// to make times start at zero
                tm = static_cast<time_t>(difftime(tm, mBaseTime));
            }
            mRefData.append(tm);
        }

        // Elevation, data always in metres
        double ele = NAN;
        if (mElevationSource==ElevationSourceGPS)	// GPS elevation
        {
            ele = tdp->elevation();
        }
        else						// DEM elevation
        {
            const double lat = tdp->latitude();
            const double lon = tdp->longitude();
            const ElevationTile *tile = ElevationManager::self()->requestTile(lat, lon, false);
            if (tile->state()==ElevationTile::Loaded) ele = tile->elevation(lat, lon);
        }
        mElevData.append(Units::internalToElevation(ele, mElevationUnit->unit()));

        // Speed, either from GPS or calculated from track
        double spd;
        if (mSpeedSource==SpeedSourceGPS)		// GPS speed
        {
            // Get the speed from the track metadata.
            const QString speedMeta = tdp->metadata("speed");
            if (speedMeta.isEmpty()) spd = NAN;
            else
            {
                // First convert the GPS speed (in metres/second, defined in
                // the GPX specification) into internal units.
                spd = Units::speedToInternal(speedMeta.toDouble(), Units::SpeedMetresSecond);
                // Then convert that speed back to the requested unit.
                spd = Units::internalToSpeed(spd, mSpeedUnit->unit());
            }
        }
        else						// speed from track
        {
            // Calculate speed based on the distance and time from the previous point
            if (timeStep==0) spd = NAN;
            else
            {
                // First get the distance in metres.
                spd = Units::internalToLength(distStep, Units::LengthMetres);
                // Then calculate the speed in metres/second
                spd /= timeStep;
                // Convert the speed (at this point known to be in
                // metres/second) into internal units.
                spd = Units::speedToInternal(spd, Units::SpeedMetresSecond);
                // Then convert that speed back to the requested unit.
                spd = Units::internalToSpeed(spd, mSpeedUnit->unit());
            }
        }
        mSpeedData.append(spd);
    }
}





void ProfileWidget::slotUpdatePlot()
{
    const bool speedEnabled = mSpeedCheck->isChecked();
    mSpeedSourceCombo->setEnabled(speedEnabled);
    mSpeedUnit->setEnabled(speedEnabled);

    const bool elevationEnabled = mElevationCheck->isChecked();
    mElevationSourceCombo->setEnabled(elevationEnabled);
    mElevationUnit->setEnabled(elevationEnabled);

    mElevationSource = static_cast<ElevationSource>(mElevationSourceCombo->currentData().toInt());
    mSpeedSource = static_cast<SpeedSource>(mSpeedSourceCombo->currentData().toInt());
    mUseTravelDistance = mReferenceDistRadio->isChecked();

    mTimeUnit->setEnabled(!mUseTravelDistance);
    mDistanceUnit->setEnabled(mUseTravelDistance);

    mCumulativeTravel = 0;
    mPrevPoint = NULL;

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

    for (int i = 0; i<mPoints.count(); ++i) getPlotData(mPoints.at(i));
    qDebug() << "got" << mRefData.count() << "data points";
    mWaypointLayerable->setData(&mWaypoints, &mRefData, &mElevData);

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

        if (mTimeTicker.isNull())			// needs to be created now
        {
            qDebug() << "creating time ticker";
            QCPAxisTickerDateTime *ticker = new QCPAxisTickerDateTime();
            ticker->setDateTimeFormat("hh:mm");
            // See the comment in getPlotData() above
            ticker->setDateTimeSpec(Qt::UTC);
            mTimeTicker.reset(ticker);
        }
        mPlot->xAxis->setTicker(mTimeTicker);
    }
    else
    {
        mPlot->xAxis->setLabel(i18n("Travel (%1)", mDistanceUnit->currentText()));

        if (mNumberTicker.isNull())			// needs to be created now
        {
            qDebug() << "creating number ticker";
            QCPAxisTicker *ticker = new QCPAxisTicker();
            mNumberTicker.reset(ticker);
        }
        mPlot->xAxis->setTicker(mNumberTicker);
    }
    mPlot->xAxis->rescale();

    KColorScheme sch(QPalette::Disabled);		// for getting disabled colour

    if (mElevationSource==ElevationSourceGPS)		// elevation axis
    {
        mPlot->yAxis->setLabel(i18n("Elevation (GPS %1)", mElevationUnit->currentText()));
    }
    else
    {
        mPlot->yAxis->setLabel(i18n("Elevation (DEM %1)", mElevationUnit->currentText()));
    }
    mPlot->yAxis->setLabelColor(!elevationEnabled ? sch.foreground(KColorScheme::NormalText).color() : Qt::red);
    mPlot->yAxis->setVisible(true);

    if (mSpeedSource==SpeedSourceGPS)			// speed axis
    {
        mPlot->yAxis2->setLabel(i18n("Speed (GPS %1)", mSpeedUnit->currentText()));
    }
    else
    {
        mPlot->yAxis2->setLabel(i18n("Speed (%1)", mSpeedUnit->currentText()));
    }
    mPlot->yAxis2->setLabelColor(!speedEnabled ? sch.foreground(KColorScheme::NormalText).color() : Qt::blue);
    mPlot->yAxis2->setVisible(true);

    if (mScaleZeroRadio->isChecked())			// zero origin
    {
        mPlot->yAxis->setRangeLower(0);
        mPlot->yAxis2->setRangeLower(0);
    }

    mPlot->replot();
}


void ProfileWidget::associateWaypoints(const TrackDataItem *item)
{
    const TrackDataAbstractPoint *tdp = nullptr;
    const TrackDataWaypoint *tdw = dynamic_cast<const TrackDataWaypoint *>(item);
    if (tdw!=nullptr) tdp = tdw;
    const TrackDataRoutepoint *tdr = dynamic_cast<const TrackDataRoutepoint *>(item);
    if (tdr!=nullptr) tdp = tdr;

    if (tdp!=NULL)
    {
#ifdef DEBUG_WAYPOINTS
        qDebug() << "trying" << tdp->name();
#endif
        const TrackDataAbstractPoint *closestPoint = nullptr;
        double closestDist = FLT_MAX;
        int closestIndex;

        for (int i = 0; i<mPoints.count(); ++i)
        {
            const TrackDataAbstractPoint *pnt = mPoints.at(i);
            double dist = tdp->distanceTo(pnt);
            if (dist<closestDist)			// closest approach so far?
            {
                closestPoint = pnt;
                closestDist = dist;
                closestIndex = i;
            }
        }

        if (closestPoint==nullptr)			// couldn't find anything
        {
#ifdef DEBUG_WAYPOINTS
            qDebug() << "  no closest point found";
#endif
        }
        else
        {
#ifdef DEBUG_WAYPOINTS
            qDebug() << "  closest point" << closestPoint->name() << "dist" << closestDist;
#endif
            if (closestDist<ASSOCIATE_DISTANCE)
            {
#ifdef DEBUG_WAYPOINTS
                qDebug() << "  accept at index" << closestIndex;
#endif
                mWaypoints.insert(tdp, closestIndex);
            }
            else
            {
#ifdef DEBUG_WAYPOINTS
                qDebug() << "  too far away";
#endif
            }

        }
    }

    for (int i = 0; i<item->childCount(); ++i) associateWaypoints(item->childAt(i));
}
