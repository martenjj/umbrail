//////////////////////////////////////////////////////////////////////////
//									//
//  Project:	NavTracks						//
//  Edit:	22-Jan-18						//
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

#include "pluginmanager.h"

#include <qpluginloader.h>
#include <qdir.h>
#include <qcoreapplication.h>
#include <qdebug.h>


static PluginManager *sInstance = NULL;


PluginManager::PluginManager()
{
    QStringList pluginPaths = QCoreApplication::libraryPaths();
    qDebug() << "initial paths" << pluginPaths;

    // Assume that the first path entry is the standard install location.
    // Our plugins will be in a subdirectory of that.
    Q_ASSERT(!pluginPaths.isEmpty());
    QString installPath = pluginPaths.takeFirst();
    pluginPaths.prepend(installPath+"/"+QCoreApplication::applicationName());

    // Also add a subdirectory of the executable directory,
    // for use when running in place.  In order to get the most up-to-date
    // plugins, this needs to have priority over all other install locations.
    pluginPaths.prepend(QCoreApplication::applicationDirPath()+"/plugins");

    // Put back the standard install location, for locating KParts and
    // other plugins which may be needed.
    pluginPaths.append(installPath);

    qDebug() << "running with paths" << pluginPaths;
    QCoreApplication::setLibraryPaths(pluginPaths);
}


PluginManager *PluginManager::self()
{
    if (sInstance==NULL)
    {
        sInstance = new PluginManager();
        qDebug() << "allocated global instance";
    }
    return (sInstance);
}


QList<QObject *> PluginManager::loadPlugins(PluginManager::PluginType type)
{
    qDebug() << "for type" << type;

    if (!mPluginMap.contains(type))			// see if plugins scanned already
    {							// if not, need to do that now
        qDebug() << "scanning for plugins";

        QString filter;
        switch (type)
        {
case PluginManager::CoordinatePlugin:
            filter = "*coordinate.*";
            break;

default:    qWarning() << "Unknown plugin type" << type;
            filter = "*unknown.*";
            break;
        }

        // Scan all of the library paths for plugins of the requested type
        const QStringList pluginPaths = QCoreApplication::libraryPaths();
        QStringList pluginLibs;
        foreach (const QString &path, pluginPaths)
        {
            QDir dir(path);
            qDebug() << "  searching" << dir.absolutePath();
            if (!dir.exists()) continue;

            const QStringList files = dir.entryList((QStringList() << filter),
                                                    QDir::Files|QDir::NoDotAndDotDot);
            foreach (const QString &file, files)
            {
                qDebug() << "    file" << file;
                // Ignore "libabstractcoordinate.so*" in build directory
                if (file.startsWith("lib")) continue;

                if (!pluginLibs.contains(file))	// if not already seen,
                {					// record name of plugin
                    qDebug() << "  found plugin" << dir.absoluteFilePath(file);
                    pluginLibs.append(file);
                }
            }
        }

        // The order of retrieval from a multi-item container is unpredictable.
        // We would like the plugins to be retrieved in a predictable order - the
        // alphabetical sorting of their names - so we manage a sorted list by hand.
        pluginLibs.sort();
        qDebug() << "pluginLibs" << pluginLibs;

        auto *pluginList = new QList<QPluginLoader *>;
        foreach (const QString &pluginLib, pluginLibs)
        {
            qDebug() << "loading plugin" << pluginLib;
            QPluginLoader *loader = new QPluginLoader(pluginLib);
            pluginList->append(loader);
        }

        mPluginMap.insert(type, pluginList);
    }

    auto *pluginList = mPluginMap.value(type);
    qDebug() << "have" << pluginList->count() << "plugins";

    QList<QObject *> pluginObjects;
    foreach (QPluginLoader *loader, *pluginList)
    {
        QObject *obj = loader->instance();
        if (obj==nullptr)
        {
            qWarning() << "failed to create instance from" << loader->fileName() << loader->errorString();
            continue;
        }

        pluginObjects.append(obj);
    }

    qDebug() << "loaded" << pluginObjects.count() << "plugins";
    return (pluginObjects);
}
