
#include "mapcontroller.h"

#include <qdebug.h>
#include <qfiledialog.h>

#include <klocalizedstring.h>
#include <kmessagebox.h>

#include <marble/MarbleAboutDialog.h>
#include <marble/GeoDataLatLonBox.h>

#include <recentsaver.h>
#include <imagefilter.h>

#include "mapview.h"
#include "trackdata.h"
#include "mapthemedialogue.h"
#include "mainwindow.h"
#include "settings.h"
#include "commands.h"
#include "filesview.h"


MapController::MapController(QObject *pnt)
    : QObject(pnt),
      MainWindowInterface(pnt)
{
    qDebug();

    mView = new MapView(mainWindow());
    connect(mView, SIGNAL(mouseMoveGeoPosition(const QString &)),
            SLOT(slotShowPosition(const QString &)));
    connect(mView, SIGNAL(zoomChanged(int)), SLOT(slotZoomChanged(int)));
    connect(mView, SIGNAL(draggedPoints(qreal,qreal)), SLOT(slotDraggedPoints(qreal,qreal)));

    mHomeLat = 51.436019;				// default to here
    mHomeLong = -0.352764;
    mHomeZoom = 2300;

    mThemeManager = NULL;				// created on demand
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

    if (KMessageBox::questionYesNo(mainWindow(),
                                   i18n("<qt>Set home position to <b>%1</b>?", TrackData::formattedLatLong(lat, lng)),
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
    qDebug() << "zoom" << mHomeZoom;
    view()->zoomView(mHomeZoom);
    emit statusMessage(i18n("At standard zoom %1", mHomeZoom));
}


void MapController::slotSaveImage()
{
    RecentSaver saver("mapsave");
    QUrl file = QFileDialog::getSaveFileUrl(mainWindow(),			// parent
                                            i18n("Save Map As Image"),		// caption
                                            saver.recentUrl("untitled"),	// dir
                                            ImageFilter::qtFilterString(ImageFilter::Writing),
                                            NULL,				// selectedFilter,
                                            QFileDialog::Options(),		// options
                                            QStringList("file"));		// supportedSchemes

    if (!file.isValid()) return;			// didn't get a file name
    saver.save(file);

    QStringList currentOverlays = view()->overlays(true);
    view()->showOverlays(QStringList());

    emit statusMessage(i18n("Saving map image..."));
    QPixmap pix = view()->mapScreenShot();
    qDebug() << "size" << pix.size() << "to" << file;
    if (!pix.save(file.path()))
    {
        KMessageBox::error(mainWindow(),
                           i18n("<qt>Failed to save image file:<br><filename>%1</filename>", file.toDisplayString()),
                           i18n("Save Failed"));
        emit statusMessage(i18n("Failed to save map image"));
    }
    else
    {
        emit statusMessage(i18n("Saved map image to '%1'", file.toDisplayString()));
    }

    view()->showOverlays(currentOverlays);
}


void MapController::slotShowPosition( const QString &pos)
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
    if (mThemeManager==NULL)
    {
        qDebug() << "creating theme manager";
        mThemeManager = new MapThemeManager(this);
    }

    MapThemeDialogue d(mThemeManager->mapThemeModel(), mainWindow());
    d.setThemeId(view()->mapThemeId());
    if (!d.exec()) return;

    QString newTheme = d.themeId();
    qDebug() << "new theme" << newTheme;

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


void MapController::gotoSelection(const QList<TrackDataItem *> &items)
{
    if (items.count()==0) return;

    BoundingArea bb = TrackData::unifyBoundingAreas(&items);
    if (!bb.isValid())					// empty container
    {
        KMessageBox::sorry(mainWindow(),
                           i18n("No valid position to show"),
                           i18n("No Position"));
        return;
    }

    GeoDataLatLonBox ll(bb.north(), bb.south(),
                        bb.east(), bb.west(), GeoDataCoordinates::Degree);
    view()->centerOn(ll, true);
}


void MapController::slotDraggedPoints(qreal latOff, qreal lonOff)
{
    qDebug() << latOff << lonOff;

    MovePointsCommand *cmd = new MovePointsCommand(filesController());
    cmd->setText(i18n("Move Points"));
    cmd->setDataItems(filesController()->view()->selectedItems());
    cmd->setData(latOff, lonOff);
    mainWindow()->executeCommand(cmd);
}
