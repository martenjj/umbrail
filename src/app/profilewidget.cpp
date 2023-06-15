//////////////////////////////////////////////////////////////////////////
//									//
//  Project:	Umbrail - GPX track viewer and editor			//
//									//
//////////////////////////////////////////////////////////////////////////
//									//
//  Copyright (c) 2014-2021 Jonathan Marten <jjm@keelhaul.me.uk>	//
//  Home and download page: <http://github.com/martenjj/umbrail>	//
//									//
//  This program is free software; you can redistribute it and/or	//
//  modify it under the terms of the GNU General Public License as	//
//  published by the Free Software Foundation, either version 3 of	//
//  the License or (at your option) any later version.			//
//									//
//  It is distributed in the hope that it will be useful, but		//
//  WITHOUT ANY WARRANTY;  without even the implied warranty of		//
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the	//
//  GNU General Public License for more details.			//
//									//
//  You should have received a copy of the GNU General Public License	//
//  along with this program;  see the file COPYING for further		//
//  details.  If not, see <http://gnu.org/licenses/gpl>.      		//
//									//
//////////////////////////////////////////////////////////////////////////

#include "profilewidget.h"

#include <time.h>
#include <float.h>

#include <qgridlayout.h>
#include <qvector.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qradiobutton.h>
#include <qpushbutton.h>
#include <qbuttongroup.h>
#include <qlabel.h>
#include <qtimer.h>
#include <qdebug.h>

#include <klocalizedstring.h>
#include <kconfiggroup.h>
#include <kiconloader.h>
#include <kcolorscheme.h>

#include <kfdialog/dialogstatewatcher.h>

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

#define ASSOCIATE_DISTANCE	(100.0/(6371*1000))	// distance tolerance, metres
#define MARKER_SIZE		8			// if no icon image available
#define ROUTEPOINT_STEP		50			// point separation, metres

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
    void setShowPoints(WaypointSelectDialogue::SelectionSet sel);

protected:
    void draw(QCPPainter *painter) override;
    void applyDefaultAntialiasingHint(QCPPainter *painter) const override;

private:
    bool isShowingPoint(const TrackDataAbstractPoint *pnt) const;

private:
    const QMap<const TrackDataAbstractPoint *, int> *mWaypoints;
    const QVector<double> *mRefData;
    const QVector<double> *mElevData;
    WaypointSelectDialogue::SelectionSet mSelection;
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


void WaypointLayerable::setShowPoints(WaypointSelectDialogue::SelectionSet sel)
{
    mSelection = sel;
}


