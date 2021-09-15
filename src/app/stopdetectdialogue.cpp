
#include "stopdetectdialogue.h"

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
#include <qdebug.h>
#include <qtimezone.h>
#include <qaction.h>

#include <klocalizedstring.h>
#include <kseparator.h>
#include <kmessagebox.h>
#include <kactioncollection.h>
#include <kxmlguiwindow.h>

#include <kfdialog/dialogstatewatcher.h>

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
#include "units.h"



static const int minInputPoints = 10;			// minimum points for detection
static const int minStopPoints = 3;			// minimum points for valid stop

static const int mergeMaxDistance = 200;		// ask about merge this distance (metres)
static const int mergeMaxTime = 3*60;			// ask about merge this time gap (seconds)


// TODO: debugging switches


// TODO: explain algorithms here

#define MOVING_AVERAGING				// use "Moving Averaging" algorithm





StopDetectDialogue::StopDetectDialogue(QWidget *pnt)
    : DialogBase(pnt),
      ApplicationDataInterface(pnt)
{
    setObjectName("StopDetectDialogue");

    setAttribute(Qt::WA_DeleteOnClose);
    setModal(false);

    setWindowTitle(i18nc("@title:window", "Locate Stops"));
    setButtons(QDialogButtonBox::Ok|QDialogButtonBox::Close);
    setButtonEnabled(QDialogButtonBox::Ok, false);
    setButtonText(QDialogButtonBox::Ok, i18nc("@action:button", "Commit"));

    filesController()->view()->selectedPoints().swap(mInputPoints);

    mTimeZone = QTimeZone::utc();			// a sensible default
    QString zoneName = filesController()->model()->rootFileItem()->metadata("timezone").toString();
    if (!zoneName.isEmpty())				// resolve from file time zone
    {
        QTimeZone tz(zoneName.toLatin1());
        if (tz.isValid()) mTimeZone = tz;
    }

    mIdleTimer = new QTimer(this);
    mIdleTimer->setInterval(2000);
    mIdleTimer->setSingleShot(true);			// don't start until show event
    connect(mIdleTimer, SIGNAL(timeout()), SLOT(slotDetectStops()));

    QWidget *w = new QWidget(this);
    QHBoxLayout *hb = new QHBoxLayout(w);

    // Left hand side: parameters
    QFormLayout *fl = new QFormLayout;
    hb->addLayout(fl);

    QLabel *l = new QLabel("", w);
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

    fl->addRow(new QLabel("", this));

    mFolderSelect = new FolderSelectWidget(this);
    mFolderSelect->setToolTip(i18n("The folder where the located stops will be saved"));
    const QString folderName = i18nc("Name of the default folder for stops", "Stops");
    const bool folderExists = (TrackData::findFolderByPath(folderName, filesController()->model()->rootFileItem())!=nullptr);
							// either existing or placeholder
    mFolderSelect->setFolderPath(folderName, !folderExists);
    connect(mFolderSelect, SIGNAL(folderChanged(const QString &)), SLOT(slotSetButtonStates()));
    fl->addRow(i18n("Destination folder:"), mFolderSelect);

    // Middle: separator
    KSeparator *sep = new KSeparator(Qt::Vertical, w);
    hb->addSpacing(DialogBase::horizontalSpacing());
    hb->addWidget(sep);
    hb->addSpacing(DialogBase::horizontalSpacing());

    // Right hand side: results
    QGridLayout *gl = new QGridLayout;

    l = new QLabel(i18n("Stops found:"), w);
    gl->addWidget(l, 0, 0, 1, -1, Qt::AlignLeft);

    mResultsList = new QListWidget(w);
    mResultsList->setAlternatingRowColors(true);
    mResultsList->setSelectionBehavior(QAbstractItemView::SelectRows);
    mResultsList->setSelectionMode(QAbstractItemView::ContiguousSelection);
    mResultsList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    mResultsList->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    connect(mResultsList, SIGNAL(itemSelectionChanged()), SLOT(slotSetButtonStates()));
    connect(mResultsList, SIGNAL(itemChanged(QListWidgetItem *)), SLOT(slotSetButtonStates()));

    gl->addWidget(mResultsList, 1, 0, 1, -1);
    gl->setRowStretch(1, 1);
    gl->setColumnStretch(1, 1);
    l->setBuddy(mResultsList);

    // Qualifying mainWidget() is necessary because of multiple inheritance:
    //
    // app/stopdetectdialogue.cpp:153: error: reference to 'mainWidget' is ambiguous
    // core/applicationdatainterface.h:48: note: candidates are:
    //                         'QWidget* ApplicationDataInterface::mainWidget() const'
    // kfdialog/dialogbase.h:68: note: 'QWidget* DialogBase::mainWidget() const'
    KXmlGuiWindow *mainwin = qobject_cast<KXmlGuiWindow *>(ApplicationDataInterface::mainWidget());

    QAction *act = mainwin->actionCollection()->action("map_go_selection");
    Q_ASSERT(act!=nullptr);

    mShowOnMapButton = new QPushButton(act->icon(), act->text(), w);
    mShowOnMapButton->setShortcut(act->shortcut());
    mShowOnMapButton->setToolTip(i18n("Show the selected stop point on the map"));
    connect(mShowOnMapButton, SIGNAL(clicked(bool)), SLOT(slotShowOnMap()));
    gl->addWidget(mShowOnMapButton, 2, 2, Qt::AlignRight);

    act = mainwin->actionCollection()->action("track_merge");
    Q_ASSERT(act!=nullptr);

    mMergeStopsButton = new QPushButton(act->icon(), i18n("Merge Stops"), w);
    mMergeStopsButton->setShortcut(act->shortcut());
    mMergeStopsButton->setToolTip(i18n("Merge the selected stop points into one"));
    connect(mMergeStopsButton, SIGNAL(clicked(bool)), SLOT(slotMergeStops()));
    gl->addWidget(mMergeStopsButton, 2, 0, Qt::AlignLeft);

    connect(this, SIGNAL(accepted()), SLOT(slotCommitResults()));

    hb->addLayout(gl);
    setMainWidget(w);
    stateWatcher()->setSaveOnButton(buttonBox()->button(QDialogButtonBox::Close));
    setMinimumSize(460, 280);
    slotSetButtonStates();
}


