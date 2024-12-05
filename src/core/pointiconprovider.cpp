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

#include "pointiconprovider.h"

#include <qdebug.h>
#include <qcache.h>

#include "trackdata.h"

//////////////////////////////////////////////////////////////////////////
//									//
//  Debugging switches							//
//									//
//////////////////////////////////////////////////////////////////////////

#undef DEBUG_CACHE

//////////////////////////////////////////////////////////////////////////
//									//
//  Static data								//
//									//
//////////////////////////////////////////////////////////////////////////

static QCache<QString, PointIcon> sIconCache;

//////////////////////////////////////////////////////////////////////////
//									//
//  Constructor/destructor/instance					//
//									//
//////////////////////////////////////////////////////////////////////////

PointIconProvider::PointIconProvider()
{
    sIconCache.setMaxCost(4000);
    qDebug() << "cache size" << sIconCache.maxCost();
}


void PointIconProvider::aboutToQuit() const
{
    // Dump statistics.  Done in a separate function called when
    // the main window is closed, to ensure that they are shown
    // before the debug streams are closed.
    qDebug() << "cache used" << sIconCache.size() << "total cost" << sIconCache.totalCost();
}


PointIconProvider *PointIconProvider::self()
{
    static PointIconProvider *instance = new PointIconProvider();
    return (instance);
}

//////////////////////////////////////////////////////////////////////////
//									//
//  Icon providers							//
//									//
//////////////////////////////////////////////////////////////////////////

const PointIcon *PointIconProvider::icon(const QString &name, PointIcon::IconNamespace nsp, const TrackDataItem *item)
{
    QString cacheKey = name+'-'+QString::number(nsp);
    if (item!=nullptr)
    {
        QVariant v = item->metadata("background");
        if (!v.isNull()) cacheKey += '-'+v.toString();
        v = item->metadata("pointcolor");
        if (!v.isNull()) cacheKey += '-'+v.toString();
    }

    if (sIconCache.contains(cacheKey))
    {
#ifdef DEBUG_CACHE
        qDebug() << "found" << cacheKey << "in cache";
#endif
        return (sIconCache.object(cacheKey));
    }

    PointIcon *ic = new PointIcon(name, nsp, item);	// deleted by cache when expired
#ifdef DEBUG_CACHE
    qDebug() << "saving" << name << "valid?" << ic->isValid() << "in cache";
#endif
    sIconCache.insert(cacheKey, ic, 2);			// named icon => lower cache cost
    return (ic);
}


const PointIcon *PointIconProvider::icon(const QColor &col)
{
    const QString name = "colour-"+col.name();		// name for this coloured icon

    if (sIconCache.contains(name))
    {
#ifdef DEBUG_CACHE
        qDebug() << "found" << name << "in cache";
#endif
        return (sIconCache.object(name));
    }

    PointIcon *ic = new PointIcon(name, col);		// deleted by cache when expired
#ifdef DEBUG_CACHE
    qDebug() << "saving" << name << "valid?" << ic->isValid() << "in cache";
#endif
    sIconCache.insert(name, ic, 3);			// coloured item => higher cache cost
    return (ic);
}


const PointIcon *PointIconProvider::icon(const QString &name, const QByteArray &nsn, const TrackDataItem *item)
{
    return (icon(name, PointIcon::namespaceId(nsn), item));
}
