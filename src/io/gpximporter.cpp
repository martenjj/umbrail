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
#undef DEBUG_DETAILED
#undef DEBUG_TOKENS

#include "gpximporter.h"

#include <QXmlStreamReader>
#include <qcolor.h>
#include <qdebug.h>

#include "trackdata.h"
#include "dataindexer.h"
#include "errorreporter.h"
#include "category.h"
#include "pointicon.h"

#ifdef DEBUG_DETAILED
#include <iostream>
#endif

#define WAYPOINTS_FOLDER_NAME	"Waypoints"
#define NOTES_FOLDER_NAME	"Notes"


GpxImporter::GpxImporter()
    : ImporterBase()
{
    qDebug();
}


bool GpxImporter::loadFrom(QIODevice *dev)
{
    qDebug() << "starting";

    mXmlIndent = 0;
    mContainedChars.clear();

    mWithinMetadata = false;
    mWithinExtensions = false;
    mWithinCategories = false;
    mCurrentTrack = nullptr;
    mCurrentRoute = nullptr;
    mCurrentSegment = nullptr;
    mCurrentPoint = nullptr;

    mUndefinedNamespaces.clear();

    mXmlReader = new QXmlStreamReader(dev);		// XML reader from device
    while (!mXmlReader->atEnd())			// process the token stream
    {
        mXmlReader->readNext();				// get next XML token
#ifdef DEBUG_TOKENS
        qDebug() << "token" << mXmlReader->tokenType() << "name" << mXmlReader->name();
#endif
        switch (mXmlReader->tokenType())		// look at token type
        {
case QXmlStreamReader::NoToken:
            qDebug() << "unexpected NoToken";
            break;

case QXmlStreamReader::Invalid:
            qDebug() << "unexpected Invalid";
            break;

case QXmlStreamReader::StartDocument:
            startDocument(mXmlReader->documentVersion(), mXmlReader->documentEncoding());
            break;

case QXmlStreamReader::EndDocument:
            endDocument();
            break;

case QXmlStreamReader::StartElement:
            checkNamespace(mXmlReader->namespaceUri(), mXmlReader->name(), mXmlReader->prefix());
            startElement(mXmlReader->name().toLocal8Bit(), mXmlReader->qualifiedName().toLocal8Bit(),
                         mXmlReader->attributes());
            break;

case QXmlStreamReader::EndElement:
            endElement(mXmlReader->name().toLocal8Bit(), mXmlReader->qualifiedName().toLocal8Bit());
            break;

case QXmlStreamReader::Characters:
            // Ignoring whitespace was previously set for QXmlSimpleReader by
            //
            //   setFeature("http://trolltech.com/xml/features/report-whitespace-only-CharData", false)
            //
            // This does the equivalent.
            if (!mXmlReader->isWhitespace()) characters(mXmlReader->text());
            break;

case QXmlStreamReader::Comment:
            //qDebug() << "ignored Comment";
            break;

case QXmlStreamReader::DTD:
            qDebug() << "ignored DTD, name" << mXmlReader->dtdName() << "public" << mXmlReader->dtdPublicId() << "system" << mXmlReader->dtdSystemId();
            break;

case QXmlStreamReader::EntityReference:
            qDebug() << "unexpected EntityReference";
            break;

case QXmlStreamReader::ProcessingInstruction:
            qDebug() << "unexpected ProcessingInstruction";
            break;
        }
    }

    const bool ok = !mXmlReader->hasError();		// XML parsing successful?
    qDebug() << "done, ok" << ok;
    // Setting a fatal error is necessary so that FilesController::importFile()
    // will recognise the failure, display the errors and give up.
    if (!ok) reporter()->setError(ErrorReporter::Fatal, "XML parsing failed", mXmlReader->lineNumber());

    delete mXmlReader;					// finished with XML reader
    return (ok);
}


QString GpxImporter::filter()
{
    return ("GPX files (*.gpx)");
}


QByteArray GpxImporter::indent() const
{
    return (QByteArray("  ").repeated(mXmlIndent+1));
}


TrackDataItem *GpxImporter::currentItem() const
{
    TrackDataItem *item = mCurrentPoint;		// find innermost current element
    if (item==nullptr) item = mCurrentSegment;
    if (item==nullptr) item = mCurrentTrack;
    if (item==nullptr) item = mCurrentRoute;
    return (item);
}


void GpxImporter::getLatLong(TrackDataAbstractPoint *pnt, const QXmlStreamAttributes &atts, const QString &localName)
{
    double lat = NAN;					// coordinates found
    double lon = NAN;

    QStringView val = atts.value("lat");
    if (!val.isEmpty()) lat = val.toDouble();
    val = atts.value("lon");
    if (!val.isEmpty()) lon = val.toDouble();

    if (!ISNAN(lat) && !ISNAN(lon)) pnt->setLatLong(lat, lon);
    else addWarning("missing LAT/LON on "+localName.toUpper()+" element");
}


