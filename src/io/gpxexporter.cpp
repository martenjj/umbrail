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

#include "gpxexporter.h"

#include <qcolor.h>
#include <qdebug.h>
#include <qqueue.h>
#include <qpair.h>

#include <QXmlStreamWriter>

#include "trackdata.h"
#include "dataindexer.h"
#include "category.h"
#include "pointicon.h"

// GPX specification: http://www.topografix.com/GPX/1/1/

// According to the GPX specification, extensions must appear after
// any children of an element.  However, the output GPX file is more
// readable if they appear before.  Define this symbol to have them
// appear after the children.
#undef EXTENSIONS_AFTER_CHILDREN


// A class to store a tag name and value pair.
class TagValue : protected QPair<QByteArray,QString>
{
public:
    TagValue(const QByteArray &name, const QString &val) : QPair<QByteArray,QString>(name, val) {}
    ~TagValue() = default;

    QByteArray name() const					{ return (first); }
    QString value() const					{ return (second); }
};


// A class to store a first-in-first-out queue of
// tag name and value pairs.
class TagQueue : protected QQueue<TagValue>
{
public:
    explicit TagQueue() = default;
    ~TagQueue() = default;

    bool isEmpty() const 					{ return (QQueue::isEmpty()); }
    void enqueue(const QByteArray &name, const QString &val)	{ QQueue::enqueue(TagValue(name, val)); }
    TagValue dequeue()						{ return (QQueue::dequeue()); }
};


static void writeValue(const QByteArray &name, const QString &val, QXmlStreamWriter &str)
{
    if (name=="link")					// special format for this
    {
        str.writeEmptyElement(name);
        str.writeAttribute("link", val);
    }
    else
    {
        str.writeTextElement(DataIndexer::nameWithNamespace(name), val);
    }
}


static void writeValue(const TagValue &tv, QXmlStreamWriter &str)
{
    writeValue(tv.name(), tv.value(), str);
}


static void writeQueue(TagQueue *queue, QXmlStreamWriter &str)
{
    while (!queue->isEmpty()) writeValue(queue->dequeue(), str);
}


static QString valueString(const QVariant &v)
{
    QString data;
    switch (v.typeId())
    {
case QMetaType::QDateTime:				// date in ISO format
        data = v.toDateTime().toString(Qt::ISODate);
        break;

case QMetaType::QStringList:				// comma separated list
        data = v.toStringList().join(',');
        break;

default:
        data = v.toString();				// default string format
        break;
    }

    return (data);
}


static bool isExtensionTag(const TrackDataItem *item, const QByteArray &name)
{
    if (dynamic_cast<const TrackDataFile *>(item)!=nullptr) return (false);
							// file metadata - never in extensions
    if (DataIndexer::isApplicationTag(name)) return (true);
							// application tag - always in extensions
    if (dynamic_cast<const TrackDataAbstractPoint *>(item)!=nullptr)
    {
        if (dynamic_cast<const TrackDataWaypoint *>(item)!=nullptr)
        {						// waypoint - these not in extensions
            if (name=="link"|| name=="sym" || name=="category" || name =="type") return (false);
        }
							// point - these not in extensions
        return (!(name=="ele" || name=="time" || name=="hdop"));
    }
    else if (dynamic_cast<const TrackDataTrack *>(item)!=nullptr)
    {							// track - these not in extensions
        return (!(name=="desc" || name=="type"));
    }
    else if (dynamic_cast<const TrackDataRoute *>(item)!=nullptr)
    {							// route - these not in extensions
        return (!(name=="desc" || name=="type"));
    }
    else if (dynamic_cast<const TrackDataSegment *>(item)!=nullptr)
    {							// segment - all in extensions
        return (true);
    }
    else return (false);				// other - assume not in extensions
}


static bool isAddressTag(const QByteArray &name)
{
    return (name=="StreetAddress" || name=="City" || name=="State" || name=="PostalCode" || name=="Country");
}


// The GPX exporter main class.
GpxExporter::GpxExporter()
    : ExporterBase()
{
    qDebug();
    mCategoriesList = nullptr;
}


// This cannot be file-static because it calls writeItem().
bool GpxExporter::writeChildren(const TrackDataItem *item, QXmlStreamWriter &str) const
{
    int num = item->childCount();
    for (int i = 0; i<num; ++i)
    {
        if (!writeItem(item->childAt(i), str)) return (false);
    }

    return (true);
}


