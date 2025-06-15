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

#ifndef GPXEXPORTER_H
#define GPXEXPORTER_H

#include "exporterbase.h"

class TrackDataFile;
class TrackDataContainer;
class QXmlStreamWriter;
class CategoryList;


class GpxExporter : public ExporterBase
{
public:
    GpxExporter();
    virtual ~GpxExporter() = default;

    static QString filter();

protected:
    bool saveTo(QIODevice *devconst, const TrackDataFile *item) override;

private:
    bool writeItem(const TrackDataItem *item, QXmlStreamWriter &str, const QString &newName = QString()) const;
    bool writeChildren(const TrackDataContainer *item, QXmlStreamWriter &str) const;

private:
    const CategoryList *mCategoriesList;
    mutable const TrackDataItem *mHomePoint;
    mutable const TrackDataItem *mWorkPoint;
};

#endif							// GPXEXPORTER_H