void GpxImporter::addCategory(const QStringView &name, const CategoryData &cat)
{
    // The first time that a valid category has been found,
    // allocate the category map and set it on the root file item.
    // The user of that root item will eventually take ownership of it.
    CategoryList *catMap = dataRoot()->categories();
    if (catMap==nullptr)
    {
        qDebug() << "new category map for" << dataRoot()->name();
        catMap = new CategoryList;
        dataRoot()->setCategories(catMap);
    }
							// add entry to categories
    catMap->addCategory(name.toString(), cat);
}


bool GpxImporter::startDocument(const QStringView &version, const QStringView &encoding)
{
#ifdef DEBUG_DETAILED
    std::cerr << std::endl << qPrintable(indent()) << "START DOCUMENT"
              << " version " << qPrintable(version.toLocal8Bit())
              << " encoding " << qPrintable(encoding.toLocal8Bit()) << std::endl;
#else
    qDebug() << "START DOCUMENT";
#endif
    ++mXmlIndent;
    return (true);
}


// Process the start of an XML element.  Create an empty item of the
// appropriate type, so that it can be filled in as its contained
// elements are read.
//
// If it is necessary to use addError() - which skips the remainder of
// the current element - after an empty item has been created, it must
// be cleaned up again so that it is not mistaken for a new item when
// the parser restarts.