StopDetectDialogue::~StopDetectDialogue()
{
    mapController()->view()->setStopLayerData(nullptr);
    qDeleteAll(mResultPoints);
    qDebug() << "done";
}


void StopDetectDialogue::showEvent(QShowEvent *ev)
{
    qDebug();
    DialogBase::showEvent(ev);
    QTimer::singleShot(0, this, SLOT(slotDetectStops()));
}


void StopDetectDialogue::slotShowOnMap()
{
    QList<QListWidgetItem *> items = mResultsList->selectedItems();
    if (items.count()!=1) return;
    const int idx = items.first()->data(Qt::UserRole).toInt();

    TrackDataWaypoint *tdw = const_cast<TrackDataWaypoint *>(mResultPoints[idx]);

    qDebug() << "index" << idx << tdw->name();

    QList<TrackDataItem *> its;
    its.append(static_cast<TrackDataItem *>(tdw));
    mapController()->gotoSelection(its);
}


static bool withinDistance(const TrackDataAbstractPoint *tdp, double lat, double lon, int maxDist)
{
    double distance = qAbs(Units::internalToLength(tdp->distanceTo(lat, lon), Units::LengthMetres));
    return (distance<=double(maxDist));			// convert to metres and check
}


static void setStopData(TrackDataWaypoint *tdw, const QDateTime &dt, int dur)
{
    QString text1 = dt.toString("hh:mm:ss");
    QString text2 = QString("%1:%2").arg(dur/60).arg(dur%60, 2, 10, QLatin1Char('0'));

    tdw->setName(i18n("Stop at %1 for %2", text1, text2), true);
    tdw->setMetadata("time", dt);
    tdw->setMetadata("duration", QVariant(dur));
    tdw->setMetadata("stop", QString(text1+' '+text2));
}


