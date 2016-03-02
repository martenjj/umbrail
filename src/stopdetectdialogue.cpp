
#include "stopdetectdialogue.h"

#include <math.h>

#include <qformlayout.h>
#include <qlistwidget.h>
#include <qtimer.h>
#include <qlabel.h>
#include <qboxlayout.h>
#include <qgridlayout.h>
#include <qspinbox.h>
#include <qpushbutton.h>
#include <qcursor.h>
#include <qlineedit.h>

#include <kdebug.h>
#include <klocale.h>
#include <kseparator.h>
#include <ktimezone.h>
#include <ksystemtimezone.h>

#include "filescontroller.h"
#include "filesview.h"
#include "filesmodel.h"
#include "mapcontroller.h"
#include "mapview.h"
#include "trackdata.h"
#include "valueslider.h"
#include "mainwindow.h"
#include "folderselectwidget.h"
#include "commands.h"
#include "dataindexer.h"



static const int minInputPoints = 10;			// minimum points for detection
static const int minStopPoints = 3;			// minimum points for valid stop

// TODO: debugging switches


// TODO: explain algorithms here

#define MOVING_AVERAGING				// use "Moving Averaging" algorithm





StopDetectDialogue::StopDetectDialogue(QWidget *pnt)
    : KDialog(pnt)
{
    setObjectName("StopDetectDialogue");

    setAttribute(Qt::WA_DeleteOnClose);
    setModal(false);

    setCaption(i18nc("@title:window", "Locate Stops"));
    setButtons(KDialog::Ok|KDialog::Close);
    showButtonSeparator(true);
    enableButtonOk(false);
    setButtonText(KDialog::Ok, i18nc("@action:button", "Commit"));

    mIdleTimer = new QTimer(this);
    mIdleTimer->setInterval(2000);
    mIdleTimer->setSingleShot(true);			// don't start until show event
    connect(mIdleTimer, SIGNAL(timeout()), SLOT(slotDetectStops()));

    mMainWindow = qobject_cast<MainWindow *>(pnt);
    Q_ASSERT(mMainWindow!=NULL);

    QWidget *w = new QWidget(this);
    QHBoxLayout *hb = new QHBoxLayout(w);

    // Left hand side: parameters
    QFormLayout *fl = new QFormLayout;
    hb->addLayout(fl);

    QLabel *l = new QLabel(QString::null, w);
    fl->addRow(l);

    mTimeSlider = new ValueSlider(w, 30, 600, true, 120);
    mTimeSlider->setToolTip(i18n("The minimum length of time which will be considered to be a stop"));
    mTimeSlider->spinBox()->setSuffix(i18nc("abbreviation for seconds unit", " sec"));
    mTimeSlider->spinBox()->setSingleStep(10);
    connect(mTimeSlider, SIGNAL(settingChanged(int)), mIdleTimer, SLOT(start()));
    fl->addRow(i18n("Minimum duration:"), mTimeSlider);

    mDistanceSlider = new ValueSlider(w, 5, 200, true, 20);
    mDistanceSlider->setToolTip(i18n("The distance tolerance for points considered to be together"));
    mDistanceSlider->spinBox()->setSuffix(i18nc("abbreviation for metres unit", " m"));
    mDistanceSlider->spinBox()->setSingleStep(5);
    connect(mDistanceSlider, SIGNAL(settingChanged(int)), mIdleTimer, SLOT(start()));
    fl->addRow(i18n("Distance threshold:"), mDistanceSlider);

    mNoiseSlider = new ValueSlider(w, 0, 20, true, 2);
    mNoiseSlider->setToolTip(i18n("The number of outlying points which may be ignored"));
    connect(mNoiseSlider, SIGNAL(settingChanged(int)), mIdleTimer, SLOT(start()));
    fl->addRow(i18n("Noise points:"), mNoiseSlider);

    fl->addRow(new QLabel(QString::null, this));

    mFolderSelect = new FolderSelectWidget(mainWindow(), this);
    connect(mFolderSelect, SIGNAL(folderChanged(const QString &)), SLOT(slotSetButtonStates()));
    fl->addRow(i18n("Destination folder:"), mFolderSelect);

    // Middle: separator
    KSeparator *sep = new KSeparator(Qt::Vertical, w);
    hb->addSpacing(KDialog::spacingHint());
    hb->addWidget(sep);
    hb->addSpacing(KDialog::spacingHint());

    // Right hand side: results
    QGridLayout *gl = new QGridLayout;

    l = new QLabel(i18n("Stops found:"), w);
    gl->addWidget(l, 0, 0, Qt::AlignLeft);

    mResultsList = new QListWidget(w);
    mResultsList->setAlternatingRowColors(true);
    mResultsList->setSelectionBehavior(QAbstractItemView::SelectRows);
    mResultsList->setSelectionMode(QAbstractItemView::SingleSelection);
    mResultsList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    mResultsList->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    connect(mResultsList, SIGNAL(itemSelectionChanged()), SLOT(slotSetButtonStates()));
    connect(mResultsList, SIGNAL(itemChanged(QListWidgetItem *)), SLOT(slotSetButtonStates()));

    gl->addWidget(mResultsList, 1, 0);
    gl->setRowStretch(1, 1);
    l->setBuddy(mResultsList);

    mShowOnMapButton = new QPushButton(KIcon("marble"), i18n("Show on Map"), w);
    connect(mShowOnMapButton, SIGNAL(clicked(bool)), SLOT(slotShowOnMap()));
    gl->addWidget(mShowOnMapButton, 2, 0, Qt::AlignRight);

    connect(this, SIGNAL(accepted()), SLOT(slotCommitResults()));

    hb->addLayout(gl);

    setMainWidget(w);
    slotSetButtonStates();

    setMinimumSize(460, 280);
    KConfigGroup grp = KGlobal::config()->group(objectName());
    restoreDialogSize(grp);
}