bool GpxImporter::startElement(const QByteArray &localName, const QByteArray &qName,
                               const QXmlStreamAttributes &atts)
{
    if (localName!="gpx")				// past the first GPX element
    {
        // Namespace processing needs to be turned off in order to be able to
        // load files that do not have an XML namespace prefix declared correctly.
        // Otherwise the parser will give an error if any element tag with an
        // undefined prefix is encountered.  See the comments for checkNamespace()
        // below.
        //
        // The processing is turned off here, after the top level GPX element with
        // its list of namespaces has started.  Turning it off right at the
        // beginning would have ignored that namespace list, which means that all
        // namespace prefixes - even those which are correctly defined - are
        // reported as undefined.
        //
        // The Qt function is slightly misnamed - it does not turn off namespace
        // processing completely, an element's namespaceURI is still reported
        // if a prefix is present and the namespace is defined.  It simply causes
        // the parser to not give an error for an undefined prefix.
        if (mXmlReader->namespaceProcessing()) mXmlReader->setNamespaceProcessing(false);
    }

    // First look for elements which simply provide contain a textual
    // value.  If the element tag is recognised, then use the value
    // as appropriate.  The readElementText() consumes the element.
    // This just an optimisation so that characters() and endElement()
    // do not have to be called as the tokeniser parses the element.
    //
    // The (default) option ErrorOnUnexpectedElement to readElementText()
    // will raise an error if any nested elements are found.  Really, the
    // option that is wanted is the missing one - return any text found
    // immediately inside the element (which may be blank) but leave the
    // parser ready to read any contained element without raising an
    // error.  Because of this, collecting the contained text with
    // characters() and processing it in endElement() is still necessary
    // to handle unknown tags as metadata.

    QString elementText;				// text found, if any

    if (localName=="name")				// start of a NAME element
    {							// may belong to any container
        elementText = mXmlReader->readElementText();

        TrackDataItem *item = currentItem();		// find innermost current element
        if (item!=nullptr) item->setName(elementText, true);	// assign its name
        else if (mWithinMetadata) dataRoot()->setMetadata(localName, elementText);
        else addError("NAME not within TRK, TRKSEG, TRKPT, WPT, RTE, RTEPT or METADATA");
    }
    else if (localName=="time")				// start of a TIME element
    {							// may belong to any element
        elementText = mXmlReader->readElementText();
        // The time spec of the decoded date/time is UTC, which is what we want.
        const QDateTime dt = QDateTime::fromString(elementText, Qt::ISODate);

        TrackDataItem *item = currentItem();		// find innermost current element
        if (item==nullptr)				// no element in progress?
        {
            // GPSbabel does not enclose TIME within METADATA:
            //
            // <?xml version="1.0" encoding="UTF-8"?>
            // <gpx version="1.0" ... >
            // <time>2010-04-18T16:28:47Z</time>
            // <bounds minlat="46.827816667" minlon="8.370250000" maxlat="46.850700000" maxlon="8.391166667"/>
            // <wpt> ...

            if (!mWithinMetadata) addError("TIME not within TRK, TRKPT, WPT or METADATA");
            item = dataRoot();				// assume to be in metadata
        }

        item->setMetadata(localName, dt);
    }
    else if (localName=="ele")				// start of an ELE element
    {
        elementText = mXmlReader->readElementText();
        const double ele = elementText.toDouble();
        if (ISNAN(ele)) return (addWarning("Value \""+elementText+"\" ignored for ELE"));

        TrackDataAbstractPoint *tdp = dynamic_cast<TrackDataAbstractPoint *>(currentItem());

        // The explicit use of QVariant(double) seems to be needed, otherwise there is
        // an ambiguous overload:
        //
        //   gpximporter.cpp:547: error: call of overloaded 'setMetadata(int, const double&)' is ambiguous
        //   trackdata.h:207: note: candidate 'void TrackDataItem::setMetadata(int, const QColor&)'
        //   trackdata.h:208: note: candidate 'void TrackDataItem::setMetadata(int, const QVariant&)'
        //
        // I don't understand why, because there is no explicit conversion defined from
        // double to QColor and an implicit conversion from double to any sort of integer
        // type should not be allowed.
        if (tdp!=nullptr) tdp->setMetadata(localName, QVariant(ele));
        else return (addError("ELE not within TRKPT or WPT"));
    }
    else if (localName=="category")			// start of a CATEGORY element
    {
        elementText = mXmlReader->readElementText();
        TrackDataWaypoint *item = dynamic_cast<TrackDataWaypoint *>(currentItem());
        if (item!=nullptr) item->setMetadata(localName, elementText);
        else addError("CATEGORY not within WPT");
    }
    else if (localName=="type")				// start of a TYPE element
    {
        elementText = mXmlReader->readElementText();

        TrackDataItem *item = currentItem();
        if (IS(TrackDataWaypoint, item))
        {
            // For a waypoint, a synonym for CATEGORY but only if
            // there is no CATEGORY already.
            const int idx2 = DataIndexer::index("category");
            if (item->metadata(idx2).isNull()) item->setMetadata(idx2, elementText);
        }
        else if (IS(TrackDataTrack, item) || IS(TrackDataSegment, item))
        {
            // For a track or segment, normal metadata.
            item->setMetadata(localName, elementText);
        }
        else addError("TYPE not within WPT, TRK or TRKSEG");
    }
    else if (qName=="gpxx:Category")
    {
        // This may already be included in CATEGORY/TYPE above, so
        // only combine with the existing category if it is not already present.
        elementText = mXmlReader->readElementText();
        TrackDataWaypoint *item = dynamic_cast<TrackDataWaypoint *>(currentItem());
        if (item!=nullptr)
        {
            QStringList cats = item->metadata("category").toStringList();
            if (!cats.contains(elementText))
            {
                cats.append(elementText);
                item->setMetadata("category", cats);
            }
        }
        else addError("GPXX:CATEGORY not within WPT");
    }
    else if (localName=="color" ||			// start of a COLOR element or
             localName=="linecolor" ||			// application-specific, which
             localName=="pointcolor")			// should be within EXTENSIONS
    {
        elementText = mXmlReader->readElementText();

        TrackDataItem *item = currentItem();		// find innermost current element
        if (item!=nullptr)
        {
            // For the moment the attribute is recorded as is,
            // COLOR will be reconciled with LINECOLOR/POINTCOLOR
            // when the element is being finalised.
            QString rgbString = elementText;
            if (!rgbString.startsWith('#')) rgbString.prepend('#');
            const QColor col(rgbString);
            if (col.isValid()) item->setMetadata(localName, col);
            else addError("invalid value for COLOR");
        }
        else if (localName=="color") addError("COLOR not within TRK, TRKSEG, TRKPT, WPT, RTE or RTEPT");
    }

    // Contrary to standard practice, perform this test with isNull() not
    // isEmpty().  We want to detect a matching element above even if its
    // text value was a null string.
    if (!elementText.isNull())
    {
#ifdef DEBUG_DETAILED
        std::cerr << qPrintable(indent()) << "TEXT <" << qPrintable(localName.toUpper()) << ">"
                  << " = '" << qPrintable(elementText) << "'" << std::endl;
#endif
        return (true);					// element has been processed
    }

    // The start of what is most likely to be a container element.
    // Create a blank item of the appropriate type, which will be
    // filled in as parsing continues and added to the data tree when
    // the corresponding end element is seen.

#ifdef DEBUG_DETAILED
    std::cerr << qPrintable(indent()) << "START <" << qPrintable(localName.toUpper()) << ">" << std::endl;
    for (const QXmlStreamAttribute &att : atts)
    {
        std::cerr << qPrintable(indent()) << "+ " << qPrintable(att.name().toLocal8Bit()) << " = " << qPrintable(att.value().toLocal8Bit()) << std::endl;
    }
#endif
    ++mXmlIndent;					// increase indent for display

    if (localName=="gpx")				// start of a GPX element
    {
        QStringView val = atts.value("version");
        if (!val.isEmpty()) dataRoot()->setMetadata(DataIndexer::index("version"), val.toString());
        val = atts.value("creator");
        if (!val.isEmpty()) dataRoot()->setMetadata(DataIndexer::index("creator"), val.toString());
    }
    else if (localName=="metadata")			// start of a METADATA element
    {
        if (mWithinMetadata || currentItem()!=nullptr)	// check not nested
        {
            addError("nested METADATA elements");
        }

        mWithinMetadata = true;				// just note for contents
    }
    else if (localName=="extensions")			// start of an EXTENSIONS element
    {
        if (mWithinExtensions)				// check not nested
        {
            addError("nested EXTENSIONS elements");
        }

        mWithinExtensions = true;			// just note for contents
    }
    else if (localName=="catmap" || localName=="points_groups")
    {
        // The start of a CATMAP (ours) or POINTS_GROUPS (OsmAnd) container
        // element.  For simplicity the two are considered equivalent.

        if (mWithinCategories)				// check not nested
        {
            addError("nested CATMAP or POINTS_GROUPS elements");
        }

        if (!mWithinExtensions)				// should be within EXTENSIONS
        {
            addWarning("CATMAP or POINTS_GROUPS not within EXTENSIONS");
        }

        mWithinCategories = true;			// just note for contents
    }
    else if (localName=="trk")				// start of a TRK element
    {
        if (currentItem()!=nullptr)			// check not nested
        {
            return (addError("TRK element nested or not at top level"));
        }
							// start new track
        mCurrentTrack = new TrackDataTrack;
    }
    else if (localName=="rte")				// start of a RTE element
    {
        if (currentItem()!=nullptr)			// check not nested
        {
            return (addError("RTE element nested or not at top level"));
        }
							// start new track
        mCurrentRoute = new TrackDataRoute;
    }
    else if (localName=="trkseg")			// start of a TRKSEG element
    {
        if (mCurrentSegment!=nullptr)			// check not nested
        {
            return (addError("nested TRKSEG elements"));
        }

        if (mCurrentTrack==nullptr)			// check properly nested
        {
            return (addError("TRKSEG not within TRK"));
        }
							// start new segment
        mCurrentSegment = new TrackDataSegment;
    }
    else if (localName=="trkpt")			// start of a TRKPT element
    {
        if (mCurrentPoint!=nullptr)			// check not nested
        {
            return (addError("nested TRKPT element"));
        }

        if (mCurrentSegment==nullptr)			// no current segment yet
        {
            if (mCurrentTrack==nullptr)			// must be within track, though
            {
                return (addError("TRKPT not within TRKSEG or TRK"));
            }

            mCurrentSegment = new TrackDataSegment;	// start new implied segment
            addWarning("TRKPT not within TRKSEG");
        }

        mCurrentPoint = new TrackDataTrackpoint;	// start new point item
        getLatLong(mCurrentPoint, atts, localName);	// get coordinates
    }
    else if (localName=="wpt")				// start of an WPT element
    {
        if (currentItem()!=nullptr)			// check not nested
        {
            return (addError("WPT element nested or not at top level"));
        }

        mCurrentPoint = new TrackDataWaypoint;		// start new waypoint item
        getLatLong(mCurrentPoint, atts, localName);	// get coordinates
    }
    else if (localName=="rtept")			// start of an RTEPT element
    {
        if (mCurrentRoute==nullptr)			// check properly nested
        {
            return (addError("RTEPT not within RTE"));
        }

        if (mCurrentPoint!=nullptr)			// check not nested
        {
            return (addError("nested RTEPT element"));
        }

        mCurrentPoint = new TrackDataRoutepoint;	// start new route point item
        getLatLong(mCurrentPoint, atts, localName);	// get coordinates
    }
    else if (localName=="link")				// start of a LINK element
    {
        if (!IS(TrackDataWaypoint, mCurrentPoint))
        {						// check contained where expected
            return (addError("LINK not within WPT"));
        }

        QStringView link = atts.value("link");
        if (link.isEmpty()) link = atts.value("href");
        if (!link.isEmpty()) mCurrentPoint->setMetadata(DataIndexer::indexWithNamespace(qName), link.toString());
        else addWarning("missing LINK/HREF attribute on LINK element");
    }
    else if (localName=="catentry" || localName=="group")
    {
        // The start of a CATENTRY (ours) or GROUP (OsmAnd) category
        // element.  For simplicity the two are again considered equivalent.
        //
        //  <catentry name="personal" color="#eecc22"/>
        //  <group name="personal" color="#eecc22" icon="special_house" background="circle"/>

        if (!mWithinCategories)
        {
            return (addError("CATENTRY or GROUP not within CATMAP or POINTS_GROUPS"));
        }

        QStringView name = atts.value("name");
        if (name.isEmpty())
        {
            // OsmAnd seems to write out
            //
            //   <extensions>
            //     <osmand:points_groups>
            //       <group name=""/>
            //     </osmand:points_groups>
            //   </extensions>
            //
            // at the end of a track recording file.
            if (localName!="group") addWarning(QString("missing NAME attribute on %1 element").arg(localName.toUpper()));
            return (true);
        }

        QColor col;					// get the colour, present for both
        QString rgbString = atts.value("color").toString();
        if (!rgbString.isEmpty())
        {
            if (!rgbString.startsWith('#')) rgbString.prepend('#');
            col = QColor(rgbString);
            if (!col.isValid()) return (addError("invalid value for COLOR"));
        }

        CategoryData cat(col);				// create the category data
        const QStringView iconName = atts.value("icon");
        if (!iconName.isEmpty()) cat.setIcon(iconName.toString());
        const QStringView shape = atts.value("background");
        if (!shape.isEmpty()) cat.setShape(shape.toLatin1());
							// then collect the remaining
        addCategory(name, cat);				// add entry to categories
    }
    else						// start of unrecognised element
    {
        // This is an error because the attribute information will be
        // lost, see endElement().
        if (!atts.isEmpty()) addError(QString("unknown element %1 with attributes").arg(localName.toUpper()));
    }

    mContainedChars.clear();				// clear element contents
    return (true);
}