// This cannot be file-static because it needs to be able to
// access ExporterBase::isSelected().
bool GpxExporter::writeItem(const TrackDataItem *item, QXmlStreamWriter &str, const QString &newName) const
{
    // If the item is not selected for export, then simply look inside
    // and process its child items.
    if (!isSelected(item)) return (writeChildren(item, str));

    // The name to use when writing out this item.  If the name is
    // automatically assigned (not explicit), then no name is written.
    // If a new name to override the existing one is specified then
    // that name is used.
    QString itemName = newName;
    if (itemName.isEmpty() && item->hasExplicitName()) itemName = item->name();

    // What sort of element?
    const TrackDataTrack *tdt = dynamic_cast<const TrackDataTrack *>(item);
    const TrackDataSegment *tds = dynamic_cast<const TrackDataSegment *>(item);
    const TrackDataRoute *tdr = dynamic_cast<const TrackDataRoute *>(item);
    const TrackDataFolder *tdf = dynamic_cast<const TrackDataFolder *>(item);

    const TrackDataAbstractPoint *tda = dynamic_cast<const TrackDataAbstractPoint *>(item);
    const TrackDataTrackpoint *tdp = dynamic_cast<const TrackDataTrackpoint *>(item);
    const TrackDataWaypoint *tdw = dynamic_cast<const TrackDataWaypoint *>(item);

    // Output queues for each tag type.  This one if for is top level
    // items which appear immediately under the element tag.
    TagQueue toplevelQueue;
    // Items which appear within <extensions>
    TagQueue extensionsQueue;
    // Items which appear also inside <gpxx:WaypointExtensions> and <gpxx:Categories>
    TagQueue categoriesQueue;
    // Items which appear also inside <gpxx:WaypointExtensions> and <gpxx:Address>
    TagQueue addressQueue;

    // Write out the appropriate element start tag, and any attributes
    // belonging to that.  Add any other data, including the item name,
    // to the appropriate queue.
    if (tdt!=nullptr)					// element TRK
    {
        str.writeCharacters("\n\n  ");
        str.writeStartElement("trk");
        if (!itemName.isEmpty()) toplevelQueue.enqueue("name", itemName);
    }
    else if (tdr!=nullptr)				// element RTE
    {
        str.writeCharacters("\n\n  ");
        str.writeStartElement("rte");
        if (!itemName.isEmpty()) toplevelQueue.enqueue("name", itemName);
    }
    else if (tds!=nullptr)				// element TRKSEG
    {
        str.writeStartElement("trkseg");
        if (!itemName.isEmpty()) extensionsQueue.enqueue("name", itemName);
    }
    else if (tda!=nullptr)				// element TRKPT, WPT or RTEPT
    {
        if (tdp!=nullptr)				// element TRKPT
        {
            str.writeStartElement("trkpt");
        }
        else if (tdw!=nullptr)				// element WPT
        {
            const QString &homeName = options().homePoint();
            if (newName.isEmpty() && !homeName.isEmpty() && homeName==tdw->name())
            {
                qDebug() << "identified Home point" << tdw->name();
                mHomePoint = tdw;
            }

            const QString &workName = options().workPoint();
            if (newName.isEmpty() && !workName.isEmpty() && workName==tdw->name())
            {
                qDebug() << "identified Work point" << tdw->name();
                mWorkPoint = tdw;
            }

            str.writeCharacters("\n\n  ");
            str.writeStartElement("wpt");
        }
        else						// element RTEPT
        {
            str.writeCharacters("\n\n    ");
            str.writeStartElement("rtept");
        }

        // lat="latitudeType"
        // lon="longitudeType"
        str.writeAttribute("lat", QString::number(tda->latitude(), 'f'));
        str.writeAttribute("lon", QString::number(tda->longitude(), 'f'));

        // <name> xsd:string </name>
        if (!itemName.isEmpty()) toplevelQueue.enqueue("name", itemName);
    }
    else if (tdf!=nullptr)				// Folder
    {							// write nothing, but recurse for children
    }
    else						// anything else
    {
        qWarning() << "unknown item type" << item << item->name();
        return (true);					// warning only, don't abort
    }

#ifdef EXTENSIONS_AFTER_CHILDREN
    // Strictly according to the GPX specification, an item's children
    // should be output before its extensions.  However, in the interests
    // of clarity, unless EXTENSIONS_AFTER_CHILDREN is defined the extensions
    // are output first and then the children follow.
    writeChildren(item, str);
#endif

    // The colour explicitly set in item metadata.
    QColor explicitColour;
    // The colour resolved from an item category.
    QColor categoryColour;

    // Look at the item metadata, ignoring any which is not to be exported.
    // Format each of the other values and add it to the appropriate queue.
    for (int idx = 0; idx<DataIndexer::count(); ++idx)
    {
        const QByteArray name = DataIndexer::name(idx);

        // Always ignore tags which are only used internally.
        if (DataIndexer::isInternalTag(name)) continue;

        // Get the item metadata value, and ignore it if the value is null.
        const QVariant &v = item->metadata(idx);
        if (v.isNull()) continue;

        // A folder is not written as an explicit GPX file element,
        // therefore its metadata cannot be written either.  Ignore
        // any folder metadata set for it, and warn if there is any
        // that we are not expecting.  The only folder metadata that
        // we set is "creator" by AddContainerCommand, so this is
        // simply ignored with no message.
        if (tdf!=nullptr)
        {
            if (name!="creator") qWarning() << "Unexpected metadata" << name << "for folder" << item->name();
            continue;
        }

        // The queue that will be used to store the tags and values,
        // except in the category and address special cases.
        TagQueue &toQueue = (isExtensionTag(item, name) ? extensionsQueue : toplevelQueue);

        // Our internal LINECOLOR/POINTCOLOR data is namespaced and only for
        // our own purposes.  The COLOR attribute of the item is also set
        // for use by other GPX applications.
        if (name=="linecolor" || name =="pointcolor")
        {
            const QColor col = v.value<QColor>();
            // An alpha value not 255 means this item has no colour (inherit).
            // See TrackItemStylePage and FilesController::slotTrackProperties().
            // However, we write out the value (with the alpha) so that the
            // colour value will be retained.
            //if (col.alpha()!=255) continue;
            // Note the colour and that an explicit colour has been set.
            explicitColour = col;

            // Save the explicit item colour unconditionally.  Use this form
            // of QColor::name() to ensure that the alpha is included in
            // the string value if it is not "full".  This does not appear to
            // be necessary for the colour display (via MetadataModel), this
            // automatically formats ARGB if necessary.
            toQueue.enqueue(name, col.name(col.alpha()==255 ? QColor::HexRgb : QColor::HexArgb));
        }
        else if (name=="color")
        {
            // Do not do anything for now, maybe will output this below
            // if there is no explicit colour as above.
            continue;
        }
        else if (name=="category")			// category, may be multiple
        {
            const QStringList cats = v.toStringList();

            // OsmAnd+ does not understand the official extension <gpxx:Category>
            // tag, but does accept a single category in a <category> tag (for older
            // versions) or a <type> tag (currently).  Originally, in order to not
            // lose information, we wrote this as a comma separated string of all
            // the categories in alphabetical order, but with "Address Book" (the
            // Garmin default category) eliminated if it was present.
            // See http://comments.gmane.org/gmane.comp.gis.openstreetmap.osmand/949
            //
            // However, this causes duplication of categories in OsmAnd+ and
            // potentially needing to search through multiple folders to find a
            // point.  Trying an alternative strategy here, where there is a
            // distinction between "primary" and "secondary" categories - internally,
            // the primary one is always the first in the list - and only the
            // "primary" is written out for OsmAnd+ in the <category> and <type> tags.
            QStringList cats2 = cats;			// copy the original list
            cats2.removeAll("Address Book");		// remove Garmin default category
            if (!cats2.isEmpty())			// more categories remain
            {
                const QString &primaryCategory = cats2.first();
                // <category>Shopping</category>
                toQueue.enqueue(name, primaryCategory);
                // <type>Shopping</type>
                toQueue.enqueue("type", primaryCategory);

                // Get the colour defined for the primary category,
                // which will be output later if no explicit colour
                // is defined.
                if (mCategoriesList!=nullptr) categoryColour = mCategoriesList->category(primaryCategory).colour();
            }

            // For Garmin, the full list of categories is written out inside
            // the <gpxx:WaypointExtensions> block, along with the address.
            for (const QString &cat : cats) categoriesQueue.enqueue("gpxx:Category", cat);
        }
        else if (isAddressTag(name))
        {
            // For Garmin, the address is written out inside the
            // <gpxx:WaypointExtensions> block, along with the categories.
            addressQueue.enqueue(name, v.toString());
        }
        else if (name=="flags")				// waypoint flags,
        {						// only if not zero
            if (v.toInt()!=0) toQueue.enqueue(name, valueString(v));
        }
        else						// any other tag
        {
            toQueue.enqueue(name, valueString(v));
        }
    }

    // If this item is being renamed - that is, it is the copy of either
    // the "Home" or "Work" points - then record the original point.
    if (!newName.isEmpty() && item->hasExplicitName()) extensionsQueue.enqueue("source", item->name());

    // Ensure that the containing folder path for a waypoint is
    // written out.  It is not stored as a property of the waypoint,
    // so generate it here.
    if (tdw!=nullptr)
    {
        const TrackDataFolder *fold = dynamic_cast<TrackDataFolder *>(tdw->parent());
        if (fold!=nullptr)				// within a folder?
        {						// note the folder path
            extensionsQueue.enqueue(DataIndexer::nameWithNamespace("folder"), fold->path());
        }
    }

    // All of the item data has been added to the appropriate queue.
    //
    // Now resolve the final point or line colour - either one that
    // has been explicitly set, or the category colour if there is one.
    if (dynamic_cast<const TrackDataFile *>(item)==nullptr)
    {							// but no COLOR at top level
        TagQueue &toQueue = (isExtensionTag(item, "color") ? extensionsQueue : toplevelQueue);

        QColor col = explicitColour;
        if (!col.isValid()) col = categoryColour;
        if (col.isValid() && col.alpha()==255)
        {
            // NavMarks applied a workaround for OsmAnd+ (as of version 2.0.4)
            // which seemed to have a problem managing colours set for waypoints.
            // If the colour specification (in the form #RRGGBB) had a small RR
            // component then it was saved to the favourites.gpx file without
            // leading zeros, and then failed to read back in.  To get around this,
            // if the colour set here only has a small red component then we force
            // a dummy component of 10 (hex), which should have minimal effect on
            // the displayed colour.
            //
            // Reported as https://github.com/osmandapp/Osmand/issues/1321, and
            // now appears to be resolved.
            //if (col.red()<0x10) col.setRed(0x10);

            const QString data = col.name();		// in format "#rrggbb"
            if (tda!=nullptr)				// a point element
            {
                // OsmAnd: <color>#c0c0c0</color>
                toQueue.enqueue("color", data);
            }
            else					// some other container element
            {
                // GPX: <topografix:color>c0c0c0</topografix:color>
                toQueue.enqueue("topografix:color", data.mid(1));
            }
        }
    }

    // Look at each of the queues and see which ones need to be processed
    // and output.  The top level queue is assumed to always need to be
    // output, but nothing (and no enclosing elements) will be written out
    // if it really is empty.
    const bool haveExtensions = !extensionsQueue.isEmpty();
    const bool haveCategories = !categoriesQueue.isEmpty();
    const bool haveAddress = !addressQueue.isEmpty();

    // The top level entries
    writeQueue(&toplevelQueue, str);

    // Extensions block
    if (haveExtensions || haveCategories || haveAddress)
    {
        str.writeStartElement("extensions");

        // The extensions entries
        writeQueue(&extensionsQueue, str);

        // Garmin waypoint extensions block
        if (haveCategories || haveAddress)
        {
            str.writeStartElement("gpxx:WaypointExtension");

            // Categories
            if (haveCategories)
            {
                str.writeStartElement("gpxx:Categories");
                writeQueue(&categoriesQueue, str);
                str.writeEndElement();			// </gpxx:Categories>
            }

            // Address
            if (haveAddress)
            {
                str.writeStartElement("gpxx:Address");
                writeQueue(&addressQueue, str);
                str.writeEndElement();			// </gpxx:Address>
            }

            str.writeEndElement();			// </gpxx:WaypointExtension>
        }

        str.writeEndElement();				// </extensions>
    }

#ifndef EXTENSIONS_AFTER_CHILDREN
    // Finally write out the item's children, if any.
    writeChildren(item, str);
#endif

    // And at last the element end tag.
    if (tdf==nullptr) str.writeEndElement();		// nothing was started for folder

    return (true);
}