StopDetectDialogue::~StopDetectDialogue()
{
    mainWindow()->mapController()->view()->setStopLayerData(NULL);
    qDeleteAll(mResultPoints);

    KConfigGroup grp = KGlobal::config()->group(objectName());
    saveDialogSize(grp);
    kDebug() << "done";
}


void StopDetectDialogue::showEvent(QShowEvent *ev)
{
    kDebug();
    QTimer::singleShot(0, this, SLOT(slotDetectStops()));
}


void StopDetectDialogue::slotShowOnMap()
{
    QList<QListWidgetItem *> items = mResultsList->selectedItems();
    if (items.count()!=1) return;
    int idx = items.first()->data(Qt::UserRole).toInt();

    TrackDataWaypoint *tdw = const_cast<TrackDataWaypoint *>(mResultPoints[idx]);

    kDebug() << "index" << idx << tdw->name();

    QList<TrackDataItem *> its;
    its.append(static_cast<TrackDataItem *>(tdw));
    mainWindow()->mapController()->gotoSelection(its);
}


static bool withinDistance(const TrackDataTrackpoint *tdp, double lat, double lon, int maxDist)
{
    double distance = qAbs(tdp->distanceTo(lat, lon))*(6371*1000);
    return (distance<=double(maxDist));			// convert to metres and check
}


static void getPointData(const TrackDataItem *item, QVector<const TrackDataTrackpoint *> *points)
{
    const TrackDataTrackpoint *tdp = dynamic_cast<const TrackDataTrackpoint *>(item);
    if (tdp!=NULL)					// is this a point?
    {
        if (isnan(tdp->latitude())) return;		// check position is valid
        if (isnan(tdp->longitude())) return;

        const QDateTime dt = tdp->time();		// check time is valid
        if (!dt.isValid()) return;

        points->append(tdp);
    }
    else						// not a point, recurse for children
    {
        const int num = item->childCount();
        for (int i = 0; i<num; ++i) getPointData(item->childAt(i), points);
    }
}