// Process the end of an XML element.  Finalise the item being created,
// and add it to the data tree in the appropriate place.
//
// The XML parser should ensure that the file is well structured, so the
// element should always have been started and nested correctly.  The
// various "XXX element not started" errors should therefore never be
// seen.  If it is necessary to use addError() - which skips the remainder
// of the current element - before the item being created has been added
// to the data tree, it must be cleaned up so that it is not mistaken for
// a new item when the parser restarts.  See the comment for WPT below.

bool GpxImporter::endElement(const QByteArray &localName, const QByteArray &qName)
{
    --mXmlIndent;
#ifdef DEBUG_DETAILED
    std::cerr << qPrintable(indent()) << "END <" << qPrintable(localName.toUpper()) << ">" << std::endl;
#endif

    if (localName=="gpx") return (true);		// end of the GPX element,
							// nothing to do
    if (localName=="metadata")				// end of a METADATA element
    {
        mWithinMetadata = false;			// just note it finished
        return (true);
    }

    if (localName=="extensions")			// end of an EXTENSIONS element
    {
        mWithinExtensions = false;			// just note it finished
        return (true);
    }

    if (localName=="catmap" || localName=="points_groups")
    {
        // The end of a CATMAP (ours) or POINTS_GROUPS (OsmAnd) container
        // element.
        mWithinCategories = false;			// just note it finished
        return (true);
    }

    // For tags that create elements, if for some reason the element
    // cannot be finalised or added to the data tree it must be cleaned
    // up before returning with addError() to skip the current element.
    // For example,
    //
    //    if (there is a problem with the waypoint)
    //    {
    //      delete mCurrentPoint; mCurrentPoint = nullptr;
    //      return (addError("Waypoint not complete"));
    //    }

    if (localName=="trk")				// end of a TRK element
    {
        if (mCurrentTrack==nullptr)			// check must have started
        {
            return (addError("TRK element not started"));
        }

        if (mCurrentSegment!=nullptr)			// segment not closed
        {						// (may be an implied one)
#ifdef DEBUG_IMPORT
            qDebug() << "got implied TRKSEG:" << mCurrentSegment->name();
#endif
            finaliseElement(mCurrentSegment);
            mCurrentTrack->addChildItem(mCurrentSegment);
            mCurrentSegment = nullptr;			// finished with temporary
        }

#ifdef DEBUG_IMPORT
        qDebug() << "got a TRK:" << mCurrentTrack->name();
#endif
        finaliseElement(mCurrentTrack);
        dataRoot()->addChildItem(mCurrentTrack);
        mCurrentTrack = nullptr;			// finished with temporary
        return (true);
    }
    else if (localName=="trkseg")			// end of a TRKSEG element
    {
        if (mCurrentSegment==nullptr)			// check must have started
        {
            return (addError("TRKSEG element not started"));
        }

#ifdef DEBUG_IMPORT
        qDebug() << "got a TRKSEG:" << mCurrentSegment->name();
#endif
        finaliseElement(mCurrentSegment);
        mCurrentTrack->addChildItem(mCurrentSegment);
        mCurrentSegment = nullptr;			// finished with temporary
        return (true);
    }
    else if (localName=="trkpt")			// end of a TRKPT element
    {
        if (!IS(TrackDataTrackpoint, mCurrentPoint))
        {
            return (addError("TRKPT element not started"));
        }

#ifdef DEBUG_IMPORT
        qDebug() << "got a TRKPT:" << mCurrentPoint->name();
#endif
        Q_ASSERT(mCurrentSegment!=nullptr || mCurrentTrack!=nullptr);
        finaliseElement(mCurrentPoint);
        if (mCurrentSegment!=nullptr) mCurrentSegment->addChildItem(mCurrentPoint);
        else mCurrentTrack->addChildItem(mCurrentPoint);
        mCurrentPoint = nullptr;			// finished with temporary
        return (true);
    }
    else if (localName=="rte")				// end of a RTE element
    {
        if (mCurrentRoute==nullptr)			// check must have started
        {
            return (addError("RTE element not started"));
        }

#ifdef DEBUG_IMPORT
        qDebug() << "got a RTE:" << mCurrentRoute->name();
#endif
        finaliseElement(mCurrentRoute);
        dataRoot()->addChildItem(mCurrentRoute);
        mCurrentRoute = nullptr;			// finished with temporary
        return (true);
    }
    else if (localName=="rtept")			// end of a RTEPT element
    {
        if (!IS(TrackDataRoutepoint, mCurrentPoint))
        {						// check start element matched
            return (addError("RTEPT element not started"));
        }

#ifdef DEBUG_IMPORT
        qDebug() << "got a RTEPT:" << mCurrentPoint->name();
#endif
        Q_ASSERT(mCurrentRoute!=nullptr);
        finaliseElement(mCurrentPoint);
        mCurrentRoute->addChildItem(mCurrentPoint);
        mCurrentPoint = nullptr;			// finished with temporary
        return (true);
    }
    else if (localName=="wpt")				// end of a WPT element
    {
        TrackDataWaypoint *tdw = dynamic_cast<TrackDataWaypoint *>(mCurrentPoint);
        if (tdw==nullptr)				// check must have started
        {
            return (addError("WPT element not started"));
        }

#ifdef DEBUG_IMPORT
        qDebug() << "got a WPT:" << mCurrentPoint->name();
#endif
        // Do we ignore this as a "Home" or "Work" point?
        if (options().hasFlag(ImporterExporterOptions::IgnoreHome))
        {
            const QString &name = tdw->name();
            if (name=="Home" ||				// Garmin
                name=="home" || name=="work")		// OsmAnd+
            {
                qDebug() << "Home point" << name << "ignored";
                delete mCurrentPoint; mCurrentPoint = nullptr;
                return (true);
            }
        }

        TrackDataFolder *folder = waypointFolder(tdw, tdw->isMediaType() ? NOTES_FOLDER_NAME : WAYPOINTS_FOLDER_NAME);
        Q_ASSERT(folder!=nullptr);

        // Clear the folder name metadata, it will be regenerated
        // when the file is exported.
        tdw->setMetadata("folder", QVariant());

        // If the waypoint has no "origin" metadata already, then add it
        // to reflect the file being loaded and time.
        const int idx1 = DataIndexer::index("origin");
        if (tdw->metadata(idx1).isNull()) tdw->setMetadata(idx1, originId());

        // If requested, mark the waypoint as "newly imported".  The flag will
        // be set on all waypoints in this file, but if they eventually get
        // merged as duplicates into the main data tree then the flag set here
        // is ignored.
        if (options().hasFlag(ImporterExporterOptions::MarkNewWaypoints))
        {
            tdw->setMetadata("flags", static_cast<int>(TrackData::NewlyImported));
        }

        // An OsmAnd+ AV note is stored as a waypoint with a special name.
        // Using the GUI, it is possible to rename such a waypoint;  relying
        // on the visible name to locate the media file would then fail.
        // To get around this, we save the original name in the waypoint's
        // metadata under the "link" key which will not get overwritten;  this
        // will from then on be saved and loaded in the GPX file.
        //
        // Only do this check if the "link" metadata has not already
        // been set by a LINK or MEDIA tag.
        if (tdw->isMediaType())
        {
            const int idx2 = DataIndexer::index("link");

            // Map the obsolete MEDIA tag to use LINK instead.
            // Check whether any MEDIA tag has ever been seen first,
            // so as not to create that tag if it not needed.
            if (DataIndexer::exists("media"))
            {
                const QVariant &v1 = tdw->metadata("media");
                if (!v1.isNull())
                {
                    const QVariant &v2 = tdw->metadata(idx2);
                    if (v2.isNull())
                    {
                        addWarning("Obsolete MEDIA changed to LINK");
                        tdw->setMetadata(idx2, v1);
                    }
                    else if (v1!=v2) addWarning("Obsolete MEDIA ignored because LINK is present");
                    tdw->setMetadata("media", QVariant());
                }
            }

            if (tdw->metadata(idx2).isNull()) tdw->setMetadata(idx2, tdw->name());
        }

        finaliseElement(tdw);
        folder->addChildItem(tdw);			// add to destination folder
        mCurrentPoint = nullptr;			// finished with temporary
        return (true);
    }

    // If we get here, the element tag is not recognised as a container
    // or a value that is treated specially.  If the element contained
    // any textual data, then add it to the current element or file metadata
    // indexed by the literal element tag.
    const QString elementText = elementContents();	// get any current contents

    // If the element does not contain textual data but has attributes,
    // unfortunately by now that information is lost.  If it turns out
    // that the attribute data is important, it will need to be retained
    // in startElement() and then stored in the element metadata here.
    if (elementText.isEmpty()) return (true);		// ignore if there is none

    TrackDataItem *item = currentItem();		// find innermost current element

    QByteArray key = qName;				// namespaced name of the element
    // Ultra GPS Logger tags waypoints with <description> instead of <desc>
    if (key=="description") key = "desc";

    // Translate some OsmAnd tags to more generic ones
    if (localName=="amenity_subtype") key = "subtype";
    else if (localName=="visited_date")
    {
        // Ensure that this is stored as a date/time value
        const QDateTime dt = QDateTime::fromString(elementText, Qt::ISODate);
        if (item!=nullptr) item->setMetadata("visited", dt);
        return (true);
    }
    // Ignore other OsmAnd tags that are not particularly useful
    else if (localName.startsWith("collapsable_") ||
             localName.startsWith("osm_tag_") ||
             localName.startsWith("amenity_"))
    {
        // An XML warning may be too noisy here, but data has been lost.
        addWarning(QString("tag %1 ignored").arg(localName.toUpper()));
        return (true);
    }

    // Ths OsmAnd "address" value is the geolocated address of a waypoint.
    // It is not particularly useful to display, but it may be useful for
    // reference so it is retained as is.

    // TODO: display as "Location"

    //
    // OsmAnd stores the user entered address components (which may not be
    // the same as the "address" above) using the same tags as Garmin, but
    // all in lower case and within the <extensions> but not within nested
    // <gpxx:WaypointExtension> and <gpxx:Address>.
    //
    // Files saved by this application will therefore have the OsmAnd tags
    // before the Garmin ones.  So each Garmin tag seen here is checked to
    // see whether the corresponding OsmAnd tag has already been seen, and
    // if so then whether the value is the same.  If this is the case then
    // the Garmin tag can simply be ignored.  If the Garmin tag exists but
    // the OsmAnd tag does not, then the value is set as the OsmAnd tag.
    // The result is that the mixed case Garmin tags should never be seen
    // within the application.

    if (localName=="StreetAddress" || localName=="City" || localName=="State" ||
        localName=="PostalCode" || localName=="Country")
    {
        const QByteArray osmandName = localName.toLower();
        const QVariant osmandData = item->metadata(osmandName);
        if (osmandData.isNull()) item->setMetadata(osmandName, elementText);
        else if (elementText!=osmandData.toString())
        {
            addWarning(QString("address %1 value mismatch, have '%2' here '%3'")
                       .arg(localName.toUpper()).arg(osmandData.toString()).arg(elementText));
        }

        return (true);
    }

    const int idx = DataIndexer::indexWithNamespace(key);
    if (item!=nullptr) item->setMetadata(idx, elementText);
    else if (mWithinMetadata) dataRoot()->setMetadata(idx, elementText);
    else addWarning(QString("unrecognised %1 not expected here").arg(localName.toUpper()));

    return (true);
}


