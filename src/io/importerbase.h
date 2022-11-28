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

#ifndef IMPORTERBASE_H
#define IMPORTERBASE_H

#include <qcolor.h>

#include "importerexporterbase.h"

class QUrl;
class QIODevice;
class TrackDataFile;
class TrackDataFolder;
class TrackDataWaypoint;


class ImporterBase : public ImporterExporterBase
{
public:
    ImporterBase();
    virtual ~ImporterBase() = default;

    TrackDataFile *load(const QUrl &file);
    virtual bool needsResave() const				{ return (false); }

protected:
    // TODO: private with accessor
    TrackDataFile *mDataRoot;

protected:
    virtual bool loadFrom(QIODevice *dev) = 0;

protected:
    TrackDataFolder *waypointFolder(const TrackDataWaypoint *tdw, const QString &defaultName);

private:
    TrackDataFolder *getFolder(const QString &path);
};

#endif							// IMPORTERBASE_H
