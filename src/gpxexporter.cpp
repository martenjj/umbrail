
// http://www.topografix.com/GPX/1/1/

#include "gpxexporter.h"

#include <errno.h>
#include <string.h>
#include <math.h>

#include <qfile.h>
#include <qdatetime.h>

#include <QXmlStreamWriter>

#include <kdebug.h>
#include <klocale.h>

#include "trackdata.h"
#include "style.h"
#include "dataindexer.h"



// According to the GPX specification, extensions must appear after
// any children of an element.  However, the output GPX file is more
// readable if they appear before.  Define this symbol to have them
// appear after the children.
#undef EXTENSIONS_AFTER_CHILDREN



GpxExporter::GpxExporter()
    : ExporterBase()
{
    kDebug();
}


GpxExporter::~GpxExporter()
{
    kDebug() << "done";
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



static void writeStyle(const TrackDataDisplayable *item, QXmlStreamWriter &str)
{
    const Style *s = item->style();
    if (s->isEmpty()) return;

    if (s->hasLineColour())
    {
        // <topografix:color>c0c0c0</topografix:color>
        startExtensions(str);
        str.writeTextElement("topografix:color",
                             QString("%1").arg(QString::number(s->lineColour().rgb() & 0x00FFFFFF, 16), 6, QChar('0')));
    }
}



static bool shouldBeInExtensions(const TrackDataDisplayable *item, const QString &name)
{
    if (dynamic_cast<const TrackDataPoint *>(item)!=NULL)
    {							// point - these not in extensions
        return (!(name=="name" || name=="ele" || name=="time" || name=="hdop"));
    }
    else if (dynamic_cast<const TrackDataTrack *>(item)!=NULL)
    {							// track - these not in extensions
        return (!(name=="name" || name=="desc" || name=="type"));
    }
    else if (dynamic_cast<const TrackDataSegment *>(item)!=NULL)
    {							// segment - all in extensions
        return (true);
    }
    else return (false);				// metadata - no extensions
}



static void writeMetadata(const TrackDataDisplayable *item, QXmlStreamWriter &str, bool wantExtensions)
{
    for (int idx = 0; idx<DataIndexer::self()->count(); ++idx)
    {
        //kDebug() << "metadata" << idx
        //         << "name" << DataIndexer::self()->name(idx)
        //         << "=" << item->metadata(idx);

        QString name = DataIndexer::self()->name(idx);
        if (shouldBeInExtensions(item, name) ^ wantExtensions) continue;
        QString data =  item->metadata(idx);
        if (data.isEmpty()) continue;

        if (wantExtensions) startExtensions(str);
        str.writeTextElement(name, data);
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
    const TrackDataSegment *tds = dynamic_cast<const TrackDataSegment *>(item);
    const TrackDataPoint *tdp = dynamic_cast<const TrackDataPoint *>(item);

    // start tag
    if (tdt!=NULL)
    {
        str.writeStartElement("trk");
        // <name> xsd:string </name>
        str.writeTextElement("name", tdt->name());
        // <desc> xsd:string </desc>
        // <type> xsd:string </type>
        writeMetadata(tdt, str, false);
        // <cmt> xsd:string </cmt>
    }
    else if (tds!=NULL)
    {
        str.writeStartElement("trkseg");
        writeMetadata(tds, str, false);
    }
    else if (tdp!=NULL)
    {
        str.writeStartElement("trkpt");
        // lat="latitudeType"
        // lon="longitudeType"
        str.writeAttribute("lat", QString::number(tdp->latitude(), 'f'));
        str.writeAttribute("lon", QString::number(tdp->longitude(), 'f'));

        // <ele> xsd:decimal </ele>
        double ele = tdp->elevation();
        if (ele!=NAN) str.writeTextElement("ele", QString::number(tdp->elevation(), 'f', 3));

        // <time> xsd:dateTime </time>
        QDateTime dt = tdp->time();
        if (dt.isValid()) str.writeTextElement("time", dt.toString(Qt::ISODate));

        // <name> xsd:string </name>
        str.writeTextElement("name", tdp->name());
        // <cmt> xsd:string </cmt>
        // <desc> xsd:string </desc>
        // <sym> xsd:string </sym>

        // <hdop> xsd:decimal </hdop>
        writeMetadata(tdp, str, false);
    }
    else
    {
        kDebug() << "unknown item type" << item << item->name();
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
    else if (tds!=NULL)					// extensions for TRKSEG
    {
        // <name> xsd:string </name>
        startExtensions(str);
        str.writeTextElement("name", tds->name());
        writeMetadata(tds, str, true);
        writeStyle(tds, str);
    }
    else if (tdp!=NULL)					// extensions for TRKPT
    {
        writeMetadata(tdp, str, true);
        writeStyle(tdp, str);
    }
    endExtensions(str);

#ifndef EXTENSIONS_AFTER_CHILDREN
    writeChildren(item, str);				// write child items
#endif

    // end tag
    str.writeEndElement();
    return (status);
}








bool GpxExporter::save(const KUrl &file, const TrackDataFile *item)
{
    kDebug() << "item" << item->name() << "to" << file;

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
    str.writeAttribute("creator", item->metadata(DataIndexer::self()->index("creator")));
    str.writeAttribute("xmlns", "http://www.topografix.com/GPX/1/1");
    str.writeNamespace("http://www.garmin.com/xmlschemas/GpxExtensions/v3", "gpxx");
    str.writeNamespace("http://www.garmin.com/xmlschemas/TrackPointExtension/v1", "gpxtpx");
    str.writeNamespace("http://www.w3.org/2001/XMLSchema-instance", "xsi");
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

//    // Waypoints
//    for (int i = 0; i<cnt; ++i)
//    {
//        str.writeCharacters("\n\n  ");
//        str.writeComment(QString(" Point %1 of %2 ").arg(i+1).arg(cnt));
//        const PointData *pnt = model->pointAt(i);
//        if (!pnt->isValid())
//        {
//            str.writeComment(" (not valid) ");
//            continue;
//        }
//
//        if (pnt->flags() & PointData::NoExport)		// ignore this point
//        {
//            str.writeComment(QString(" %1 ").arg(pnt->name()));
//            str.writeComment(" (not exported) ");
//            continue;
//        }
//
//        // <wpt lat="51.307657" lon="-0.768199">
//        str.writeStartElement("wpt");
//        str.writeAttribute("lat", QString::number(pnt->latitude(), 'f', 6));
//        str.writeAttribute("lon", QString::number(pnt->longtitude(), 'f', 6));
//
//        // <ele>66.22</ele>
//        double ele = pnt->elevation();
//        if (!isnan(ele)) str.writeTextElement("ele", QString::number(ele, 'f', 2));
//
//        // <name>11 Carmarthen Close</name>
//        str.writeTextElement("name", pnt->name());
//        // <link href="Media/icons/002.png"></link>
//        // if a photo is assigned via the GPS's GUI,
//        // but doesn't seem to work if set in this file...
//
//        // <sym>Residence</sym>
//        str.writeTextElement("sym", pnt->symbol());
//
//        const QStringList *cats = pnt->categories();
//        const QStringList *addr = pnt->addresses();
//        if (!cats->isEmpty() || !addr->isEmpty())	// are there any extensions?
//        {
//            // <extensions>
//            str.writeStartElement("extensions");
//            // <gpxx:WaypointExtension>
//            str.writeStartElement("gpxx:WaypointExtension");
//
//            if (!cats->isEmpty())			// are there any categories?
//            {
//                // <gpxx:Categories>
//                str.writeStartElement("gpxx:Categories");
//
//                for (QStringList::const_iterator it = cats->constBegin();
//                     it!=cats->constEnd(); ++it)
//                {
//                    QString cat = (*it);
//                    // <gpxx:Category>Address Book</gpxx:Category>
//                    str.writeTextElement("gpxx:Category", cat);
//                }
//
//                str.writeEndElement();			// </gpxx:Categories>
//            }
//
//            if (!addr->isEmpty())			// is there an address?
//            {
//                // <gpxx:Address>
//                str.writeStartElement("gpxx:Address");
//
//                // <gpxx:StreetAddress>Eisenbahnstrasse 52</gpxx:StreetAddress>
//                QString a = addr->value(PointData::AddressStreet);
//                if (!a.isEmpty()) str.writeTextElement("gpxx:StreetAddress", a);
//                // <gpxx:City>Hausach</gpxx:City>
//                a = addr->value(PointData::AddressCity);
//                if (!a.isEmpty()) str.writeTextElement("gpxx:City", a);
//                // <gpxx:State>Ortenaukreis</gpxx:State>
//                a = addr->value(PointData::AddressState);
//                if (!a.isEmpty()) str.writeTextElement("gpxx:State", a);
//                // <gpxx:Country>DEU</gpxx:Country>
//                a = addr->value(PointData::AddressCountry);
//                if (!a.isEmpty()) str.writeTextElement("gpxx:Country", a);
//                // <gpxx:PostalCode>77756</gpxx:PostalCode>
//                a = addr->value(PointData::AddressPostCode);
//                if (!a.isEmpty()) str.writeTextElement("gpxx:PostalCode", a);
//
//                str.writeEndElement();			// </gpxx:Address>
//            }
//
//            str.writeEndElement();			// </gpxx:WaypointExtension>
//            str.writeEndElement();			// </extensions>
//        }
//
//        str.writeEndElement();				// </wpt>
//    }

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
        kDebug() << "XML writing failed!";
        mSaveFile.abort();
        return (false);
    }

    if (!mSaveFile.finalize())
    {
        setError(mSaveFile.errorString());
        return (false);
    }

    return (true);
}


QString GpxExporter::filter()
{
    return ("*.gpx|GPX files");
}