bool GpxExporter::saveTo(QIODevice *dev, const TrackDataFile *item)
{
    qDebug() << "item" << item->name();
    qDebug() << "home" << options().homePoint() << "work" << options().workPoint();

    mHomePoint = nullptr;				// points not found yet
    mWorkPoint = nullptr;

    QXmlStreamWriter str(dev);
    str.setAutoFormatting(true);
    str.setAutoFormattingIndent(2);

    // xml write
    str.writeStartDocument("1.0", false);

    // <gpx>
    str.writeStartElement("gpx");
    str.writeAttribute("version", "1.1");
    str.writeAttribute("creator", item->metadata("creator").toString());
    str.writeAttribute("xmlns", "http://www.topografix.com/GPX/1/1");
    str.writeNamespace("http://www.garmin.com/xmlschemas/GpxExtensions/v3", "gpxx");
    str.writeNamespace("http://www.garmin.com/xmlschemas/TrackPointExtension/v1", "gpxtpx");
    str.writeNamespace("http://www.w3.org/2001/XMLSchema-instance", "xsi");
    // OsmAnd+ namespace may be needed for the category map
    str.writeNamespace("https://osmand.net", "osmand");
    // our own extensions
    str.writeNamespace(("http://www.keelhaul.me.uk/" PROJECT_NAME), DataIndexer::applicationNamespace());
    // namespace URI from https://code.google.com/p/mytracks/issues/detail?id=276
    str.writeNamespace("http://www.topografix.com/GPX/gpx_style/0/2", "topografix");
    // any other namespaces seen in files
    const QList<QByteArray> &namespaces = DataIndexer::namespacesWithUri();
    for (const QByteArray &nsp : namespaces)
    {
        if (nsp==DataIndexer::applicationNamespace()) continue;	// already added above
        if (nsp=="topografix") continue;			// already added above
        if (nsp=="osmand") continue;				// already added above
        if (nsp=="gpxtpx") continue;				// already added above

        str.writeNamespace(DataIndexer::uriForNamespace(nsp), nsp);
    }
    str.writeCharacters("\n\n  ");

    // File <metadata> to be written out.  There is no complication with
    // extensions or namespace here, so a simple loop will do.
    str.writeStartElement("metadata");
    for (int idx = 0; idx<DataIndexer::count(); ++idx)
    {
        const QByteArray name = DataIndexer::name(idx);
        if (DataIndexer::isInternalTag(name)) continue;
        const QVariant &v = item->metadata(idx);
        if (v.isNull()) continue;
        // <link href="http://www.garmin.com"><text>Garmin International</text></link>
        // <time>2011-11-29T14:39:05Z</time>
        writeValue(name, valueString(v), str);
    }
    str.writeEndElement();				// </metadata>

    // File <extensions>, category list if present - as appropriate for
    // either a save or an export.
    mCategoriesList = item->categories();
    if (mCategoriesList!=nullptr && mCategoriesList->count()>0)
    {
        str.writeCharacters("\n\n  ");
        str.writeStartElement("extensions");

        if (options().hasFlag(ImporterExporterOptions::ImportExport))
        {
            // The OsmAnd POINTS_GROUPS/GROUP format
            str.writeStartElement("osmand:points_groups");
        }
        else
        {
            // The original CATMAP/CATENTRY format (with OsmAnd extensions)
            str.writeStartElement(DataIndexer::applicationNamespace()+":catmap");
        }

        const QStringList catNames = mCategoriesList->allNames();
        for (const QString &name : catNames)
        {
            if (options().hasFlag(ImporterExporterOptions::ImportExport)) str.writeEmptyElement("osmand:group");
            else str.writeEmptyElement(DataIndexer::applicationNamespace()+":catentry");
            str.writeAttribute("name", name);

            const CategoryData &cat = mCategoriesList->category(name);
            const QColor col = cat.colour();
            if (col.isValid()) str.writeAttribute("color", col.name());
            const QString icon = cat.icon();
            if (!icon.isEmpty()) str.writeAttribute("icon", icon);
            const QString shape = cat.shape();
            if (!shape.isEmpty()) str.writeAttribute("background", shape);
        }

        str.writeEndElement();				// </catmap> or </points_groups>
        str.writeEndElement();				// </extensions>
    }

    writeChildren(item, str);				// write out child elements

    // Write out a copy of the "Home" and "Work" waypoints, if they
    // were requested and have been found.
    if (mHomePoint!=nullptr)
    {
        str.writeCharacters("\n\n");
        writeItem(mHomePoint, str, "home");
    }
    if (mWorkPoint!=nullptr)
    {
        str.writeCharacters("\n\n");
        writeItem(mWorkPoint, str, "work");
    }

    str.writeCharacters("\n\n");
    str.writeEndElement();				// </gpx>
    str.writeEndDocument();

    if (str.hasError())
    {
        qDebug() << "XML writing failed!";
        return (false);
    }

    return (true);
}


/* static */ QString GpxExporter::filter()
{
    return ("GPX files (*.gpx)");
}
