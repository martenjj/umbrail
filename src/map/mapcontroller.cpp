
#include "mapcontroller.h"

#include <qdebug.h>
#include <qfiledialog.h>
#include <qurlquery.h>

#include <klocalizedstring.h>
#include <kmessagebox.h>

#include <marble/MarbleAboutDialog.h>
#include <marble/GeoDataLatLonBox.h>

#include <kfdialog/recentsaver.h>
#include <kfdialog/imagefilter.h>

#include "mapview.h"
#include "trackdata.h"
#include "mapthemedialogue.h"
#include "settings.h"


MapController::MapController(QObject *pnt)
    : QObject(pnt),
      ApplicationDataInterface(pnt)
{
    qDebug();

    mView = new MapView(mainWidget());
    connect(mView, SIGNAL(mouseMoveGeoPosition(const QString &)),
            SLOT(slotShowPosition(const QString &)));
    connect(mView, SIGNAL(zoomChanged(int)), SLOT(slotZoomChanged(int)));
    connect(mView, SIGNAL(draggedPoints(qreal,qreal)), SIGNAL(mapDraggedPoints(qreal,qreal)));

    mHomeLat = 51.436019;				// default to here
    mHomeLong = -0.352764;
    mHomeZoom = 2300;

    mThemeManager = nullptr;				// created on demand
}


MapController::~MapController()
{
    qDebug() << "done";
}


void MapController::clear()
{
    qDebug();
    slotGoHome();
}


void MapController::readProperties()
{
    bool haveHome = false;
    QString s = Settings::mapHome();
    if (!s.isEmpty())
    {
        haveHome = positionFromString(s, &mHomeLat, &mHomeLong, &mHomeZoom);
        qDebug() << "have home?" << haveHome;
    }

    view()->readProperties();

    if (!Settings::mapCurrent().isEmpty()) haveHome = false;
    if (haveHome) slotGoHome();				// go home if have that but no current
}

void MapController::saveProperties()
{
    QString s = positionToString(mHomeLat, mHomeLong, mHomeZoom);
    qDebug() << "home" << s;
    Settings::setMapHome(s);

    view()->saveProperties();
}



QString MapController::positionToString(double lat, double lon, int zoom)
{
    return (QString("%1,%2,%3").arg(lat).arg(lon).arg(zoom));
}


bool MapController::positionFromString(const QString &str, double *plat, double *plon, int *pzoom)
{
    if (str.isEmpty()) return (false);
    QStringList l = str.split(',');
    if (l.count()!=3) return (false);

    *plat = l[0].toDouble();
    *plon = l[1].toDouble();
    *pzoom = l[2].toInt();
    return (true);
}



void MapController::slotGoHome()
{
    qDebug() << "lat" << mHomeLat << "lon" << mHomeLong << "zoom" << mHomeZoom;
    view()->zoomView(mHomeZoom);
    view()->centerOn(mHomeLong, mHomeLat);

    emit statusMessage(i18n("At home position %1", TrackData::formattedLatLong(mHomeLat, mHomeLong)));
}


void MapController::slotSetHome()
{
    double lat = view()->centerLatitude();
    double lng = view()->centerLongitude();

    if (KMessageBox::questionYesNo(mainWidget(),
                                   xi18nc("@info", "Set home position to <emphasis strong=\"1\">%1</emphasis>?", TrackData::formattedLatLong(lat, lng)),
                                   i18n("Set Home Position?"),
                                   KGuiItem(i18n("Set"), KStandardGuiItem::yes().icon()),
                                   KStandardGuiItem::cancel(),
                                   "setHome")!=KMessageBox::Yes) return;
    mHomeLat = lat;
    mHomeLong = lng;
    // TODO: do this here?
    //mHomeZoom = view()->zoom();

    emit statusMessage(i18n("Home position set to %1", TrackData::formattedLatLong(mHomeLat, mHomeLong)));
    emit modified();
}


