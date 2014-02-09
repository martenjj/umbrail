
#include "mapcontroller.h"

#include <kdebug.h>
#include <klocale.h>
#include <kconfiggroup.h>
#include <kfiledialog.h>
#include <kimageio.h>
#include <kmessagebox.h>

#include <marble/MarbleAboutDialog.h>
#include <marble/GeoDataLatLonBox.h>

#include "mapview.h"
#include "trackdata.h"
#include "mapthemedialogue.h"
#include "mainwindow.h"



#define GROUP_MAP		"Map"
#define CONFIG_HOME		"Home"
#define CONFIG_CURRENT		"Current"
#define CONFIG_OVERLAYS		"Overlays"
#define CONFIG_THEME		"Theme"



MapController::MapController(QObject *pnt)
    : QObject(pnt)
{
    kDebug();

    mView = new MapView(mainWindow());
    connect(mView, SIGNAL(mouseMoveGeoPosition(const QString &)),
            SLOT(slotShowPosition(const QString &)));
    connect(mView, SIGNAL(zoomChanged(int)), SLOT(slotZoomChanged(int)));

    mHomeLat = 51.436019;				// default to here
    mHomeLong = -0.352764;
    mHomeZoom = 2300;

    mThemeManager = NULL;				// created on demand
}


MapController::~MapController()
{
    kDebug() << "done";
}


void MapController::clear()
{
    kDebug();
}


void MapController::readProperties()
{
    view()->readProperties();
}

void MapController::saveProperties()
{
    view()->saveProperties();
}


QString MapController::save(KConfig *conf)
{
    kDebug() << "saving to" << conf->name();

    KConfigGroup grp = conf->group(GROUP_MAP);

    QList<double> list;
    list << view()->centerLatitude() << view()->centerLongitude() << view()->zoom();
    kDebug() << "  current" << list;
    grp.writeEntry(CONFIG_CURRENT, list);

    list.clear();
    list << mHomeLat << mHomeLong << mHomeZoom;
    kDebug() << "  home" << list;
    grp.writeEntry(CONFIG_HOME, list);

// TODO: crosshairs etc?
    grp.writeEntry(CONFIG_THEME, view()->mapThemeId());
    grp.writeEntry(CONFIG_OVERLAYS, view()->overlays(true));

    return (QString::null);
}


QString MapController::load(const KConfig *conf)
{
    kDebug() << "reading from" << conf->name();
    KConfigGroup grp = conf->group(GROUP_MAP);

    bool haveHome = false;
    QList<double> list = grp.readEntry(CONFIG_HOME, QList<double>());
    kDebug() << "  home" << list;
    if (list.count()==3)
    {
        mHomeLat = list[0];
        mHomeLong = list[1];
        mHomeZoom = list[2];
        haveHome = true;
    }

    list = grp.readEntry(CONFIG_CURRENT, QList<double>());
    kDebug() << "  current" << list;
    if (list.count()==3)				// have a current setting
    {
        view()->zoomView(list[2]);
        view()->centerOn(list[1], list[0]);
    }
    else if (haveHome)					// not got that, go home
    {							// if have a home setting
        slotGoHome();
    }

// TODO: crosshairs etc?
    QString themeId = grp.readEntry(CONFIG_THEME, "");
    if (!themeId.isEmpty()) view()->setMapThemeId(themeId);
    view()->showOverlays(grp.readEntry(CONFIG_OVERLAYS, QStringList()));

    return (QString::null);
}



void MapController::slotGoHome()
{
    kDebug() << "lat" << mHomeLat << "long" << mHomeLong << "zoom" << mHomeZoom;
    view()->zoomView(mHomeZoom);
    view()->centerOn(mHomeLong, mHomeLat);

//    emit statusMessage(i18n("At home position %1",
//                            PointData::displayLatLong(mHomeLat, mHomeLong, true)));
}


void MapController::slotSetHome()
{
    double lat = view()->centerLatitude();
    double lng = view()->centerLongitude();

    if (KMessageBox::questionYesNo(mainWindow(),
                                   i18n("<qt>Set home position to <b>%1</b>?", 
//                                        PointData::displayLatLong(lat, lng, true)
QString::null
),
                                   i18n("Set Home Position?"),
                                   KGuiItem(i18n("Set"), KStandardGuiItem::yes().icon()),
                                   KStandardGuiItem::cancel(),
                                   "setHome")!=KMessageBox::Yes) return;
    mHomeLat = lat;
    mHomeLong = lng;
    // TODO: do this here?
    //mHomeZoom = view()->zoom();

//    emit statusMessage(i18n("Home position set to %1",
//                            PointData::displayLatLong(mHomeLat, mHomeLong, true)));
    emit modified();
}