void StopDetectDialogue::slotDetectStops()
{
    qDebug() << "starting";

    const int minTime = mTimeSlider->value();
    const int maxDist = mDistanceSlider->value();
    const int noise = mNoiseSlider->value();
    qDebug() << "minTime" << minTime << "maxDist" << maxDist << "noise" << noise;

    setCursor(Qt::BusyCursor);

    mapController()->view()->setStopLayerData(nullptr);

    // Clear the result array but do not delete the points it contains,
    // previously committed points are now part of the main data tree.
    mResultPoints.clear();

    // Assemble all of the points of interest into a single linear list.
    // This list is assumed to be in time order.
    QVector<const TrackDataTrackpoint *> inputPoints;

    qDebug() << "have" << mInputPoints.count() << "input points";
    if (mInputPoints.count()<minInputPoints)
    {
        qDebug() << "not enough points!";
        unsetCursor();
        return;
    }

// may need sorting for time here

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
    const TrackDataAbstractPoint *pnt;

    for (;;)						// loop 2: for each stop found
    {
        if (startIndex>=mInputPoints.count()) break;	// reached end of points list

        qDebug() << "### loop2: startIndex" << startIndex;

        pnt = mInputPoints.at(startIndex);		// initial point in run
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
            qDebug() << "### loop1: currentIndex" << currentIndex;

            int foundIndex = -1;			// index of found point
            for (int i = 0; i<=noise; ++i)
            {
                int searchIndex = currentIndex+1+i;	// index of point being checked
                qDebug() << "  looking at searchIndex" << searchIndex;

                if (searchIndex>=mInputPoints.count()) break;
							// end of points list
                pnt = mInputPoints.at(searchIndex);
                if (withinDistance(pnt, runLat, runLon, maxDist))
                {					// within distance tolerance?
                    qDebug() << "  within distance tol";
                    foundIndex = searchIndex;
                    break;
                }
                else qDebug() << "  not within tol";
            }

            qDebug() << "### foundIndex" << foundIndex;

            if (foundIndex<0) break; // loop1

            // startIndex = index of start of this run
            // foundIndex = index of end of this run

            qDebug() << "run found from startIndex" << startIndex << "to foundIndex" << foundIndex;
            pnt = mInputPoints.at(startIndex);
            qDebug() << "  start point" << pnt->name() << "found point" << mInputPoints.at(foundIndex)->name();

#ifdef MOVING_AVERAGING
            pnt = mInputPoints.at(foundIndex);
            sumLat += pnt->latitude();
            sumLon += pnt->longitude();
            ++sumNum;

            qDebug() << "average now of" << sumNum << "points";
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
                 pnt = mInputPoints.at(i);
                 if (withinDistance(pnt, refLat, refLon, maxDist))
                 {
                     qDebug() << "  including" << i << "in average";
                     lat += pnt->latitude();
                     lon += pnt->longitude();
                     ++num;
                 }
             }
 
             qDebug() << "average of" << num << "points";
             runLat = lat/num;
             runLon = lon/num;
#endif // MOVING_AVERAGING

            currentIndex = foundIndex;			// continue search at next point
        } // loop1

        // startIndex = index of start of this run
        // currentIndex = index of end of the run

        if ((currentIndex-startIndex)>=minStopPoints)
        {
            qDebug() << "******** stop found from startIndex" << startIndex << "to currentIndex" << currentIndex;

            const TrackDataAbstractPoint *startPoint = mInputPoints.at(startIndex);
            const TrackDataAbstractPoint *endPoint = mInputPoints.at(currentIndex);

            qDebug() << "  start point" << startPoint->name() << "end point" << endPoint->name();

            QDateTime dt1(startPoint->time());
            QDateTime dt2(endPoint->time());
            const int dur = dt1.secsTo(dt2);
            qDebug() << "  duration" << dur;

            if (dur>=minTime)				// longer than threshold?
            {
                qDebug() << "@@@@@@@@@@@ valid stop found";
							// do time zone conversion
                dt1 = dt1.toUTC().toTimeZone(mTimeZone);

                TrackDataWaypoint *tdw = new TrackDataWaypoint;
                setStopData(tdw, dt1, dur);
                tdw->setLatLong(runLat, runLon);	// want explicit name here
                mResultPoints.append(tdw);

                startIndex = currentIndex;		// start search again after stop
            }
            else qDebug() << "time too short";
        }
        else qDebug() << "not enough points";

        ++startIndex;					// start search again from next
    } // loop2

    updateResults();
    unsetCursor();

    qDebug() << "done";
}