bool WaypointLayerable::isShowingPoint(const TrackDataAbstractPoint *pnt) const
{
    if (dynamic_cast<const TrackDataRoutepoint *>(pnt)!=nullptr)
    {							// is this a route point?
        return (mSelection & WaypointSelectDialogue::SelectRoutepoints);
    }

    const TrackDataWaypoint *tdw = dynamic_cast<const TrackDataWaypoint *>(pnt);
    if (tdw==nullptr) return (false);			// otherwise, should be a waypoint

    const TrackData::WaypointType type = tdw->waypointType();
    switch (type)
    {
case TrackData::WaypointNormal:		return (mSelection & WaypointSelectDialogue::SelectWaypoints);
case TrackData::WaypointAudioNote:	return (mSelection & WaypointSelectDialogue::SelectAudioNotes);
case TrackData::WaypointVideoNote:	return (mSelection & WaypointSelectDialogue::SelectVideoNotes);
case TrackData::WaypointPhoto:		return (mSelection & WaypointSelectDialogue::SelectPhotos);
case TrackData::WaypointStop:		return (mSelection & WaypointSelectDialogue::SelectStops);
default:				return (false);
    }
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
        if (!isShowingPoint(tdw))			// showing this type of point?
        {
#ifdef DEBUG_WAYPOINTS
            qDebug() << "not showing" << tdw->name();
#endif
            continue;
        }

        const int idx = it.value();

        const double ele = mElevData->at(idx);
        if (ISNAN(ele))
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
      ApplicationDataInterface(pnt)
{
    setObjectName("ProfileWidget");
    setButtons(QDialogButtonBox::Close);

    // Get the selected points.
    filesController()->view()->selectedPoints().swap(mPoints);
    Q_ASSERT(!mPoints.isEmpty());

    // See if the first of those is a route point.  If so, assume that all of them are
    // and the plot is in route mode (interpolated points, limited options).
    const TrackDataRoutepoint *tdr = dynamic_cast<const TrackDataRoutepoint *>(mPoints.first());
    mRouteMode = (tdr!=nullptr);
    qDebug() << "route mode?" << mRouteMode;

    // If a route, rewrite the selected point data to contain the original route points
    // with intermediate points interpolated between them.
    if (mRouteMode) getRoutePoints();

    // Now it is possible to set the window title appropriately.
    setWindowTitle(mRouteMode ? i18n("Route Profile") : i18n("Track Profile"));

    // Resolve the file time zone.
    mTimeZone = nullptr;
    QVariant zoneName = filesController()->model()->rootFileItem()->metadata("timezone");
    if (!zoneName.isNull())
    {
        QTimeZone *tz = new QTimeZone(zoneName.toByteArray());
        if (tz->isValid()) mTimeZone = tz;		// use as time zone
        else qWarning() << "unknown time zone" << zoneName;
    }

    mUpdateTimer = new QTimer(this);
    mUpdateTimer->setSingleShot(true);
    mUpdateTimer->setInterval(50);
    connect(mUpdateTimer, &QTimer::timeout, this, &ProfileWidget::slotUpdatePlot);

    QWidget *w = new QWidget(this);
    QGridLayout *gl = new QGridLayout(w);

    int col = 0;

    mPlot = new QCustomPlot(this);
    mPlot->addGraph(mPlot->xAxis, mPlot->yAxis);
    if (!mRouteMode) mPlot->addGraph(mPlot->xAxis, mPlot->yAxis2);
    gl->addWidget(mPlot, 0, col, 1, -1);

    gl->setRowStretch(0, 1);
    gl->setRowMinimumHeight(1, DialogBase::verticalSpacing());
							// First column: elevation/speed label
    QLabel *l = new QLabel(i18n("Show:"), this);
    gl->addWidget(l, 2, 0, Qt::AlignRight);

    ++col;						// New column: elevation/speed checks
    mElevationCheck = new QCheckBox(i18n("Elevation"), this);
    connect(mElevationCheck, &QAbstractButton::toggled, mUpdateTimer, QOverload<>::of(&QTimer::start));
    gl->addWidget(mElevationCheck, 2, col);

    mSpeedCheck = new QCheckBox(i18n("Speed"), this);
    if (mRouteMode) mSpeedCheck->setEnabled(false);
    connect(mSpeedCheck, &QAbstractButton::toggled, mUpdateTimer, QOverload<>::of(&QTimer::start));
    gl->addWidget(mSpeedCheck, 3, col);

    ++col;						// New column: elevation/speed units
    mElevationUnit = new VariableUnitCombo(VariableUnitCombo::Elevation);
    connect(mElevationUnit, QOverload<int>::of(&QComboBox::currentIndexChanged),
            mUpdateTimer, QOverload<>::of(&QTimer::start));
    gl->addWidget(mElevationUnit, 2, col);

    mSpeedUnit = new VariableUnitCombo(VariableUnitCombo::Speed);
    connect(mSpeedUnit, QOverload<int>::of(&QComboBox::currentIndexChanged),
            mUpdateTimer, QOverload<>::of(&QTimer::start));
    gl->addWidget(mSpeedUnit, 3, col);

    ++col;						// New column: source labels

    ++col;						// New column: source combos
    mElevationSourceCombo = new QComboBox(this);
    mElevationSourceCombo->addItem(i18n("GPS"), ElevationSourceGPS);
    mElevationSourceCombo->addItem(i18n("DEM"), ElevationSourceDEM);
    if (mRouteMode)
    {
        mElevationSourceCombo->setCurrentIndex(1);	// only DEM allowed for route
        mElevationSourceCombo->setEnabled(false);
    }
    connect(mElevationSourceCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            mUpdateTimer, QOverload<>::of(&QTimer::start));
    l = new QLabel(i18n("source"), this);
    l->setBuddy(mElevationSourceCombo);
    gl->addWidget(l, 2, col-1);
    gl->addWidget(mElevationSourceCombo, 2, col);

    mSpeedSourceCombo = new QComboBox(this);
    mSpeedSourceCombo->addItem(i18n("GPS"), SpeedSourceGPS);
    mSpeedSourceCombo->addItem(i18n("Track"), SpeedSourceTrack);
    connect(mSpeedSourceCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            mUpdateTimer, QOverload<>::of(&QTimer::start));
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
    if (mRouteMode) mReferenceTimeRadio->setEnabled(false);
    connect(mReferenceTimeRadio, &QAbstractButton::toggled, mUpdateTimer, QOverload<>::of(&QTimer::start));
    bg->addButton(mReferenceTimeRadio);
    gl->addWidget(mReferenceTimeRadio, 2, col);

    mReferenceDistRadio = new QRadioButton(i18n("Distance"), this);
    if (mRouteMode) mReferenceDistRadio->setChecked(true);
    connect(mReferenceDistRadio, &QAbstractButton::toggled, mUpdateTimer, QOverload<>::of(&QTimer::start));
    bg->addButton(mReferenceDistRadio);
    gl->addWidget(mReferenceDistRadio, 3, col);

    ++col;						// New column: distance/time units

    mTimeUnit = new VariableUnitCombo(VariableUnitCombo::Time);
    connect(mTimeUnit, QOverload<int>::of(&QComboBox::currentIndexChanged), mUpdateTimer, QOverload<>::of(&QTimer::start));
    gl->addWidget(mTimeUnit, 2, col);

    mDistanceUnit = new VariableUnitCombo(VariableUnitCombo::Distance);
    connect(mDistanceUnit, QOverload<int>::of(&QComboBox::currentIndexChanged), mUpdateTimer, QOverload<>::of(&QTimer::start));
    gl->addWidget(mDistanceUnit, 3, col);

    ++col;						// New column: spacer
    gl->setColumnMinimumWidth(col, DialogBase::horizontalSpacing());

    ++col;						// New column: scaling/waypoint label

    l = new QLabel(i18n("Waypoints:"), this);
    gl->addWidget(l, 3, col, Qt::AlignRight);

    l = new QLabel(i18n("Scaling:"), this);
    gl->addWidget(l, 2, col, Qt::AlignRight);

    ++col;						// New column: scaling combo, waypoint select

    mScaleRangeCombo = new QComboBox(this);
    mScaleRangeCombo->addItem(i18n("Auto Range"), ScaleRangeAuto);
    mScaleRangeCombo->addItem(i18n("Zero Origin"), ScaleRangeZero);
    connect(mScaleRangeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            mUpdateTimer, QOverload<>::of(&QTimer::start));
    l->setBuddy(mScaleRangeCombo);
    gl->addWidget(mScaleRangeCombo, 2, col);

    QPushButton *waypointSelectButton = new QPushButton(QIcon::fromTheme("visibility"), i18n("Show..."), this);
    connect(waypointSelectButton, &QAbstractButton::clicked, this, &ProfileWidget::slotSelectWaypoints);
    gl->addWidget(waypointSelectButton, 3, col);

    ++col;						// New column: stretch
    gl->setColumnStretch(col, 1);

    setMainWidget(w);
    setStateSaver(this);
    stateWatcher()->setSaveOnButton(buttonBox()->button(QDialogButtonBox::Close));

    if (!mElevationCheck->isChecked() && !mSpeedCheck->isChecked())
    {							// if nothing on, set both on
        mElevationCheck->setChecked(true);
        if (mSpeedCheck->isEnabled()) mSpeedCheck->setChecked(true);
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
    associateWaypoints(filesController()->model()->rootFileItem());
    qDebug() << "found" << mWaypoints.count() << "associated waypoints";
    mWaypointSelection = WaypointSelectDialogue::SelectWaypoints|WaypointSelectDialogue::SelectRoutepoints;

    connect(ElevationManager::self(), &ElevationManager::tileReady,
            this, [this](const ElevationTile *tile){ mUpdateTimer->start(); });
    mUpdateTimer->start();				// do the first plot update
}


ProfileWidget::~ProfileWidget()
{
    delete mTimeZone;
    qDeleteAll(mRoutePoints);
}


void ProfileWidget::restoreConfig(QDialog *dialog, const KConfigGroup &grp)
{
    mElevationCheck->setChecked(grp.readEntry("ShowElevation", true));
    if (mSpeedCheck->isEnabled()) mSpeedCheck->setChecked(grp.readEntry("ShowSpeed", true));
    mElevationUnit->setCurrentIndex(grp.readEntry("UnitElevation", 0));
    mSpeedUnit->setCurrentIndex(grp.readEntry("UnitSpeed", 0));

    int val = grp.readEntry("ScaleRange", static_cast<int>(ScaleRangeAuto));
    int idx = mScaleRangeCombo->findData(val);
    if (idx!=-1) mScaleRangeCombo->setCurrentIndex(idx);

    if (mElevationSourceCombo->isEnabled())
    {
        val = grp.readEntry("ElevationSource", static_cast<int>(ElevationSourceGPS));
        idx = mElevationSourceCombo->findData(val);
        if (idx!=-1) mElevationSourceCombo->setCurrentIndex(idx);
    }

    val = grp.readEntry("SpeedSource", static_cast<int>(SpeedSourceGPS));
    idx = mSpeedSourceCombo->findData(val);
    if (idx!=-1) mSpeedSourceCombo->setCurrentIndex(idx);

    if (mReferenceTimeRadio->isEnabled()) mReferenceTimeRadio->setChecked(grp.readEntry("ReferenceTime", true));
    if (mReferenceDistRadio->isEnabled()) mReferenceDistRadio->setChecked(grp.readEntry("ReferenceDist", false));
    mTimeUnit->setCurrentIndex(grp.readEntry("UnitTime", 0));
    mDistanceUnit->setCurrentIndex(grp.readEntry("UnitDistance", 0));

    mWaypointSelection = static_cast<WaypointSelectDialogue::Selection>(grp.readEntry("ShowWaypoints",
                                                                                      static_cast<int>(mWaypointSelection)));
    DialogStateSaver::restoreConfig(dialog, grp);
}


void ProfileWidget::saveConfig(QDialog *dialog, KConfigGroup &grp) const
{
    grp.writeEntry("ShowElevation", mElevationCheck->isChecked());
    if (mSpeedCheck->isEnabled()) grp.writeEntry("ShowSpeed", mSpeedCheck->isChecked());
    grp.writeEntry("UnitElevation", mElevationUnit->currentIndex());
    grp.writeEntry("UnitSpeed", mSpeedUnit->currentIndex());
    if (mElevationSourceCombo->isEnabled()) grp.writeEntry("ElevationSource", mElevationSourceCombo->currentData().toInt());
    grp.writeEntry("SpeedSource", mSpeedSourceCombo->currentData().toInt());
    grp.writeEntry("ScaleRange", mScaleRangeCombo->currentData().toInt());
    if (mReferenceTimeRadio->isEnabled()) grp.writeEntry("ReferenceTime", mReferenceTimeRadio->isChecked());
    if (mReferenceDistRadio->isEnabled()) grp.writeEntry("ReferenceDist", mReferenceDistRadio->isChecked());
    grp.writeEntry("UnitTime", mTimeUnit->currentIndex());
    grp.writeEntry("UnitDistance", mDistanceUnit->currentIndex());
    grp.writeEntry("ShowWaypoints", static_cast<int>(mWaypointSelection));

    DialogStateSaver::saveConfig(dialog, grp);
}


void ProfileWidget::getPlotData(const TrackDataAbstractPoint *point)
{
    // The point here can be either a track or a route point.  If the plot is for a
    // route, the selected route or route points will already have had interpolated
    // points (of type Trackpoint) added between them.

    double distStep = 0;
    double timeStep = 0;

    if (mPrevPoint!=nullptr)
    {
        distStep = mPrevPoint->distanceTo(point, true);	// distance travelled this step
        timeStep = mPrevPoint->timeTo(point);		// time interval this step
    }

    mCumulativeTravel += Units::internalToLength(distStep, mDistanceUnit->unit());
    mPrevPoint = point;					// note this as new previous

    // Reference: either cumulative travel distance, or GPS time (if valid)
    if (mUseTravelDistance)				// plot against distance
    {
        mRefData.append(mCumulativeTravel);
    }
    else						// plot against time
    {
        QDateTime dt = point->time();
        if (!dt.isValid()) return;			// no time available this point

        // do time zone conversion
        if (mTimeZone!=nullptr) dt = dt.toUTC().toTimeZone(*mTimeZone);

        time_t tm = dt.toTime_t();

        if (mTimeZone!=nullptr)				// file time zone available
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
            if (mTimeZone!=nullptr) qDebug() << "base time zone offset" << mTimeZone->offsetFromUtc(dt);
        }

        if (mTimeUnit->unit()==Units::TimeRelative)
        {						// to make times start at zero
            tm = static_cast<time_t>(difftime(tm, mBaseTime));
        }
        mRefData.append(tm);
    }

    // Elevation, data always in metres
    double ele = NAN;
    if (mElevationSource==ElevationSourceGPS)		// GPS elevation
    {
        // Not expected to be valid for route plotting.
        ele = point->elevation();
    }
    else						// DEM elevation
    {
        const double lat = point->latitude();
        const double lon = point->longitude();
        const ElevationTile *tile = ElevationManager::self()->requestTile(lat, lon, false);
        if (tile->state()==ElevationTile::Loaded) ele = tile->elevation(lat, lon);
    }
    mElevData.append(Units::internalToElevation(ele, mElevationUnit->unit()));

    // Speed, either from GPS or calculated from track
    double spd = NAN;
    if (!mRouteMode)					// not for route plotting
    {
        if (mSpeedSource==SpeedSourceGPS)		// GPS speed
        {
            // Get the speed from the track metadata.
            const QVariant speedMeta = point->metadata("speed");
            if (speedMeta.isNull()) spd = NAN;
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
    }
    mSpeedData.append(spd);
}


void ProfileWidget::slotUpdatePlot()
{
    const bool speedEnabled = mSpeedCheck->isChecked();
    mSpeedSourceCombo->setEnabled(speedEnabled);
    mSpeedUnit->setEnabled(speedEnabled);

    const bool elevationEnabled = mElevationCheck->isChecked();
    mElevationSourceCombo->setEnabled(elevationEnabled && !mRouteMode);
    mElevationUnit->setEnabled(elevationEnabled);

    mElevationSource = static_cast<ElevationSource>(mElevationSourceCombo->currentData().toInt());
    mSpeedSource = static_cast<SpeedSource>(mSpeedSourceCombo->currentData().toInt());
    mUseTravelDistance = mReferenceDistRadio->isChecked();

    mTimeUnit->setEnabled(!mUseTravelDistance);
    mDistanceUnit->setEnabled(mUseTravelDistance);

    mCumulativeTravel = 0;
    mPrevPoint = nullptr;

    mRefData.clear();
    mElevData.clear();
    mSpeedData.clear();
    mBaseTime = 0;

    const int num = mPoints.count()-1;
    for (int i = 0; i<=num; ++i) getPlotData(mPoints.at(i));
    qDebug() << "got" << mRefData.count() << "data points";
    mWaypointLayerable->setData(&mWaypoints, &mRefData, &mElevData);
    mWaypointLayerable->setShowPoints(mWaypointSelection);

    QCPGraph *graph = mPlot->graph(0);			// elevation graph
    graph->setData(mRefData, mElevData);
    graph->setVisible(elevationEnabled);
    graph->setPen(QPen(Qt::red));
    graph->rescaleValueAxis();

    if (!mRouteMode)
    {
        graph = mPlot->graph(1);			// speed graph
        graph->setData(mRefData, mSpeedData);
        graph->setVisible(speedEnabled);
        graph->setPen(QPen(Qt::blue));
        graph->rescaleValueAxis();
    }

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
        mPlot->xAxis->setLabel(i18n("Distance (%1)", mDistanceUnit->currentText()));

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

    if (mSpeedSource==SpeedSourceGPS)		// speed axis
    {
        mPlot->yAxis2->setLabel(i18n("Speed (GPS %1)", mSpeedUnit->currentText()));
    }
    else
    {
        mPlot->yAxis2->setLabel(i18n("Speed (From track %1)", mSpeedUnit->currentText()));
    }
    mPlot->yAxis2->setLabelColor(!speedEnabled ? sch.foreground(KColorScheme::NormalText).color() : Qt::blue);
    mPlot->yAxis2->setVisible(!mRouteMode);

    if (mScaleRangeCombo->currentData()==ScaleRangeZero)
    {							// zero origin
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

    if (tdp!=nullptr)
    {
#ifdef DEBUG_WAYPOINTS
        qDebug() << "trying" << tdp->name();
#endif
        const TrackDataAbstractPoint *closestPoint = nullptr;
        double closestDist = FLT_MAX;
        // Should always get set because of the above,
        // but initialise to avoid a compiler warning.
        int closestIndex = -1;

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


void ProfileWidget::slotSelectWaypoints()
{
    WaypointSelectDialogue d(this);
    d.setSelection(mWaypointSelection);
    if (!d.exec()) return;

    mWaypointSelection = d.selection();
    mUpdateTimer->start();
}


void ProfileWidget::getRoutePoints()
{
    qDebug() << "originally" << mPoints.count() << "input points";

    QVector<const TrackDataAbstractPoint *> originalPoints;
    mPoints.swap(originalPoints);			// temporary copy of original

    const TrackDataAbstractPoint *prevPoint = originalPoints.first();
    mPoints.append(prevPoint);				// copy first route point

    for (int i1 = 1; i1<originalPoints.count(); ++i1)
    {
        const TrackDataAbstractPoint *thisPoint = originalPoints.at(i1);
        qDebug() << " " << prevPoint->name() << "->" << thisPoint->name();

        // Interpolate latitude/longitude between the previous point and
        // this one, not including that previous point but including this
        // as the last.

        const double dist = prevPoint->distanceTo(thisPoint);
        const double distMetres = Units::internalToLength(dist, Units::LengthMetres);
        const int num = static_cast<int>((distMetres+(ROUTEPOINT_STEP/2))/ROUTEPOINT_STEP);
        const double distStep = dist/num;
        qDebug() << "  distM" << distMetres << "num" << num << "step" << Units::internalToLength(distStep, Units::LengthMetres);

        double lat = prevPoint->latitude();
        double lon = prevPoint->longitude();

        const double latStep = (thisPoint->latitude()-lat)/num;
        const double lonStep = (thisPoint->longitude()-lon)/num;

        for (int i2 = 1; i2<num; ++i2)
        {
            lat += latStep;				// position of new point
            lon += lonStep;

            TrackDataTrackpoint *pnt = new TrackDataTrackpoint;
            pnt->setLatLong(lat, lon);			// create and set position

            mPoints.append(pnt);			// add to plotted points
            mRoutePoints.append(pnt);			// record to be deleted later
        }

        mPoints.append(thisPoint);			// finally the current point
        prevPoint = thisPoint;				// note this as new previous
    }

    qDebug() << "now" << mPoints.count() << "points";
}
