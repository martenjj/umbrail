//////////////////////////////////////////////////////////////////////////
//									//
//  Project:	NavTracks						//
//  Edit:	19-Nov-17						//
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

#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#include <qlist.h>
#include <qmap.h>


class QObject;
class QPluginLoader;


/**
 * @short Manages plugins
 *
 *
 * @author Jonathan Marten
 **/

class PluginManager
{
public:
    enum PluginType
    {
        CoordinatePlugin
    };

    /**
     * Get the singleton instance, creating it if necessary.
     *
     * @return the instance
     **/
    static PluginManager *self();

    /**
     * Load and create an instance object from all of the plugins of a specified type.
     *
     * @param type The type of the plugins required
     * @return a list of plugin instance objects of that type
     **/
    QList<QObject *> loadPlugins(PluginManager::PluginType type);

private:
    PluginManager();
    ~PluginManager() = default;

private:
    QMap<PluginManager::PluginType, QList<QPluginLoader *> *> mPluginMap;
};

#endif							// PLUGINMANAGER_H
