
#include "positioninfodialogue.h"

#include <qgridlayout.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qdebug.h>

#include <klocalizedstring.h>

#include <dialogstatewatcher.h>

#include "mapview.h"
#include "elevationmanager.h"
#include "elevationtile.h"
#include "trackdata.h"

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
    Q_ASSERT(view!=NULL);

    // Position
    QLabel *l = new QLabel(i18nc("@title:row", "Position:"), this);
    gl->addWidget(l, 0, 0, Qt::AlignTop);

    mLatitude = 0.0;
    mLongitude = 0.0;
    const bool valid = view->geoCoordinates(posX, posY, mLongitude, mLatitude, GeoDataCoordinates::Degree);

    l = new QLabel(i18n("<html><font %1>%2</font></html>",
                        (valid ? "" : "color=\"red\""),
                        TrackData::formattedLatLong(mLatitude, mLongitude)), this);
    gl->addWidget(l, 0, 2, Qt::AlignTop);

    if (valid)
    {
        // Elevation
        l = new QLabel(i18nc("@title:row", "Elevation:"), this);
        gl->addWidget(l, 1, 0, Qt::AlignTop);

        mElevationLabel = new QLabel(i18n("(Awaiting elevation)"), this);
        gl->addWidget(mElevationLabel, 1, 2, Qt::AlignTop);

        connect(ElevationManager::self(), &ElevationManager::tileReady,
                this, &PositionInfoDialogue::slotShowElevation, Qt::QueuedConnection);
        mRequestedTile = ElevationManager::self()->requestTile(mLatitude, mLongitude);

        // Address
        l = new QLabel(i18nc("@title:row", "Address:"), this);
        gl->addWidget(l, 2, 0, Qt::AlignTop);

        mAddressLabel = new QLabel(i18n("(Awaiting address)"), this);
        gl->addWidget(mAddressLabel, 2, 2, Qt::AlignTop);

        // from MarbleWidgetPopupMenu::startReverseGeocoding()
        auto *runnerManager = new ReverseGeocodingRunnerManager(view->model(), this);
        // The most verbose connect cast ever...
        connect(runnerManager, static_cast<void (ReverseGeocodingRunnerManager::*)(const GeoDataCoordinates &, const GeoDataPlacemark &)>
                                          (&ReverseGeocodingRunnerManager::reverseGeocodingFinished),
                this, &PositionInfoDialogue::slotShowAddressInformation);

        GeoDataCoordinates coords(mLongitude, mLatitude, 0, GeoDataCoordinates::Degree);
        runnerManager->reverseGeocoding(coords);
    }

    gl->setColumnMinimumWidth(2, DialogBase::horizontalSpacing());
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
}


void PositionInfoDialogue::slotShowElevation(const ElevationTile *tile)
{
    qDebug();
    if (tile!=mRequestedTile) return;
    mElevationLabel->setText(QString::number(tile->elevation(mLatitude, mLongitude)));
}