void StopDetectDialogue::slotDetectStops()
{
    kDebug() << "starting";

    const int minTime = mTimeSlider->value();
    const int maxDist = mDistanceSlider->value();
    const int noise = mNoiseSlider->value();
    kDebug() << "minTime" << minTime << "maxDist" << maxDist << "noise" << noise;

    setCursor(Qt::BusyCursor);

    mainWindow()->mapController()->view()->setStopLayerData(NULL);

    // Clear the result array but do not delete the points it contains,
    // previously committed points are now part of the main data tree.
    mResultPoints.clear();

    // Assemble all of the points of interest into a single linear list.
    // This list is assumed to be in time order.
    QVector<const TrackDataTrackpoint *> inputPoints;

    const QList<TrackDataItem *> items = mainWindow()->filesController()->view()->selectedItems();
    for (int i = 0; i<items.count(); ++i) getPointData(items[i], &inputPoints);
    kDebug() << "total points" << inputPoints.count();
    if (inputPoints.count()<minInputPoints)
    {
        kDebug() << "not enough points!";
        unsetCursor();
        return;
    }

// may need sorting for time here

    // Resolve the file time zone
    KTimeZone tz;
    QString zoneName = mainWindow()->filesController()->model()->rootFileItem()->metadata("timezone");
    if (!zoneName.isEmpty()) tz = KSystemTimeZones::zone(zoneName);

// Detect stops 
//
// Start at the first point, and check that within the next 'noise+1' points
// there is another which is within the 'maxDist' tolerance.  If there is,
// consider all the points found so far as part of a stop, update the current
// position to be the average of all the within-tolerance points, and continue
// the search from the next point.
//
// If no following point is found, then check that the total time span is
// longer than 'minTime'.  If it is, this is a valid stop so record it;  if
// it is too short, then forget this cluster.  Then start the search again
// from the next point.

    int startIndex = 0;					// start of points list
    const TrackDataTrackpoint *pnt;

    for (;;)						// loop 2: for each stop found
    {
        if (startIndex>=inputPoints.count()) break;	// reached end of points list

        kDebug() << "### loop2: startIndex" << startIndex;

        pnt = inputPoints[startIndex];			// initial point in run
        double runLat = pnt->latitude();		// centre of current run
        double runLon = pnt->longitude();

#ifdef MOVING_AVERAGING
        double sumLat = runLat;
        double sumLon = runLon;
        int sumNum = 1;
#endif // MOVING_AVERAGING

        int currentIndex = startIndex;

        for (;;)					// loop 1: looking for a run
        {
            kDebug() << "### loop1: currentIndex" << currentIndex;

            int foundIndex = -1;			// index of found point
            for (int i = 0; i<=noise; ++i)
            {
                int searchIndex = currentIndex+1+i;	// index of point being checked
                kDebug() << "  looking at searchIndex" << searchIndex;

                if (searchIndex>=inputPoints.count()) break;
							// end of points list
                pnt = inputPoints[searchIndex];
                if (withinDistance(pnt, runLat, runLon, maxDist))
                {					// within distance tolerance?
                    kDebug() << "  within distance tol";
                    foundIndex = searchIndex;
                    break;
                }
                else kDebug() << "  not within tol";
            }

            kDebug() << "### foundIndex" << foundIndex;

            if (foundIndex<0) break; // loop1

            // startIndex = index of start of this run
            // foundIndex = index of end of this run

            kDebug() << "run found from startIndex" << startIndex << "to foundIndex" << foundIndex;
            pnt = inputPoints[startIndex];
            kDebug() << "  start point" << pnt->name() << "found point" << inputPoints[foundIndex]->name();

#ifdef MOVING_AVERAGING
            pnt = inputPoints[foundIndex];
            sumLat += pnt->latitude();
            sumLon += pnt->longitude();
            ++sumNum;

            kDebug() << "average now of" << sumNum << "points";
            runLat = sumLat/sumNum;
            runLon = sumLon/sumNum;
#else // MOVING_AVERAGING
             double refLat = pnt->latitude();
             double refLon = pnt->longitude();

             double lat = refLat;
             double lon = refLon;
             int num = 1;
 
             for (int i = startIndex+1; i<=foundIndex; ++i)
             {
                 pnt = inputPoints[i];
                 if (withinDistance(pnt, refLat, refLon, maxDist))
                 {
                     kDebug() << "  including" << i << "in average";
                     lat += pnt->latitude();
                     lon += pnt->longitude();
                     ++num;
                 }
             }
 
             kDebug() << "average of" << num << "points";
             runLat = lat/num;
             runLon = lon/num;
#endif // MOVING_AVERAGING

            currentIndex = foundIndex;			// continue search at next point
        } // loop1

        // startIndex = index of start of this run
        // currentIndex = index of end of the run

        if ((currentIndex-startIndex)>=minStopPoints)
        {
            kDebug() << "******** stop found from startIndex" << startIndex << "to currentIndex" << currentIndex;

            const TrackDataTrackpoint *startPoint = inputPoints[startIndex];
            const TrackDataTrackpoint *endPoint = inputPoints[currentIndex];

            kDebug() << "  start point" << startPoint->name() << "end point" << endPoint->name();

            QDateTime dt1(startPoint->time());
            QDateTime dt2(endPoint->time());
            const int dur = dt1.secsTo(dt2);
            kDebug() << "  duration" << dur;

            if (dur>=minTime)				// longer than threshold?
            {
                kDebug() << "@@@@@@@@@@@ valid stop found";
							// do time zone conversion
                if (tz.isValid()) dt1 = tz.toZoneTime(dt1.toUTC());

                QString text1 = dt1.toString("hh:mm:ss");
                QString text2 = QString("%1:%2").arg(dur/60).arg(dur%60, 2, 10, QLatin1Char('0'));

                TrackDataWaypoint *tdw = new TrackDataWaypoint(i18n("Stop at %1 for %2", text1, text2));
                tdw->setLatLong(runLat, runLon);
                tdw->setTime(dt1);
                tdw->setMetadata(DataIndexer::self()->index("stop"), (text1+' '+text2));

                mResultPoints.append(tdw);

                startIndex = currentIndex;		// start search again after stop
            }
            else kDebug() << "time too short";
        }
        else kDebug() << "not enough points";

        ++startIndex;					// start search again from next
    } // loop2

    const int num = mResultPoints.count();
    kDebug() << "###### found" << num << "stops";






    mResultsList->blockSignals(true);

    while (mResultsList->count()>0)			// clear existing results
    {
        QListWidgetItem *item = mResultsList->takeItem(0);
        delete item;
    }

    for (int i = 0; i<num; ++i)				// generate new results
    {
        const TrackDataWaypoint *tdw = mResultPoints[i];

        QListWidgetItem *item = new QListWidgetItem(tdw->name());
        item->setFlags(item->flags()|Qt::ItemIsUserCheckable);
        item->setData(Qt::CheckStateRole, Qt::Checked);
        item->setData(Qt::UserRole, i);
        mResultsList->addItem(item);
    }

    slotSetButtonStates();
    mResultsList->blockSignals(false);

    mainWindow()->mapController()->view()->setStopLayerData(&mResultPoints);
    unsetCursor();

    kDebug() << "done";
}


