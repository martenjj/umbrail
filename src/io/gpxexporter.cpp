
// http://www.topografix.com/GPX/1/1/

#include "gpxexporter.h"

#include <errno.h>
#include <string.h>

#include <qfile.h>
#include <qdatetime.h>
#include <qcolor.h>
#include <qdebug.h>

#include <QXmlStreamWriter>

#include <klocalizedstring.h>

#include "trackdata.h"
#include "dataindexer.h"
#include "errorreporter.h"



// According to the GPX specification, extensions must appear after
// any children of an element.  However, the output GPX file is more
// readable if they appear before.  Define this symbol to have them
// appear after the children.
#undef EXTENSIONS_AFTER_CHILDREN



GpxExporter::GpxExporter()
    : ExporterBase()
{
    qDebug();
}


static bool startedExtensions = false;



static void startExtensions(QXmlStreamWriter &str)
{
    if (startedExtensions) return;			// already started
    str.writeStartElement("extensions");
    startedExtensions = true;
}


static void endExtensions(QXmlStreamWriter &str)
{
    if (!startedExtensions) return;			// not started
    str.writeEndElement();
    startedExtensions = false;
}


static bool isExtensionTag(const TrackDataItem *item, const QByteArray &name)
{
    if (dynamic_cast<const TrackDataFile *>(item)!=nullptr) return (false);
							// file metadata - never in extensions
    if (DataIndexer::isApplicationTag(name)) return (true);
							// application tag - always in extensions
    if (dynamic_cast<const TrackDataAbstractPoint *>(item)!=nullptr)
    {							// point - these not in extensions
        return (!(name=="name" || name=="ele" || name=="time" || name=="hdop"));
    }
    else if (dynamic_cast<const TrackDataTrack *>(item)!=nullptr)
    {							// track - these not in extensions
        return (!(name=="name" || name=="desc" || name=="type"));
    }
    else if (dynamic_cast<const TrackDataRoute *>(item)!=nullptr)
    {							// route - these not in extensions
        return (!(name=="name" || name=="desc" || name=="type"));
    }
    else if (dynamic_cast<const TrackDataSegment *>(item)!=nullptr)
    {							// segment - all in extensions
        return (true);
    }
    else if (dynamic_cast<const TrackDataWaypoint *>(item)!=nullptr)
    {							// waypoint - these not in extensions
        return (!(name=="link"));
    }
    else return (false);				// other - assume not in extensions
}


static void writeMetadata(const TrackDataItem *item, QXmlStreamWriter &str, bool wantExtensions)
{
    for (int idx = 0; idx<DataIndexer::count(); ++idx)
    {
        //qDebug() << "metadata" << idx
        //         << "name" << DataIndexer::name(idx)
        //         << "=" << item->metadata(idx);

        const QByteArray name = DataIndexer::name(idx);
        if (DataIndexer::isInternalTag(name)) continue;
							// ignore internally used tags
        if (isExtensionTag(item, name) ^ wantExtensions) continue;
							// check matches extension state
        const QVariant &v = item->metadata(idx);	// get metadata from item
        if (v.isNull()) continue;			// no data to output

        if (wantExtensions) startExtensions(str);	// start extensions if needed

        QString data;					// string form to output

        // Our internal LINECOLOR/POINTCOLOR data is namespaced and only for
        // our own purposes.  The COLOR attribute of the item is also set
        // for use by other GPX applications.

        if (name=="linecolor" || name =="pointcolor")	// line or point colour
        {
            const QColor col = v.value<QColor>();
            // An alpha value of 0 means this item has no colour.
            // See TrackItemStylePage and FilesController::slotTrackProperties().
            if (col.alpha()==0) continue;

            data = col.name();				// in format "#rrggbb"
            if (dynamic_cast<const TrackDataFile *>(item)==nullptr)
            {						// no COLOR at top level
                if (dynamic_cast<const TrackDataAbstractPoint *>(item)!=nullptr)
                {					// a point element
                    // OsmAnd: <color>#c0c0c0</color>
                    str.writeTextElement("color", data);
                }
                else					// a container element
                {
                    // GPX: <topografix:color>c0c0c0</topografix:color>
                    str.writeTextElement("topografix:color", data.mid(1));
                }
            }
        }
        else if (name=="time")				// point or file time
        {
            data = v.toDateTime().toString(Qt::ISODate);
        }
        else						// all other items
        {
            data = v.toString();			// default string format
        }

        if (name=="link")				// special format for this
        {
            str.writeEmptyElement(name);
            str.writeAttribute("link", data);
        }
        else
        {
            str.writeTextElement(DataIndexer::nameWithNamespace(name), data);
        }
    }
}



bool GpxExporter::writeChildren(const TrackDataItem *item, QXmlStreamWriter &str) const
{
    int num = item->childCount();
    for (int i = 0; i<num; ++i)
    {
        if (!writeItem(item->childAt(i), str)) return (false);
    }

    return (true);
}







