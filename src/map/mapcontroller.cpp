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

#include "mapcontroller.h"

#include <qdebug.h>
#include <qfiledialog.h>

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
    connect(mView, &MarbleWidget::mouseMoveGeoPosition, this, &MapController::slotShowPosition);
    connect(mView, &MarbleWidget::zoomChanged, this, &MapController::slotZoomChanged);
    connect(mView, &MapView::draggedPoints, this, &MapController::mapDraggedPoints);

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

    if (KMessageBox::questionTwoActions(mainWidget(),
                                        xi18nc("@info", "Set home position to <emphasis strong=\"1\">%1</emphasis>?", TrackData::formattedLatLong(lat, lng)),
                                        i18n("Set Home Position?"),
                                        // KStandardGuiItem::yes() used same icon as ok()
                                        KGuiItem(i18n("Set"), KStandardGuiItem::ok().icon()),
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

    if (KMessageBox::questionTwoActions(mainWidget(),
                                        xi18nc("@info", "Set standard zoom to <emphasis strong=\"1\">%1</emphasis>?", zoom),
                                        i18n("Set Standard Zoom?"),
                                        KGuiItem(i18n("Set"), KStandardGuiItem::ok().icon()),
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
        KMessageBox::error(mainWidget(),
                           i18n("No valid position to show"),
                           i18n("No Position"));
        return;
    }

    GeoDataLatLonBox ll(bb.north(), bb.south(),
                        bb.east(), bb.west(), GeoDataCoordinates::Degree);
    view()->centerOn(ll, true);
}


void MapController::openExternalMap(MapBrowser::MapProvider map, const QList<TrackDataItem *> &items)
{
    // The displayed map bounding area in pixels
    const QRect rect = view()->mapRegion().boundingRect();

    // Not a BoundingArea, its API is not quite up to this task
    QRectF displayedArea;

    qreal lon, lat;
    if (!view()->geoCoordinates(rect.left(), rect.bottom(), lon, lat)) return;
    displayedArea.setLeft(lon);
    displayedArea.setBottom(lat);
    if (!view()->geoCoordinates(rect.right(), rect.top(), lon, lat)) return;
    displayedArea.setRight(lon);
    displayedArea.setTop(lat);
    qDebug() << "map" << map << "bounds" << displayedArea;

    const TrackDataAbstractPoint *selpoint = nullptr;
    if (items.count()==1)				// a single selected item
    {							// which must be a point
        selpoint = dynamic_cast<const TrackDataAbstractPoint *>(items.first());
    }

    MapBrowser::openBrowser(map, displayedArea, selpoint, mainWidget());
}
