//////////////////////////////////////////////////////////////////////////
//									//
//  Project:	NavTracks						//
//  Edit:	06-Aug-21						//
//									//
//////////////////////////////////////////////////////////////////////////
//									//
//  Copyright (c) 2012-2014 Jonathan Marten <jjm@keelhaul.me.uk>	//
//  Home and download page:  http://www.keelhaul.me.uk/TBD/		//
//									//
//  This program is free software; you can redistribute it and/or	//
//  modify it under the terms of the GNU General Public License as	//
//  published by the Free Software Foundation; either version 2 of	//
//  the License, or (at your option) any later version.			//
//									//
//  It is distributed in the hope that it will be useful, but		//
//  WITHOUT ANY WARRANTY; without even the implied warranty of		//
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the	//
//  GNU General Public License for more details.			//
//									//
//  You should have received a copy of the GNU General Public		//
//  License along with this program; see the file COPYING for further	//
//  details.  If not, write to the Free Software Foundation, Inc.,	//
//  59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.		//
//									//
//////////////////////////////////////////////////////////////////////////

#include "mapbrowser.h"

#include <qwidget.h>
#include <qdebug.h>

#include <kio/applicationlauncherjob.h>
#include <kio/openurljob.h>

#include "settings.h"


MapBrowser::MapBrowser()
{
    qDebug() << "allocated global instance";
}


MapBrowser *MapBrowser::self()
{
    static MapBrowser *instance = new MapBrowser();
    return (instance);
}


void MapBrowser::openBrowser(MapBrowser::MapProvider map, const QUrl &url, QWidget *pnt)
{
    qDebug() << "map" << map << "url" << url;

    QString browserService;
    switch (map)
    {
case MapBrowser::OSM:
        browserService = Settings::mapBrowserOSM();
        break;

#ifdef ENABLE_OPEN_WITH_GOOGLE
case MapBrowser::Google:
        browserService = Settings::mapBrowserGoogle();
        break;
#endif // ENABLE_OPEN_WITH_GOOGLE

#ifdef ENABLE_OPEN_WITH_BING
case MapBrowser::Bing:
        browserService = Settings::mapBrowserBing();
        break;
#endif // ENABLE_OPEN_WITH_BING

default:
        qWarning() << "unknown external map" << map;
        return;
    }

    // Make sure that the browser URL is valid.  It should always have been
    // resolved by MapController::openExternalMap() by now, because otherwise
    // the 'default' above will have been hit.
    if (!url.isValid()) return;

    // TODO: if not OSM, display a warning (with "Do not show again")
    // that copying data from Google/Bing to OSM is not allowed

    qDebug() << "service" << browserService;
    if (browserService.isEmpty())
    {
        // There is no configured browser service, just open the map URL
        // in the default browser.
        KIO::OpenUrlJob *job = new KIO::OpenUrlJob(url, "text/html", pnt);
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
        job->setUrls(QList<QUrl>() << url);
        job->start();
    }
}
