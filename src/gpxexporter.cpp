
// http://www.topografix.com/GPX/1/1/

#include "gpxexporter.h"

#include <errno.h>
#include <string.h>

#include <qfile.h>
#include <qdatetime.h>
#include <qdebug.h>

#include <QXmlStreamWriter>

#include <klocalizedstring.h>

#include "trackdata.h"
#include "style.h"
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


GpxExporter::~GpxExporter()
{
    qDebug() << "done";
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


static QString colourValue(const QColor &col)
{
    QString result = col.name();
    if (result.startsWith('#')) result.remove(0, 1);
    return (result);
}


static void writeStyle(const TrackDataItem *item, QXmlStreamWriter &str)
{
    const Style *s = item->style();
    if (s->isEmpty()) return;

    if (s->hasLineColour())				// for a track element
    {
        // GPX: <topografix:color>c0c0c0</topografix:color>
        startExtensions(str);
        str.writeTextElement("topografix:color", colourValue(s->lineColour()));
    }
    else if (s->hasPointColour())			// for a waypoint
    {
        // OsmAnd: <color>#c0c0c0</color>
        startExtensions(str);
        str.writeTextElement("color", '#'+colourValue(s->pointColour()));
    }
}



static bool isExtensionTag(const TrackDataItem *item, const QString &name)
{
    if (dynamic_cast<const TrackDataAbstractPoint *>(item)!=NULL)
    {							// point - these not in extensions
        return (!(name=="name" || name=="ele" || name=="time" || name=="hdop"));
    }
    else if (dynamic_cast<const TrackDataTrack *>(item)!=NULL)
    {							// track - these not in extensions
        return (!(name=="name" || name=="desc" || name=="type"));
    }
    else if (dynamic_cast<const TrackDataRoute *>(item)!=NULL)
    {							// route - these not in extensions
        return (!(name=="name" || name=="desc" || name=="type"));
    }
    else if (dynamic_cast<const TrackDataSegment *>(item)!=NULL)
    {							// segment - all in extensions
        return (true);
    }
    else if (dynamic_cast<const TrackDataWaypoint *>(item)!=NULL)
    {							// waypoint - these not in extensions
        return (!(name=="link"));
    }
    else return (false);				// metadata - no extensions
}



static bool isNamespacedTag(const QString &name)
{
    return (name=="status" || name=="source" || name=="stop" || name=="bearingline");
}



static bool isInternalTag(const QString &name)
{
    return (name=="linecolour" || name=="pointcolour");
}


static void writeMetadata(const TrackDataItem *item, QXmlStreamWriter &str, bool wantExtensions)
{
    for (int idx = 0; idx<DataIndexer::self()->count(); ++idx)
    {
        //qDebug() << "metadata" << idx
        //         << "name" << DataIndexer::self()->name(idx)
        //         << "=" << item->metadata(idx);

        QString name = DataIndexer::self()->name(idx);
        if (isInternalTag(name)) continue;		// internal to application only
        if (isExtensionTag(item, name) ^ wantExtensions) continue;
						        // not matching extension option
        QString data = item->metadata(idx);
        if (data.isEmpty()) continue;			// no data to output

        if (wantExtensions) startExtensions(str);

        if (name=="link")				// special format for this
        {
            str.writeEmptyElement(name);
            str.writeAttribute("link", data);
        }
        else
        {
            if (isNamespacedTag(name)) name = "navtracks:"+name;
            str.writeTextElement(name, data);
        }
    }
}



static bool writeItem(const TrackDataItem *item, QXmlStreamWriter &str);



static inline bool writeChildren(const TrackDataItem *item, QXmlStreamWriter &str)
{
    int num = item->childCount();
    for (int i = 0; i<num; ++i)
    {
        if (!writeItem(item->childAt(i), str)) return (false);
    }

    return (true);
}







static bool writeItem(const TrackDataItem *item, QXmlStreamWriter &str)
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

    // start tag
    if (tdt!=NULL)					// element TRK
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
    else if (tdr!=NULL)					// element RTE
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
    else if (tds!=NULL)					// element TRKSEG
    {
        str.writeStartElement("trkseg");
        writeMetadata(tds, str, false);
    }
    else if (tdp!=NULL || tdw!=NULL || tdm!=NULL)	// element TRKPT or WPT
    {
        const TrackDataAbstractPoint *p;
        if (tdp!=NULL)					// element TRKPT
        {
            p = tdp;
            str.writeStartElement("trkpt");
        }
        else if (tdw!=NULL)				// element WPT
        {
            p = tdw;
            str.writeCharacters("\n\n  ");
            str.writeStartElement("wpt");
        }
        else						// element RTEPT
        {
            p = tdm;
            str.writeCharacters("\n\n    ");
            str.writeStartElement("rtept");
        }

        // lat="latitudeType"
        // lon="longitudeType"
        str.writeAttribute("lat", QString::number(p->latitude(), 'f'));
        str.writeAttribute("lon", QString::number(p->longitude(), 'f'));

        // <ele> xsd:decimal </ele>
        double ele = p->elevation();
        if (!ISNAN(ele)) str.writeTextElement("ele", QString::number(ele, 'f', 3));

        // <time> xsd:dateTime </time>
        QDateTime dt = p->time();
        if (dt.isValid()) str.writeTextElement("time", dt.toString(Qt::ISODate));

        // <name> xsd:string </name>
        if (item->hasExplicitName()) str.writeTextElement("name", p->name());
        // <cmt> xsd:string </cmt>
        // <desc> xsd:string </desc>
        // <sym> xsd:string </sym>

        // <hdop> xsd:decimal </hdop>
        writeMetadata(p, str, false);
    }
    else if (tdf!=NULL)					// Folder
    {							// write nothing, but recurse for children
    }
    else						// anything else
    {
        qDebug() << "unknown item type" << item << item->name();
        return (true);					// warning only, don't abort
    }

#ifdef EXTENSIONS_AFTER_CHILDREN
    writeChildren(item, str);				// write child items
#endif

    // <extensions> extensionsType </extensions>
    if (tdt!=NULL)					// extensions for TRK
    {
        writeMetadata(tdt, str, true);
        writeStyle(tdt, str);
    }
    else if (tdr!=NULL)					// extensions for RTE
    {
        writeMetadata(tdr, str, true);
        writeStyle(tdr, str);
    }
    else if (tds!=NULL)					// extensions for TRKSEG
    {
        // <name> xsd:string </name>
        startExtensions(str);
        if (item->hasExplicitName()) str.writeTextElement("name", tds->name());
        writeMetadata(tds, str, true);
        writeStyle(tds, str);
    }
    else if (tdp!=NULL)					// extensions for TRKPT
    {
        writeMetadata(tdp, str, true);
        writeStyle(tdp, str);
    }
    else if (tdw!=NULL)					// extensions for WPT
    {
        writeStyle(tdw, str);
        writeMetadata(tdw, str, true);
        const TrackDataFolder *fold = dynamic_cast<TrackDataFolder *>(tdw->parent());
        if (fold!=NULL)					// within a folder?
        {						// save the folder path
            startExtensions(str);
            str.writeTextElement("navtracks:folder", fold->path());
        }
    }

    endExtensions(str);

#ifndef EXTENSIONS_AFTER_CHILDREN
    writeChildren(item, str);				// write child items
#endif

    // end tag
    if (tdf==NULL) str.writeEndElement();		// nothing was started for this
    return (status);
}