void StopDetectDialogue::slotMergeStops()
{
    QList<QListWidgetItem *> items = mResultsList->selectedItems();
    const int num = items.count();
    qDebug() << num << "points";
    if (num<2) return;

    const int idx1 = items.first()->data(Qt::UserRole).toInt();
    const int idx2 = items.last()->data(Qt::UserRole).toInt();

    TrackDataWaypoint *firstPoint = const_cast<TrackDataWaypoint *>(mResultPoints[idx1]);
    qDebug() << "first point" << firstPoint->name();	// data for first point

    double avgLat = firstPoint->latitude();
    double avgLon = firstPoint->longitude();
    qint64 startTime = firstPoint->metadata("time").toDateTime().toSecsSinceEpoch();
    qint64 endTime = startTime+firstPoint->metadata("duration").toInt();

    for (int i = idx1+1; i<=idx2; ++i)
    {							// data for current point
        const TrackDataWaypoint *thisPoint = mResultPoints[i];
        qDebug() << "+ next point" << thisPoint->name();

        double thisLat = thisPoint->latitude();
        double thisLon = thisPoint->longitude();
        qint64 thisStartTime = thisPoint->metadata("time").toDateTime().toSecsSinceEpoch();
        qint64 thisEndTime = thisStartTime+thisPoint->metadata("duration").toInt();

        // Check that the stops to be merged are not too far apart in distance.
        if (!withinDistance(firstPoint, thisLat, thisLon, mergeMaxDistance))
        {
            if (KMessageBox::questionYesNo(this,
                                           xi18nc("@info", "The stops <emphasis strong=\"1\">%1</emphasis> and <emphasis strong=\"1\">%2</emphasis><nl/> are %3&nbsp;metres apart.<nl/><nl/>Really merge the stops?",
                                                  firstPoint->name(), thisPoint->name(),
                                                  qRound(Units::internalToLength(firstPoint->distanceTo(thisPoint), Units::LengthMetres))),
                                           i18n("Merge Distance"),
                                           KGuiItem(i18n("Merge"), QIcon::fromTheme("merge")),
                                           KStandardGuiItem::cancel())!=KMessageBox::Yes) return;
        }

        // Check that there is not too long a gap between the stops to be merged.
        if ((thisStartTime-endTime)>mergeMaxTime)
        {
            if (KMessageBox::questionYesNo(this,
                                           xi18nc("@info", "The stops <emphasis strong=\"1\">%1</emphasis> and <emphasis strong=\"1\">%2</emphasis><nl/> are separated by %3&nbsp;seconds.<nl/><nl/>Really merge the stops?",
                                                  firstPoint->name(), thisPoint->name(),
                                                  thisStartTime-endTime),
                                           i18n("Merge Time"),
                                           KGuiItem(i18n("Merge"), QIcon::fromTheme("merge")),
                                           KStandardGuiItem::cancel())!=KMessageBox::Yes) return;
        }

        avgLat += thisLat;				// sum for averaging later
        avgLon += thisLon;
        endTime = thisEndTime;				// end time of this point
    }

    // Update the first point item with the results, and delete all the others.
    // The start time of the merged point is the same as the start time of the
    // original first point.
    const int dur = endTime-startTime;
    setStopData(firstPoint, firstPoint->metadata("time").toDateTime(), dur);
    firstPoint->setLatLong((avgLat/num), (avgLon/num));

    // Remove the other points that were merged into the first one.
    for (int i = idx2; i>=idx1+1; --i)
    {
        const TrackDataWaypoint *tdw = mResultPoints.takeAt(i);
        delete tdw;
    }

    updateResults();
    // Select the merged point in the results list.
    QListWidgetItem *resultItem = mResultsList->item(idx1);
    mResultsList->setCurrentItem(resultItem);
    mResultsList->scrollToItem(resultItem);
}


