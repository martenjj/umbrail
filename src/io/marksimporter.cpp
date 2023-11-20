//////////////////////////////////////////////////////////////////////////
//									//
//  Project:	Umbrail - GPX track viewer and editor			//
//									//
//////////////////////////////////////////////////////////////////////////
//									//
//  Copyright (c) 2014-2022 Jonathan Marten <jjm@keelhaul.me.uk>	//
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

#undef DEBUG_IMPORT

#include "marksimporter.h"

#include <qdebug.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qdatetime.h>
#include <qcolor.h>

#include <kconfig.h>
#include <kconfiggroup.h>

#include "trackdata.h"
#include "dataindexer.h"
#include "errorreporter.h"
#include "category.h"

#define MARKS_FOLDER_NAME	"Marks"
#define GROUP_CREATOR		"Creator"
#define GROUP_MAP		"Map"
#define GROUP_CATEGORIES	"Categories"
#define GROUP_POINT		"Point_"


MarksImporter::MarksImporter()
    : ImporterBase()
{
    qDebug();
}


// Based on navmarks/src/marksimporter.cpp
bool MarksImporter::loadFrom(QIODevice *dev)
{
    QFile *file = qobject_cast<QFile *>(dev);
    if (file==nullptr)
    {
        reporter()->setError(ErrorReporter::Fatal, "Cannot read from device");
        return (false);
    }

    const QString filePath = file->fileName();
    qDebug() << "from" << filePath;

    const KConfig conf(filePath, KConfig::SimpleConfig);
    KConfigGroup grp = conf.group(GROUP_CREATOR);	// create config from file
    if (!grp.exists())					// check for our marker group
    {
        reporter()->setError(ErrorReporter::Fatal, "Unrecognised file format (no [" GROUP_CREATOR "] group)");
        return (false);
    }

    dataRoot()->setMetadata("creator", grp.readEntry("AppName", ""));

    // Set the current map position/zoom from the settings saved
    // in the import file.  The other map settings - home point,
    // overlays and theme - use the global application settings.
    grp = conf.group(GROUP_MAP);
    dataRoot()->setMetadata("position", grp.readEntry("Current", ""));

    // Allocate the category map and set it on the root file item.
    // The user of that root item takes ownership of it.
    CategoryList *catMap = new CategoryList;
    dataRoot()->setCategories(catMap);

    // Load the category->colour map from the [Categories] group,
    // which may be needed later to resolve the point colour data for
    // imported waypoints.  Do not set the "pointcolor" metadata for
    // the imported points directly, because that would result in the
    // points being exported with an explicit colour instead of it
    // being determined by category.
    grp = conf.group(GROUP_CATEGORIES);
    for (int i = 0; ; ++i)
    {
        QString nameKey = "Name"+QString::number(i);	// key for category name
        if (!grp.hasKey(nameKey)) break;		// no such key => end of map
        QString nameVal = grp.readEntry(nameKey, "");	// get category name
        if (nameVal.isEmpty()) continue;		// blank name, should never happen

        QString colKey = "Colour"+QString::number(i);	// key for category colour
        QColor colVal = grp.readEntry(colKey, QColor());
						        // get colour for category
        catMap->addCategory(nameVal, CategoryData(colVal));
    }							// add entry to categories
    qDebug() << "category map" << catMap->count() << "entries";

    int num = 0;
    const QStringList groups = conf.groupList();
    for (const QString &grpName : groups)
    {
        if (!grpName.startsWith(GROUP_POINT)) continue;
        grp = conf.group(grpName);

        TrackDataWaypoint *pnt = new TrackDataWaypoint;	// start new waypoint item

        // based on PointData::load() in navmarks/src/pointdata.cpp

        QString s = grp.readEntry("Name", "");
        if (!s.isEmpty()) pnt->setName(s, true);

        pnt->setMetadata("sym", grp.readEntry("Symbol", ""));
        pnt->setMetadata("desc", grp.readEntry("Desc", ""));

        TrackData::WaypointFlags flags = static_cast<TrackData::WaypointFlags>(grp.readEntry("Flags", static_cast<int>(TrackData::NoFlags)));
        pnt->setMetadata("flags", static_cast<int>(flags));

        double lat = grp.readEntry("Latitude", NAN);
        double lon = grp.readEntry("Longtitude", NAN);
        if (!ISNAN(lat) && !ISNAN(lon)) pnt->setLatLong(lat, lon);
        double ele = grp.readEntry("Elevation", NAN);
        if (!ISNAN(ele)) pnt->setMetadata("ele", ele);

        // The address saved here is a 5-element list:
        // StreetAddress, City, State, PostalCode, Country
        QStringList l = grp.readEntry("Address", QStringList());
        if (!l.isEmpty())
        {
            // Values from 'enum AddressTag' in navmarks/src/pointdata.h
            // No metadata item will be set if the string valus is empty.
            pnt->setMetadata(DataIndexer::indexWithNamespace("StreetAddress", "gpxx"), l.value(0));
            pnt->setMetadata(DataIndexer::indexWithNamespace("City", "gpxx"), l.value(1));
            pnt->setMetadata(DataIndexer::indexWithNamespace("State", "gpxx"), l.value(2));
            pnt->setMetadata(DataIndexer::indexWithNamespace("PostalCode", "gpxx"), l.value(3));
            pnt->setMetadata(DataIndexer::indexWithNamespace("Country", "gpxx"), l.value(4));
        }

        pnt->setMetadata("category", grp.readEntry("Categories", QStringList()));
        pnt->setMetadata("origin", grp.readEntry("Sources", (QStringList() << originId())));

        // This will always use the default folder, because none
        // is ever saved in the file.
        TrackDataFolder *folder = waypointFolder(pnt, MARKS_FOLDER_NAME);
        Q_ASSERT(folder!=nullptr);
        // Clear the folder name metadata, it will be regenerated
        // when the file is exported.
        pnt->setMetadata("folder", QVariant());

        // If requested, mark the waypoint as "newly imported".  As with GPX
        // import the flag will be set on all waypoints in this file, but if
        // they eventually get merged as duplicates into the main data tree
        // then the flag set here is ignored.
        if (options().hasFlag(ImporterExporterOptions::MarkNewWaypoints))
        {
            pnt->setMetadata("flags", static_cast<int>(TrackData::NewlyImported));
        }

        folder->addChildItem(pnt);			// add to destination folder
        ++num;						// count up this point
    }

    qDebug() << "done, imported" << num << "points";
    return (true);
}


QString MarksImporter::filter()
{
    return ("POI marks files (*.marks)");
}


