
#include "positioninfodialogue.h"

#include <qgridlayout.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qdebug.h>

#include <klocalizedstring.h>

#include <kfdialog/dialogstatewatcher.h>

#include "mapview.h"
#include "elevationmanager.h"
#include "elevationtile.h"
#include "trackdata.h"
#include "variableunitdisplay.h"

#include <marble/ReverseGeocodingRunnerManager.h>


PositionInfoDialogue::PositionInfoDialogue(int posX, int posY, QWidget *pnt)
    : DialogBase(pnt)
{
    setObjectName("PositionInfoDialogue");

    setModal(true);
    setButtons(QDialogButtonBox::Close);
    setWindowTitle(i18n("Position Information"));

    QWidget *w = new QWidget(this);
    QGridLayout *gl = new QGridLayout(w);
    setMainWidget(w);
    stateWatcher()->setSaveOnButton(buttonBox()->button(QDialogButtonBox::Close));

    MapView *view = qobject_cast<MapView *>(pnt);
    Q_ASSERT(view!=nullptr);

    // Position
    QLabel *l = new QLabel(i18nc("@title:row", "Position:"), this);
    gl->addWidget(l, 0, 0, Qt::AlignTop);

    mLatitude = 0.0;
    mLongitude = 0.0;
    const bool valid = view->geoCoordinates(posX, posY, mLongitude, mLatitude, GeoDataCoordinates::Degree);

    QLabel *positionDisplay = new QLabel(i18n("<html><font %1>%2</font></html>",
                                              (valid ? "" : "color=\"red\""),
                                              TrackData::formattedLatLong(mLatitude, mLongitude)), this);
    positionDisplay->setTextInteractionFlags(Qt::TextSelectableByMouse|Qt::TextSelectableByKeyboard);
    gl->addWidget(positionDisplay, 0, 2, Qt::AlignTop);
    l->setBuddy(positionDisplay);

    if (valid)						// have a valid Earth position
    {
        // Elevation
        l = new QLabel(i18nc("@title:row", "Elevation:"), this);
        gl->addWidget(l, 1, 0, Qt::AlignVCenter);

        mElevationDisplay = new VariableUnitDisplay(VariableUnitCombo::Elevation, this);
        mElevationDisplay->setEnabled(false);
        gl->addWidget(mElevationDisplay, 1, 2, Qt::AlignVCenter);
        l->setBuddy(mElevationDisplay);

        connect(ElevationManager::self(), &ElevationManager::tileReady,
                this, &PositionInfoDialogue::slotShowElevation, Qt::QueuedConnection);
        ElevationManager::self()->requestTile(mLatitude, mLongitude);

        // Address
        l = new QLabel(i18nc("@title:row", "Address:"), this);
        gl->addWidget(l, 3, 0, Qt::AlignTop);

        mAddressLabel = new QLabel(i18n("(Awaiting address)"), this);
        mAddressLabel->setEnabled(false);
        mAddressLabel->setTextInteractionFlags(Qt::TextSelectableByMouse|Qt::TextSelectableByKeyboard);
        gl->addWidget(mAddressLabel, 3, 2, Qt::AlignTop);
        l->setBuddy(mAddressLabel);

        // from MarbleWidgetPopupMenu::startReverseGeocoding()
        auto *runnerManager = new ReverseGeocodingRunnerManager(view->model(), this);
        // The most verbose connect cast so far...
        connect(runnerManager, static_cast<void (ReverseGeocodingRunnerManager::*)(const GeoDataCoordinates &, const GeoDataPlacemark &)>
                                          (&ReverseGeocodingRunnerManager::reverseGeocodingFinished),
                this, &PositionInfoDialogue::slotShowAddressInformation);

        GeoDataCoordinates coords(mLongitude, mLatitude, 0, GeoDataCoordinates::Degree);
        runnerManager->reverseGeocoding(coords);
    }

    gl->setColumnMinimumWidth(1, DialogBase::horizontalSpacing());
    gl->setRowMinimumHeight(2, DialogBase::verticalSpacing());
    gl->setColumnStretch(2, 1);
    gl->setRowStretch(4, 1);
}


void PositionInfoDialogue::slotShowAddressInformation(const GeoDataCoordinates &coords,
                                                      const GeoDataPlacemark &placemark)
{
    qDebug();
    // from MarbleWidgetPopupMenu::showAddressInformation()
    QString addr = placemark.address();
    addr.replace(", ", "\n");
    mAddressLabel->setText(addr);
    mAddressLabel->setEnabled(true);
}


void PositionInfoDialogue::slotShowElevation(const ElevationTile *tile)
{
    qDebug();
    if (!tile->isValidFor(mLatitude, mLongitude)) return;
    mElevationDisplay->setValue(tile->elevation(mLatitude, mLongitude));
    mElevationDisplay->setEnabled(true);
}