bool GpxImporter::endDocument()
{
    --mXmlIndent;
#ifdef DEBUG_DETAILED
    std::cerr << qPrintable(indent()) << "END DOCUMENT" << std::endl;
#else
    qDebug() << "END DOCUMENT";
#endif

    if (currentItem()!=nullptr)				// check terminated
    {
        addError("Point or container not terminated");
    }
    else if (mWithinMetadata || mWithinExtensions)	// check terminated
    {
        addError("METADATA or EXTENSIONS not terminated");
    }

    if (!mUndefinedNamespaces.isEmpty())
    {
        addWarning("Undefined XML namespaces, re-save file to correct");
    }

    return (true);
}


// This is still necessary, because readElementText() will not work
// as described in startElement().

bool GpxImporter::characters(const QStringView &ch)
{
#ifdef DEBUG_DETAILED
    std::cerr << qPrintable(indent()) << "= '" << qPrintable(ch.toLocal8Bit()) << "'" << std::endl;
#endif
    mContainedChars = ch.trimmed().toString();		// save for element end
    return (true);
}


QString GpxImporter::elementContents()
{
    const QString cc = mContainedChars;			// stored by characters() above
    mContainedChars.clear();				// contents are now consumed
    return (cc);
}


void GpxImporter::addMessage(ErrorReporter::Severity severity, const QString &msg)
{
    reporter()->setError(severity, msg, mXmlReader->lineNumber());
}