bool GpxExporter::save(const QUrl &file, const TrackDataFile *item)
{
    qDebug() << "item" << item->name() << "to" << file;

    if (!prepareSaveFile(file)) return (false);
    startedExtensions = false;

    QXmlStreamWriter str(&mSaveFile);
    str.setAutoFormatting(true);
    str.setAutoFormattingIndent(2);

    // xml write
    str.writeStartDocument("1.0", false);

    // <gpx>
    str.writeStartElement("gpx");
    str.writeAttribute("version", "1.1");
    str.writeAttribute("creator", item->metadata("creator"));
    str.writeAttribute("xmlns", "http://www.topografix.com/GPX/1/1");
    str.writeNamespace("http://www.garmin.com/xmlschemas/GpxExtensions/v3", "gpxx");
    str.writeNamespace("http://www.garmin.com/xmlschemas/TrackPointExtension/v1", "gpxtpx");
    str.writeNamespace("http://www.w3.org/2001/XMLSchema-instance", "xsi");
    // our own extensions
    str.writeNamespace("http://www.keelhaul.me.uk/navtracks", "navtracks");
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
        mSaveFile.cancelWriting();
        return (false);
    }

    if (!mSaveFile.commit())
    {
        reporter()->setError(ErrorReporter::Fatal, mSaveFile.errorString());
        return (false);
    }

    return (true);
}


QString GpxExporter::filter()
{
    return ("GPX files (*.gpx)");
}