void StopDetectDialogue::updateResults()
{
    const int num = mResultPoints.count();
    qDebug() << "have" << num << "stops";

    mapController()->view()->setStopLayerData(nullptr);	// clear any existing stops overlay

    QSignalBlocker block(mResultsList);			// block slotSetButtonStates() each time
    while (mResultsList->count()>0)			// clear existing results items
    {
        QListWidgetItem *item = mResultsList->takeItem(0);
        delete item;
    }

    for (int i = 0; i<num; ++i)				// generate items for new results
    {
        const TrackDataWaypoint *tdw = mResultPoints[i];

        QListWidgetItem *item = new QListWidgetItem(tdw->name());
        item->setFlags(item->flags()|Qt::ItemIsUserCheckable);
        item->setData(Qt::CheckStateRole, Qt::Checked);
        item->setData(Qt::UserRole, i);
        mResultsList->addItem(item);
    }

    slotSetButtonStates();				// now that all is finished
    mapController()->view()->setStopLayerData(&mResultPoints);
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

    setButtonEnabled(QDialogButtonBox::Ok, numChecked>0 && !mFolderSelect->folderPath().isEmpty() && !isReadOnly());
    mShowOnMapButton->setEnabled(mResultsList->selectedItems().count()==1);
    mMergeStopsButton->setEnabled(mResultsList->selectedItems().count()>1);
}


void StopDetectDialogue::slotCommitResults()
{
    qDebug();

    QUndoCommand *cmd = new QUndoCommand();		// parent command
    cmd->setText(i18n("Locate Stops"));

    const QString folderPath = mFolderSelect->folderPath();
    Q_ASSERT(!folderPath.isEmpty());

    TrackDataItem *root = filesController()->model()->rootFileItem();
    // The destination folder may not exist at this point,
    // if the default entry has been accepted.
    TrackDataFolder *destFolder = TrackData::findFolderByPath(folderPath, root);
    if (destFolder==nullptr)				// does not yet exist
    {
        AddContainerCommand *cmd1 = new AddContainerCommand(filesController());

        // This assumes that the folder to be created is at the top level.
        // It is safe to assume this, because the FolderSelectWidget line edit
        // is read only so it is not possible to enter an arbitrary string.
        // The folderPath will be either the default "Stops", or another which
        // will have been selected via the FolderSelectDialogue (which must
        // already exist or have been created).  So it is not possible to
        // enter an arbitrary folder path which does not yet exist.
        cmd1->setData(TrackData::Folder, root);
        cmd1->setName(folderPath);
        cmd1->setText(i18n("Create Stops Folder"));

        // This will end up with two operations in the undo history,
        // but unfortunately it is necessary because we need to
        // specify the destination folder for adding the waypoints.
        executeCommand(cmd1);
        destFolder = TrackData::findFolderByPath(folderPath, root);
        Q_ASSERT(destFolder!=nullptr);			// should now have been created
    }

    // Create the waypoints
    for (int i = 0; i<mResultsList->count(); ++i)
    {
        const QListWidgetItem *item = mResultsList->item(i);
        Qt::CheckState check = static_cast<Qt::CheckState>(item->data(Qt::CheckStateRole).toInt());
        if (check!=Qt::Checked) continue;		// include this in results?

        const TrackDataWaypoint *tdw = mResultPoints[i];
							// source point - its metadata copied
        AddWaypointCommand *cmd2 = new AddWaypointCommand(filesController(), cmd);
        cmd2->setData(tdw->name(), tdw->latitude(), tdw->longitude(),
                      dynamic_cast<TrackDataFolder *>(destFolder), tdw);
    }

    if (cmd->childCount()==0)				// anything to actually do?
    {							// nothing added above, so
        delete cmd;					// don't need this after all
        return;
    }

    executeCommand(cmd);
}
