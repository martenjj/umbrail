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

#undef DEBUG_IMPORT

#include "marksimporter.h"

#include <qdebug.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qdatetime.h>

#include <kconfig.h>
#include <kconfiggroup.h>

#include "trackdata.h"
#include "dataindexer.h"
#include "errorreporter.h"

#define MARKS_FOLDER_NAME	"Marks"
#define GROUP_CREATOR		"Creator"
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

    mDataRoot->setMetadata("creator", grp.readEntry("AppName", ""));

    // from MainWindow::importFile() in navmarks/src/mainwindow.cpp
    QString source = QFileInfo(filePath).baseName();	// generate default source tag
    source += '_';
    source += QDateTime::currentDateTime().toString(Qt::ISODate);

    // TODO; build the colour/category map from the [Categories] group
    // and use it to set the "pointcolor" data for the waypoint

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

        s = grp.readEntry("Symbol", "");
        if (!s.isEmpty()) pnt->setMetadata("sym", s);
        s = grp.readEntry("Desc", "");
        if (!s.isEmpty()) pnt->setMetadata("desc", s);

        TrackData::WaypointFlags flags = static_cast<TrackData::WaypointFlags>(grp.readEntry("Flags", static_cast<int>(TrackData::NoFlags)));
        pnt->setMetadata("flags", static_cast<int>(flags));

        double lat = grp.readEntry("Latitude", NAN);
        double lon = grp.readEntry("Longtitude", NAN);
        if (lat!=NAN && lon!=NAN) pnt->setLatLong(lat, lon);
        double ele = grp.readEntry("Elevation", NAN);
        if (ele!=NAN) pnt->setMetadata("ele", s);

        // The address saved here is a 5-element list:
        // StreetAddress, City, State, PostalCode, Country
        QStringList l = grp.readEntry("Address", QStringList());
        if (!l.isEmpty())
        {
            // Values from 'enum AddressTag' in navmarks/src/pointdata.h
            s = l.value(0);
            if (!s.isEmpty()) pnt->setMetadata(DataIndexer::indexWithNamespace("StreetAddress", "gpxx"), s);
            s = l.value(1);
            if (!s.isEmpty()) pnt->setMetadata(DataIndexer::indexWithNamespace("City", "gpxx"), s);
            s = l.value(2);
            if (!s.isEmpty()) pnt->setMetadata(DataIndexer::indexWithNamespace("State", "gpxx"), s);
            s = l.value(3);
            if (!s.isEmpty()) pnt->setMetadata(DataIndexer::indexWithNamespace("PostalCode", "gpxx"), s);
            s = l.value(4);
            if (!s.isEmpty()) pnt->setMetadata(DataIndexer::indexWithNamespace("Country", "gpxx"), s);
        }

        l = grp.readEntry("Categories", QStringList());
        if (!l.isEmpty()) pnt->setMetadata("category", l.join(','));
        l = grp.readEntry("Sources", (QStringList() << source));
        if (!l.isEmpty()) pnt->setMetadata("origin", l.join(','));

        // This will always use the default folder, because none
        // is ever saved in the file.
        TrackDataFolder *folder = waypointFolder(pnt);
        Q_ASSERT(folder!=nullptr);
        // Clear the folder name metadata, it will be regenerated
        // when the file is exported.
        pnt->setMetadata("folder", QVariant());

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


// TODO: move to ImporterBase
TrackDataFolder *MarksImporter::getFolder(const QString &path)
{
#ifdef DEBUG_IMPORT
    qDebug() << path;
#endif

    const QStringList folders = path.split('/');
    Q_ASSERT(!folders.isEmpty());
    TrackDataItem *cur = mDataRoot;
    TrackDataFolder *foundFolder = nullptr;

    for (const QString &name : folders)			// look for existing subfolder
    {
        foundFolder = TrackData::findFolderByPath(name, cur);
        if (foundFolder==nullptr)			// nothing existing found
        {
            qDebug() << "creating" << name << "under" << cur->name();
            foundFolder = new TrackDataFolder;
            foundFolder->setName(name, true);
            cur->addChildItem(foundFolder);
        }

        cur = foundFolder;
    }

    return (foundFolder);
}


// TODO: move to ImporterBase with default name as parameter
TrackDataFolder *MarksImporter::waypointFolder(const TrackDataWaypoint *tdw)
{
    Q_ASSERT(tdw!=nullptr);

    // If the waypoint has a folder defined, then that folder is used.
    // Otherwise, an appropriately named top level folder is used, or
    // created if necessary.

    const QVariant path = tdw->metadata("folder");	// waypoint folder, if it has one
    if (!path.isNull()) return (getFolder(path.toString()));
							// find or create folder
    return (getFolder(MARKS_FOLDER_NAME));
}