bool GpxImporter::addError(const QString &msg)
{
    addMessage(ErrorReporter::Error, msg);

    // If the error detected is at the start of an element (which
    // indicates bad nesting, an unexpected tag or similar), then
    // ignore the remainder of the element.
    if (mXmlReader->isStartElement())
    {
#ifdef DEBUG_IMPORT
        qDebug() << "skipping current" << mXmlReader->name()  << "element";
#endif
        mXmlReader->skipCurrentElement();
    }

    return (true);					// but continue reading
}


bool GpxImporter::addWarning(const QString &msg)
{
    addMessage(ErrorReporter::Warning, msg);
    return (true);					// continue reading
}


bool GpxImporter::addFatal(const QString &msg)
{
    addMessage(ErrorReporter::Fatal, msg);
    mXmlReader->raiseError("XML parsing failed");
    return (false);					// stop reading now
}


bool GpxImporter::needsResave() const
{
    return (!mUndefinedNamespaces.isEmpty());
}


// Look to see whether the current element has an XML namespace prefix
// that is not defined.  This happens with files from an older version
// of the application (before commit a79378d7 in January 2016) which
// did not declare the namespace correctly.
//
// If this is detected, add a file warning and note the prefix of the
// undefined namespace.  This will be checked by the FilesController via
// needsResave() and the user will be prompted to resave the file in order
// to update it with the correct namespace declaration.