bool GpxExporter::writeItem(const TrackDataItem *item, QXmlStreamWriter &str) const
{
    bool status = true;

    // what sort of element?
    const TrackDataTrack *tdt = dynamic_cast<const TrackDataTrack *>(item);
    const TrackDataRoute *tdr = dynamic_cast<const TrackDataRoute *>(item);
    const TrackDataSegment *tds = dynamic_cast<const TrackDataSegment *>(item);
    const TrackDataTrackpoint *tdp = dynamic_cast<const TrackDataTrackpoint *>(item);
    const TrackDataFolder *tdf = dynamic_cast<const TrackDataFolder *>(item);
    const TrackDataWaypoint *tdw = dynamic_cast<const TrackDataWaypoint *>(item);
    const TrackDataRoutepoint *tdm = dynamic_cast<const TrackDataRoutepoint *>(item);

    const bool isSel = isSelected(item);		// is the item selected?
    if (isSel)
    {
        // start tag
        if (tdt!=nullptr)				// element TRK
        {
            str.writeCharacters("\n\n  ");
            str.writeStartElement("trk");
            // <name> xsd:string </name>
            if (item->hasExplicitName()) str.writeTextElement("name", tdt->name());
            // <desc> xsd:string </desc>
            // <type> xsd:string </type>
            writeMetadata(tdt, str, false);
            // <cmt> xsd:string </cmt>
        }
        else if (tdr!=nullptr)				// element RTE
        {
            str.writeCharacters("\n\n  ");
            str.writeStartElement("rte");
            // <name> xsd:string </name>
            if (item->hasExplicitName()) str.writeTextElement("name", tdr->name());
            // <desc> xsd:string </desc>
            // <type> xsd:string </type>
            writeMetadata(tdr, str, false);
            // <cmt> xsd:string </cmt>
        }
        else if (tds!=nullptr)				// element TRKSEG
        {
            str.writeStartElement("trkseg");
            writeMetadata(tds, str, false);
        }
        else if (tdp!=nullptr || tdw!=nullptr || tdm!=nullptr)
        {						// element TRKPT, WPT or RTEPT
            const TrackDataAbstractPoint *p;
            if (tdp!=nullptr)				// element TRKPT
            {
                p = tdp;
                str.writeStartElement("trkpt");
            }
            else if (tdw!=nullptr)			// element WPT
            {
                p = tdw;
                str.writeCharacters("\n\n  ");
                str.writeStartElement("wpt");
            }
            else					// element RTEPT
            {
                p = tdm;
                str.writeCharacters("\n\n    ");
                str.writeStartElement("rtept");
            }

            // lat="latitudeType"
            // lon="longitudeType"
            str.writeAttribute("lat", QString::number(p->latitude(), 'f'));
            str.writeAttribute("lon", QString::number(p->longitude(), 'f'));

            // <name> xsd:string </name>
            if (item->hasExplicitName()) str.writeTextElement("name", p->name());
            // <cmt> xsd:string </cmt>
            // <desc> xsd:string </desc>
            // <sym> xsd:string </sym>

            // <hdop> xsd:decimal </hdop>
            writeMetadata(p, str, false);
        }
        else if (tdf!=nullptr)				// Folder
        {						// write nothing, but recurse for children
        }
        else						// anything else
        {
            qDebug() << "unknown item type" << item << item->name();
            return (true);				// warning only, don't abort
        }
    }

#ifdef EXTENSIONS_AFTER_CHILDREN
    writeChildren(item, str);				// write child items
#endif

    if (isSel)
    {
        // <extensions> extensionsType </extensions>
        if (tdt!=nullptr)				// extensions for TRK
        {
            writeMetadata(tdt, str, true);
        }
        else if (tdr!=nullptr)				// extensions for RTE
        {
            writeMetadata(tdr, str, true);
        }
        else if (tds!=nullptr)				// extensions for TRKSEG
        {
            // <name> xsd:string </name>
            startExtensions(str);
            if (item->hasExplicitName()) str.writeTextElement("name", tds->name());
            writeMetadata(tds, str, true);
        }
        else if (tdp!=nullptr)				// extensions for TRKPT
        {
            writeMetadata(tdp, str, true);
        }
        else if (tdw!=nullptr)				// extensions for WPT
        {
            writeMetadata(tdw, str, true);
            const TrackDataFolder *fold = dynamic_cast<TrackDataFolder *>(tdw->parent());
            if (fold!=nullptr)				// within a folder?
            {						// save the folder path
                startExtensions(str);
                str.writeTextElement(DataIndexer::nameWithNamespace("folder"), fold->path());
            }
        }

        endExtensions(str);
    }

#ifndef EXTENSIONS_AFTER_CHILDREN
    writeChildren(item, str);				// write child items
#endif

    if (isSel)
    {
        // end tag
        if (tdf==nullptr) str.writeEndElement();	// nothing was started for this
    }

    return (status);
}








bool GpxExporter::saveTo(QIODevice *dev, const TrackDataFile *item)
{
    qDebug() << "item" << item->name() << "to" << dev;

    startedExtensions = false;

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
    // our own extensions
    str.writeNamespace("http://www.keelhaul.me.uk/navtracks", DataIndexer::applicationNamespace());
    // namespace URI from https://code.google.com/p/mytracks/issues/detail?id=276
    str.writeNamespace("http://www.topografix.com/GPX/gpx_style/0/2", "topografix");
    str.writeCharacters("\n\n  ");

    // <metadata>
    str.writeStartElement("metadata");
    writeMetadata(item, str, false);
//    // <link href="http://www.garmin.com"><text>Garmin International</text></link>
//    str.writeStartElement("link");
//    str.writeAttribute("href", "http://www.garmin.com");
//    str.writeTextElement("text", "Garmin International");
//    str.writeEndElement();				// </link>
    // <time>2011-11-29T14:39:05Z</time>
    str.writeEndElement();				// </metadata>

    int num = item->childCount();			// write out child elements
    for (int i = 0; i<num; ++i)
    {
        if (!writeItem(item->childAt(i), str)) break;
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


QString GpxExporter::filter()
{
    return ("GPX files (*.gpx)");
}
