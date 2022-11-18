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

//////////////////////////////////////////////////////////////////////////
//									//
//  Debugging switches							//
//									//
//////////////////////////////////////////////////////////////////////////

#undef DEBUG_CACHE
#define DEBUG_CACHE

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
#ifdef DEBUG_CACHE
    qDebug() << "cache size" << sIconCache.maxCost();
#endif
}


void PointIconProvider::aboutToQuit() const
{
    // Dump statistics.  Done in a separate function called when
    // the main window is closed, to ensure that they are shown
    // before the debug streams are closed.
#ifdef DEBUG_CACHE
    qDebug() << "cache used" << sIconCache.size() << "total cost" << sIconCache.totalCost();
#endif
}


PointIconProvider *PointIconProvider::self()
{
    static PointIconProvider *instance = nullptr;
    if (instance==nullptr) instance = new PointIconProvider();
    return (instance);
}

//////////////////////////////////////////////////////////////////////////
//									//
//  Icon providers							//
//									//
//////////////////////////////////////////////////////////////////////////

const PointIcon *PointIconProvider::icon(const QString &name, PointIcon::IconNamespace nsp)
{
    if (sIconCache.contains(name))
    {
#ifdef DEBUG_CACHE
        qDebug() << "found" << name << "in cache";
#endif
        return (sIconCache.object(name));
    }

    PointIcon *ic = new PointIcon(name, nsp);		// deleted by cache when expired
#ifdef DEBUG_CACHE
    qDebug() << "saving" << name << "valid?" << ic->isValid() << "in cache";
#endif
    sIconCache.insert(name, ic, 2);			// named icon => lower cache cost
    return (ic);
}


const PointIcon *PointIconProvider::icon(const QColor &col)
{
    const QString name = "colour"+col.name();		// name for this coloured icon

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