void MapController::slotSetZoom()
{
    int zoom = view()->zoom();

    if (KMessageBox::questionYesNo(mainWidget(),
                                   xi18nc("@info", "Set standard zoom to <emphasis strong=\"1\">%1</emphasis>?", zoom),
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
    qDebug() << "zoom" << mHomeZoom;
    view()->zoomView(mHomeZoom);
    emit statusMessage(i18n("At standard zoom %1", mHomeZoom));
}


void MapController::slotSaveImage()
{
    RecentSaver saver("mapsave");
    QUrl file = QFileDialog::getSaveFileUrl(mainWidget(),			// parent
                                            i18n("Save Map As Image"),		// caption
                                            saver.recentUrl("untitled"),	// dir
                                            ImageFilter::qtFilterString(ImageFilter::Writing),
                                            nullptr,				// selectedFilter,
                                            QFileDialog::Options(),		// options
                                            QStringList("file"));		// supportedSchemes

    if (!file.isValid()) return;			// didn't get a file name
    saver.save(file);

    QStringList currentOverlays = view()->allOverlays(true);
    view()->showOverlays(QStringList());

    emit statusMessage(i18n("Saving map image..."));
    QPixmap pix = view()->mapScreenShot();
    qDebug() << "size" << pix.size() << "to" << file;
    if (!pix.save(file.path()))
    {
        KMessageBox::error(mainWidget(),
                           xi18nc("@info", "Failed to save image file:<nl/><filename>%1</filename>", file.toDisplayString()),
                           i18n("Save Failed"));
        emit statusMessage(i18n("Failed to save map image"));
    }
    else
    {
        emit statusMessage(i18n("Saved map image to '%1'", file.toDisplayString()));
    }

    view()->showOverlays(currentOverlays);
}


void MapController::slotShowPosition(const QString &pos)
{
// TODO: better display format
//    qDebug() << pos;
}


void MapController::slotZoomChanged(int zoom)
{
    //qDebug() << zoom << "min" << view()->minimumZoom() << "max" << view()->maximumZoom();
    emit mapZoomChanged((zoom<(view()->maximumZoom())),
                        (zoom>(view()->minimumZoom())));
// TODO: improve display, check against scale bar!
    emit statusMessage(i18n("At zoom %1 = %2 km", zoom,
                            view()->distanceFromZoom(zoom)));
}


void MapController::slotSelectTheme()
{
    if (mThemeManager==nullptr)
    {
        qDebug() << "creating theme manager";
        mThemeManager = new MapThemeManager(this);
    }

    MapThemeDialogue *d = new MapThemeDialogue(mThemeManager->mapThemeModel(), mainWidget());
    d->setThemeId(view()->mapThemeId());
    d->setWindowModality(Qt::WindowModal);

    connect(d, &MapThemeDialogue::themeSelected, this, &MapController::slotMapThemeSelected);
    d->setAttribute(Qt::WA_DeleteOnClose);
    d->show();
}


void MapController::slotMapThemeSelected(const QString &themeId)
{
    qDebug() << "theme" << themeId;

    QStringList currentOverlays = view()->allOverlays(true);
    view()->showOverlays(QStringList());		// save/restore overlays state
    view()->setMapThemeId(themeId);
    view()->showOverlays(currentOverlays);

    emit statusMessage(i18n("Map theme '%1'", themeId));
}


void MapController::slotAboutMarble()
{
    MarbleAboutDialog d(mainWidget());
    d.exec();
}


void MapController::gotoSelection(const QList<TrackDataItem *> &items)
{
    if (items.count()==0) return;

    BoundingArea bb = TrackData::unifyBoundingAreas(&items);
    if (!bb.isValid())					// empty container
    {
        KMessageBox::sorry(mainWidget(),
                           i18n("No valid position to show"),
                           i18n("No Position"));
        return;
    }

    GeoDataLatLonBox ll(bb.north(), bb.south(),
                        bb.east(), bb.west(), GeoDataCoordinates::Degree);
    view()->centerOn(ll, true);
}


//  Derive a Google Maps zoom level for lat/lon map bounds,
//  which also seems to be suitable for Bing.
//  https://stackoverflow.com/questions/6048975/google-maps-v3-how-to-calculate-the-zoom-level-for-a-given-bounds

static double latRad(double lat)
{
    double s = sin(lat*M_PI/180);
    double rx2 = log((1+s)/(1-s))/2;
    return (qBound(-M_PI, rx2, M_PI)/2);
}


static int zoom(int mapPx, int worldPx, double frac)
{
    return (static_cast<int>(floor(log((mapPx/worldPx)/frac)/log(2.0))));
}


static int getBoundsZoomLevel(double minlon, double minlat, double maxlon, double maxlat)
{
    double lngDiff = maxlon-minlon;

    double latFraction = (latRad(maxlat)-latRad(minlat))/M_PI;
    double lngFraction = lngDiff<0 ? (lngDiff+360)/360 : lngDiff/360;

    // Assuming a size of 1024x768 for the browser window.
    int latZoom = zoom(768, 256, latFraction);
    int lngZoom = zoom(1024, 256, lngFraction);
    return (qMin(qMin(latZoom, lngZoom), 21));
}


void MapController::openExternalMap(MapBrowser::MapProvider map, const QList<TrackDataItem *> &items)
{
    qDebug() << "map" << map << "with" << items.count() << "items";

    const QRect rect = view()->mapRegion().boundingRect();

    qreal minlon, minlat;
    if (!view()->geoCoordinates(rect.left(), rect.bottom(), minlon, minlat)) return;
    qreal maxlon, maxlat;
    if (!view()->geoCoordinates(rect.right(), rect.top(), maxlon, maxlat)) return;

    qDebug() << "map bounds" << minlat << minlon << "-" << maxlat << maxlon;

    const TrackDataAbstractPoint *selpoint = nullptr;
    if (items.count()==1)				// a single selected item
    {							// which must be a point
        selpoint = dynamic_cast<const TrackDataAbstractPoint *>(items.first());
    }

    QUrl u;
    QUrlQuery q;

    switch (map)
    {
case MapBrowser::OSM:
        // For OSM URL format see https://wiki.openstreetmap.org/wiki/Browsing
        u = QUrl("https://openstreetmap.org/");

        q.addQueryItem("bbox", QString("%1,%2,%3,%4").arg(minlon).arg(minlat).arg(maxlon).arg(maxlat));
        if (selpoint!=nullptr)
        {
            q.addQueryItem("mlat", QString::number(selpoint->latitude()));
            q.addQueryItem("mlon", QString::number(selpoint->longitude()));
        }

        u.setQuery(q);
        break;

#ifdef ENABLE_OPEN_WITH_GOOGLE
case MapBrowser::Google:
        // For Google Maps URL format see
        // https://developers.google.com/maps/documentation/urls/get-started#map-action
        u = QUrl("https://google.com");
        q.addQueryItem("api", "1");

        // Unfortunately a marker does not appear to be supported together with
        // specified map position and zoom.  If there is a single selected item
        // then display the map at that position;  otherwise, display the map
        // as close to the currently displayed bounds as possible.
        if (selpoint!=nullptr)
        {
            u.setPath("/maps/search/");			// trailing slash is needed
            q.addQueryItem("query", QString("%1,%2").arg(selpoint->latitude()).arg(selpoint->longitude()));
        }
        else
        {
            u.setPath("/maps/@");
            q.addQueryItem("map_action", "map");
            q.addQueryItem("center", QString("%1,%2").arg((minlat+maxlat)/2).arg((minlon+maxlon)/2));
            q.addQueryItem("zoom", QString::number(getBoundsZoomLevel(minlon, minlat, maxlon, maxlat)));
        }
        break;
#endif // ENABLE_OPEN_WITH_GOOGLE

#ifdef ENABLE_OPEN_WITH_BING
case MapBrowser::Bing:
        // For Bing Maps URL format see
        // https://docs.microsoft.com/en-us/bingmaps/articles/create-a-custom-map-url
        u = QUrl("https://bing.com/maps/default.aspx");

        q.addQueryItem("cp", QString("%1~%2").arg((minlat+maxlat)/2).arg((minlon+maxlon)/2));
        q.addQueryItem("lvl", QString::number(getBoundsZoomLevel(minlon, minlat, maxlon, maxlat)));

        if (selpoint!=nullptr)
        {

            q.addQueryItem("sp", QString("point.%1_%2_%3").arg(selpoint->latitude())
                                                          .arg(selpoint->longitude())
                                                          .arg(selpoint->name()));
        }
        break;
#endif // ENABLE_OPEN_WITH_BING
    }

    if (!q.isEmpty()) u.setQuery(q);
    MapBrowser::self()->openBrowser(map, u, mainWidget());
}