void MapController::slotSetZoom()
{
    int zoom = view()->zoom();

    if (KMessageBox::questionYesNo(mainWindow(),
                                   i18n("<qt>Set standard zoom to <b>%1</b>?", zoom),
                                   i18n("Set Standard Zoom?"),
                                   KGuiItem(i18n("Set"), KStandardGuiItem::yes().icon()),
                                   KStandardGuiItem::cancel(),
                                   "setZoom")!=KMessageBox::Yes) return;
    mHomeZoom = zoom;
    emit statusMessage(i18n("Standard zoom set to %1", mHomeZoom));
    emit modified();
}


void MapController::slotResetZoom()
{
    kDebug() << "zoom" << mHomeZoom;
    view()->zoomView(mHomeZoom);
    emit statusMessage(i18n("At standard zoom %1", mHomeZoom));
}




void MapController::slotSaveImage()
{
    const QStringList mimetypes = KImageIO::mimeTypes(KImageIO::Writing);
    QString saveFile = KFileDialog::getSaveFileName(KUrl("kfiledialog:///mapsave"),
                                                    mimetypes.join(" "),
                                                    mainWindow(),
                                                    i18n("Save Map as Image"),
                                                    KFileDialog::ConfirmOverwrite);
    if (saveFile.isEmpty()) return;

    QStringList currentOverlays = view()->overlays(true);
    view()->showOverlays(QStringList());

    emit statusMessage(i18n("Saving map image...", saveFile));
    QPixmap pix = view()->mapScreenShot();
    kDebug() << "size" << pix.size() << "to" << saveFile;
    if (!pix.save(saveFile))
    {
        KMessageBox::error(mainWindow(),
                           i18n("<qt>Failed to save image file:<br><filename>%1</filename>", saveFile),
                           i18n("Save Failed"));
        emit statusMessage(i18n("Failed to save map image"));
    }
    else
    {
        emit statusMessage(i18n("Saved map image to '%1'", saveFile));
    }

    view()->showOverlays(currentOverlays);
}


void MapController::slotShowPosition( const QString &pos)
{
// TODO: better display format
//    kDebug() << pos;
}


void MapController::slotZoomChanged(int zoom)
{
    //kDebug() << zoom << "min" << view()->minimumZoom() << "max" << view()->maximumZoom();
    emit mapZoomChanged((zoom<(view()->maximumZoom())),
                        (zoom>(view()->minimumZoom())));
// TODO: improve display, check against scale bar!
    emit statusMessage(i18n("At zoom %1 = %2 km", zoom,
                            view()->distanceFromZoom(zoom)));
}


void MapController::slotSelectTheme()
{
    if (mThemeManager==NULL)
    {
        kDebug() << "creating theme manager";
        mThemeManager = new MapThemeManager(this);
    }

    MapThemeDialogue d(mThemeManager->mapThemeModel(), mainWindow());
    d.setThemeId(view()->mapThemeId());
    if (!d.exec()) return;

    QString newTheme = d.themeId();
    kDebug() << "new theme" << newTheme;

    QStringList currentOverlays = view()->overlays(true);
    view()->showOverlays(QStringList());		// save/restore overlays state
    view()->setMapThemeId(newTheme);
    view()->showOverlays(currentOverlays);

    emit statusMessage(i18n("Map theme '%1'", newTheme));
}


void MapController::slotAboutMarble()
{
    MarbleAboutDialog d(mainWindow());
    d.exec();
}


MainWindow *MapController::mainWindow() const
{
    return (qobject_cast<MainWindow *>(parent()));
}


void MapController::gotoSelection(const QList<TrackDataItem *> &items)
{
    if (items.count()==0) return;

    BoundingArea bb = TrackData::unifyBoundingAreas(items);
    GeoDataLatLonBox ll(bb.north(), bb.south(),
                        bb.east(), bb.west(), GeoDataCoordinates::Degree);
    view()->centerOn(ll, true);
}