void GpxImporter::checkNamespace(const QStringView &namespaceURI,
                                 const QStringView &localName,
                                 const QStringView &nsPrefix)
{
    if (nsPrefix.isEmpty()) return;			// no namespace to check

    if (namespaceURI.isEmpty())				// element with undefined namespace
    {
        const QString qName = nsPrefix+':'+localName;
        if (!mUndefinedNamespaces.contains(qName))	// only report each one once
        {
            addWarning(QString("Undefined namespace '%1' for element &lt;%2&gt;").arg(nsPrefix).arg(localName));
            mUndefinedNamespaces.append(qName);
        }
    }
    else						// namespace and URI are defined
    {
        // If the namespace prefix has a namespace URI associated
        // already, then it must match the current one because the source
        // of the earlier and current - the <gpx> element - is the same
        // for both.  There is therefore no need to check for duplication
        // here.
        DataIndexer::setUriForNamespace(nsPrefix.toLatin1(), namespaceURI.toLatin1());
    }
}


bool GpxImporter::finaliseElement(TrackDataItem *item)
{
    // Check the colour values.  COLOR is the standard element tag that
    // may be generated and interpreted by other applications, while
    // LINECOLOR/POINTCOLOR are our own internal tags as the authoritative
    // record of colour within this application.
    //
    // COLOR may be set or updated by other applications.  Therefore the
    // interpretation of the colour tags varies depending on whether this
    // operation is an import or a file load.
    //
    // For a file load, which is assumed to have been previously saved by us,
    // LINECOLOR/POINTCOLOR are taken as the authoritative values.  If COLOR
    // is present and set to the same value then it is ignored, if not the
    // same value then a warning is given.  If COLOR is present but there are
    // no other colour tags, then the appropriate one is set from COLOR and
    // the original tag removed.
    //
    // For an import, it is assumed that another application may have set
    // the COLOR tag.  If present it is allowed to override LINECOLOR/POINTCOLOR,
    // with again a warning if the values are different.
    //
    // In either case, after copying or ignoring the value as appropriate,
    // the COLOR tag is removed.  It will be regenerated on export if
    // necessary.

    // Only our own colour tags should be present for the top level file
    // element (in file metadata), and both may be present.  Accept and
    // retain them without any further checking.
    if (IS(TrackDataFile, item)) return (true);

    // These checks only need to be performed if the COLOR tag is present.
    const QColor col = item->metadata("color").value<QColor>();
    if (col.isValid())
    {
        // Note whether this is a point element (POINTCOLOR applies), or
        // any other element (LINECOLOR applies).  Then note the tag name
        // as appropriate and get the corresponding colour value.
        const bool isPoint = IS(TrackDataAbstractPoint, item);
        const QByteArray &name = (isPoint ? "pointcolor" : "linecolor");
        const QColor ourCol = item->metadata(name).value<QColor>();

        if (options().hasFlag(ImporterExporterOptions::ImportExport))
        {						// an import operation
            if (ourCol.isValid() && ourCol!=col) addWarning(QString("%1 ignored, using COLOR value").arg(QString(name).toUpper()));
            item->setMetadata(name, col);
        }
        else						// a file load operation
        {
            if (ourCol.isValid() && ourCol!=col) addWarning(QString("COLOR ignored, using %1 value").arg(QString(name).toUpper()));
            else item->setMetadata(name, col);
        }

        item->setMetadata("color", QVariant());		// clear the COLOR value
    }

    // Try to identify whether any map symbol in the file applies to Garmin
    // or to OsmAnd.  If this can be unambiguously identified - that is,
    // there is only a SYM or an ICON tag present - then set the SYMSET
    // hint appropriately so as to simplify lookup within the application.
    // If both are present then do not set or change any such hint, the
    // priority for display will be resolved in TrackDataWaypoint::icon().
    const QVariant icn = item->metadata("icon");
    const QVariant sym = item->metadata("sym");
    const QVariant set = item->metadata("symset");

    if (!icn.isNull() && sym.isNull() && set.isNull()) item->setMetadata("symset", "osmand");
    if (!sym.isNull() && icn.isNull() && set.isNull()) item->setMetadata("symset", "garmin");

    return (true);
}
