
#include "trackpropertiesplotpages.h"

#include <qformlayout.h>
#include <qcheckbox.h>
#include <qspinbox.h>
#include <qdebug.h>

#include <klocalizedstring.h>

#include "trackdata.h"


TrackItemPlotPage::TrackItemPlotPage(const QList<TrackDataItem *> *items, QWidget *pnt)
    : TrackPropertiesPage(items, pnt)
{
    qDebug();
    setObjectName("TrackItemPlotPage");

    const TrackDataItem *item = items->first();
    // TODO: maybe for routepoints too
    const TrackDataWaypoint *tdw = dynamic_cast<const TrackDataWaypoint *>(item);
    Q_ASSERT(tdw!=nullptr);				// only applicable to waypoints

    QString brgVal = item->metadata("bearingline");
    qDebug() << "brg value" << brgVal;

    mBearingLineCheck = new QCheckBox(i18n("Show bearing line"), this);
    mBearingLineCheck->setChecked(!brgVal.isEmpty() && !brgVal.startsWith('!'));

    connect(mBearingLineCheck, SIGNAL(toggled(bool)), SLOT(slotUpdateButtons()));
    addSeparatorField();
    mFormLayout->addRow(QString::null, mBearingLineCheck);

    mBearingEntry = new QSpinBox(this);
    mBearingEntry->setMinimum(0);
    mBearingEntry->setMaximum(359);
    mBearingEntry->setWrapping(true);
    mBearingEntry->setSuffix(i18nc("suffix for degrees", " degrees"));
    mBearingEntry->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);

    if (brgVal.startsWith('!')) brgVal.remove(0, 1);
    if (!brgVal.isEmpty()) mBearingEntry->setValue(brgVal.toInt());

    connect(mBearingEntry, SIGNAL(valueChanged(int)), SLOT(slotDataChanged()));
    mFormLayout->addRow(i18nc("@label:spinbox", "Bearing line:"), mBearingEntry);

    slotUpdateButtons();
}


void TrackItemPlotPage::slotUpdateButtons()
{
    mBearingEntry->setEnabled(mBearingLineCheck->isChecked());
}


QString TrackItemPlotPage::newBearingLine() const
{
    if (mBearingEntry==nullptr) return ("-");		// not for this data

    const bool set = mBearingEntry->isEnabled();	// is the option set?
    QString val = mBearingEntry->cleanText();		// the value entered

    if (!set && val.isEmpty()) return ("-");		// not applicable
    return (!set ? ('!'+val) : val);			// enable state and value
}


TrackWaypointPlotPage::TrackWaypointPlotPage(const QList<TrackDataItem *> *items, QWidget *pnt)
    : TrackItemPlotPage(items, pnt)
{
    qDebug();
    setObjectName("TrackWaypointPlotPage");
}


CREATE_PROPERTIES_PAGE(Waypoint, Plot);

NULL_PROPERTIES_PAGE(File, Plot);
NULL_PROPERTIES_PAGE(Track, Plot);
NULL_PROPERTIES_PAGE(Route, Plot);
NULL_PROPERTIES_PAGE(Segment, Plot);
NULL_PROPERTIES_PAGE(Trackpoint, Plot);
NULL_PROPERTIES_PAGE(Folder, Plot);
NULL_PROPERTIES_PAGE(Routepoint, Plot);