void StopDetectDialogue::slotSetButtonStates()
{
    int numChecked = 0;
    for (int i = 0; i<mResultsList->count(); ++i)
    {
        const QListWidgetItem *item = mResultsList->item(i);
        Qt::CheckState check = static_cast<Qt::CheckState>(item->data(Qt::CheckStateRole).toInt());
        if (check==Qt::Checked) ++numChecked;
    }

    enableButtonOk(numChecked>0 && !mFolderSelect->folderPath().isEmpty());
    mShowOnMapButton->setEnabled(mResultsList->selectedItems().count()==1);
}


void StopDetectDialogue::slotCommitResults()
{
    kDebug();

    QUndoCommand *cmd = new QUndoCommand();		// parent command
    cmd->setText(i18n("Locate Stops"));

    const QString folderPath = mFolderSelect->folderPath();
    Q_ASSERT(!folderPath.isEmpty());
    const QStringList folders = folderPath.split('/');
    Q_ASSERT(!folders.isEmpty());

    FilesModel *model = mainWindow()->filesController()->model();

    // The destination folder must exist at this point
    // TODO: -> TrackDataFolder *TrackData::findFolderByPath()
    // also used in FolderSelectDialogue::setDestinationPath()
    TrackDataItem *destFolder = model->rootFileItem();
    foreach (const QString &folder, folders)
    {
        destFolder = TrackData::findChildFolder(folder, destFolder);
        Q_ASSERT(destFolder!=NULL);
    }

    // Create the waypoints
    for (int i = 0; i<mResultsList->count(); ++i)
    {
        const QListWidgetItem *item = mResultsList->item(i);
        Qt::CheckState check = static_cast<Qt::CheckState>(item->data(Qt::CheckStateRole).toInt());
        if (check!=Qt::Checked) continue;		// include this in results?

        const TrackDataWaypoint *tdw = mResultPoints[i];
							// source point - its metadata copied
        AddWaypointCommand *cmd2 = new AddWaypointCommand(mainWindow()->filesController(), cmd);
        cmd2->setData(tdw->name(), tdw->latitude(), tdw->longitude(),
                      dynamic_cast<TrackDataFolder *>(destFolder),
                      tdw);
    }

    if (cmd->childCount()==0)				// anything to actually do?
    {							// nothing added above, so
        delete cmd;					// don't need this after all
        return;
    }

    mainWindow()->executeCommand(cmd);
}
