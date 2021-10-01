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

#include "mapbrowser.h"

#include <math.h>

#include <qwidget.h>
#include <qdebug.h>
#include <qurlquery.h>

#include <kmessagebox.h>
#include <klocalizedstring.h>
#include <kio/applicationlauncherjob.h>
#include <kio/openurljob.h>

#include "settings.h"
#include "trackdata.h"


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


static int getBoundsZoomLevel(const QRectF &bounds)
{
    double lngDiff = bounds.right()-bounds.left();

    double latFraction = (latRad(bounds.top())-latRad(bounds.bottom()))/M_PI;
    double lngFraction = lngDiff<0 ? (lngDiff+360)/360 : lngDiff/360;

    // Assuming a size of 1024x768 for the browser window.
    int latZoom = zoom(768, 256, latFraction);
    int lngZoom = zoom(1024, 256, lngFraction);
    return (qMin(qMin(latZoom, lngZoom), 21));
}


void MapBrowser::openBrowser(MapBrowser::MapProvider map,
                             const QRectF &displayedArea,
                             const TrackDataAbstractPoint *selectedPoint,
                             QWidget *pnt)
{
    QString browserService;
    QUrl u;
    QUrlQuery q;

    switch (map)
    {
case MapBrowser::OSM:
        browserService = Settings::mapBrowserOSM();

        // For OSM URL format see https://wiki.openstreetmap.org/wiki/Browsing
        u.setUrl("https://openstreetmap.org/");

        q.addQueryItem("bbox", QString("%1,%2,%3,%4").arg(displayedArea.left())
                                                     .arg(displayedArea.bottom())
                                                     .arg(displayedArea.right())
                                                     .arg(displayedArea.top()));
        if (selectedPoint!=nullptr)
        {
            q.addQueryItem("mlat", QString::number(selectedPoint->latitude()));
            q.addQueryItem("mlon", QString::number(selectedPoint->longitude()));
        }
        break;

#ifdef ENABLE_OPEN_WITH_GOOGLE
case MapBrowser::Google:
        browserService = Settings::mapBrowserGoogle();

        // For Google Maps URL format see
        // https://developers.google.com/maps/documentation/urls/get-started
        u.setUrl("https://google.com");
        q.addQueryItem("api", "1");

        // Unfortunately a marker does not appear to be supported together with
        // specified map position and zoom.  If there is a single selected item
        // then display the map at that position;  otherwise, display the map
        // as close to the currently displayed bounds as possible.
        if (selectedPoint!=nullptr)
        {
            u.setPath("/maps/search/");			// trailing slash is needed
            q.addQueryItem("query", QString("%1,%2").arg(selectedPoint->latitude())
                                                    .arg(selectedPoint->longitude()));
        }
        else
        {
            u.setPath("/maps/@");
            q.addQueryItem("map_action", "map");
            q.addQueryItem("center", QString("%1,%2").arg(displayedArea.center().y())
                                                     .arg(displayedArea.center().x()));
            q.addQueryItem("zoom", QString::number(getBoundsZoomLevel(displayedArea)));
        }
        break;
#endif // ENABLE_OPEN_WITH_GOOGLE

#ifdef ENABLE_OPEN_WITH_BING
case MapBrowser::Bing:
        browserService = Settings::mapBrowserBing();

        // For Bing Maps URL format see
        // https://docs.microsoft.com/en-us/bingmaps/articles/create-a-custom-map-url
        u.setUrl("https://bing.com/maps/default.aspx");

        q.addQueryItem("cp", QString("%1~%2").arg(displayedArea.center().y())
                                             .arg(displayedArea.center().x()));
        q.addQueryItem("lvl", QString::number(getBoundsZoomLevel(displayedArea)));

        if (selectedPoint!=nullptr)
        {
            q.addQueryItem("sp", QString("point.%1_%2_%3").arg(selectedPoint->latitude())
                                                          .arg(selectedPoint->longitude())
                                                          .arg(selectedPoint->name()));
        }
        break;
#endif // ENABLE_OPEN_WITH_BING

default:
        qWarning() << "unknown external map" << map;
        return;
    }

    // Make sure that the browser URL is valid.  It should always have been
    // resolved by now, because otherwise the 'default' above will have been
    // hit.  Then add the URL query parameters.
    if (!u.isValid()) return;
    if (!q.isEmpty()) u.setQuery(q);

    // If not opening OpenStreetMap, display a warning that copying data from
    // Google/Bing to OSM is not allowed.  This is not saying that the user is
    // intending to do so, but just a warning on the first opportunity.
    if (map!=MapBrowser::OSM)
    {
        KMessageBox::information(pnt,
                                 xi18nc("@info",
"<para>"
"Please note that copying data from Google or Bing Maps (or any other copyright map or data source) "
"to OpenStreetMap is not allowed, except in certain limited cases. You should ensure that your use "
"of these maps is permitted under their terms of service."
"</para>"
"<para>"
"See <link url=\"https://wiki.openstreetmap.org/wiki/Copyright\">Copyright</link> "
"on the OpenStreetMap Wiki for more information."
"</para>"
),
                                 QString(),
                                 "closedMapWarning",
                                 KMessageBox::AllowLink);
    }

    qDebug() << "for map" << map << "service" << browserService << "url" << u;
    if (browserService.isEmpty())
    {
        // There is no configured browser service, just open the map URL
        // in the default web browser.
        KIO::OpenUrlJob *job = new KIO::OpenUrlJob(u, "text/html", pnt);
        job->start();					// assume MIME type
    }
    else						// there is a configured service
    {
        // There is a configured browser service, use it to open the map URL.
        KService::Ptr service = KService::serviceByStorageId(browserService);
        if (!service)
        {
            qWarning() << "unknown service" << browserService;
            return;
        }

        KIO::ApplicationLauncherJob *job = new KIO::ApplicationLauncherJob(service, pnt);
        job->setUrls(QList<QUrl>() << u);
        job->start();
    }
}
