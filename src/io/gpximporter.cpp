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
#undef DEBUG_DETAILED
#undef DEBUG_TOKENS

#include "gpximporter.h"

#include <QXmlStreamReader>
#include <qcolor.h>
#include <qdebug.h>

#include "trackdata.h"
#include "dataindexer.h"
#include "errorreporter.h"

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
            qDebug() << "ignored Comment";
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


TrackDataFolder *GpxImporter::getFolder(const QString &path)
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


TrackDataFolder *GpxImporter::waypointFolder(const TrackDataWaypoint *tdw)
{
    Q_ASSERT(tdw!=nullptr);

    // If the waypoint has a folder defined, then that folder is used.
    // Otherwise, an appropriately named top level folder is used, or
    // created if necessary.

    const QVariant path = tdw->metadata("folder");	// waypoint folder, if it has one
    if (!path.isNull()) return (getFolder(path.toString()));
							// find or create folder
    return (getFolder(tdw->isMediaType() ? NOTES_FOLDER_NAME : WAYPOINTS_FOLDER_NAME));
}


void GpxImporter::getLatLong(TrackDataAbstractPoint *pnt, const QXmlStreamAttributes &atts, const QString &localName)
{
    double lat = NAN;					// coordinates found
    double lon = NAN;

    QStringRef val = atts.value("lat");
    if (!val.isEmpty()) lat = val.toDouble();
    val = atts.value("lon");
    if (!val.isEmpty()) lon = val.toDouble();

    if (!ISNAN(lat) && !ISNAN(lon)) pnt->setLatLong(lat, lon);
    else addWarning("missing LAT/LON on "+localName.toUpper()+" element");
}


bool GpxImporter::startDocument(const QStringRef &version, const QStringRef &encoding)
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
    // do not havs to be called as the tokeniser parses the element.
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
        else if (mWithinMetadata) mDataRoot->setMetadata(localName, elementText);
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
            item = mDataRoot;				// assume to be in metadata
        }

        item->setMetadata(localName, dt);
    }
    else if (localName=="ele")				// start of an ELE element
    {
        elementText = mXmlReader->readElementText();
        const double ele = elementText.toDouble();
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
        if (dynamic_cast<TrackDataWaypoint *>(item)!=nullptr)
        {
            // For a waypoint, a synonym for CATEGORY but only if
            // there is no CATEGORY already.
            const int idx2 = DataIndexer::index("category");
            if (item->metadata(idx2).isNull()) item->setMetadata(idx2, elementText);
        }
        else if (dynamic_cast<TrackDataTrack *>(item)!=nullptr || dynamic_cast<TrackDataSegment *>(item)!=nullptr)
        {
            // For a track or segment, normal metadata.
            item->setMetadata(localName, elementText);
        }
        else addError("TYPE not within WPT, TRK or TRKSEG");
    }
    else if (qName=="gpxx:Category")
    {
        // Ignore this, covered by CATEGORY/TYPE above
        elementText = mXmlReader->readElementText();
        return (true);
    }
    else if (localName=="color")			// start of a COLOR element, which
    {							// should be within EXTENSIONS
        elementText = mXmlReader->readElementText();

        TrackDataItem *item = currentItem();		// find innermost current element
        if (item!=nullptr)
        {
            QString rgbString = elementText;
            if (!rgbString.startsWith('#')) rgbString.prepend('#');
            QColor col(rgbString);
            if (!col.isValid()) return (addError("invalid value for COLOR"));

            // The COLOR attribute will only set our internal LINECOLOR/POINTCOLOR
            // attributes if they are not already set.
            if (dynamic_cast<const TrackDataAbstractPoint *>(item)!=nullptr)
            {						// colour for a point
                const int idx2 = DataIndexer::index("pointcolor");
                const QVariant &v = item->metadata(idx2);
                if (v.isNull()) item->setMetadata(idx2, col);
            }
            else					// colour for a line/container
            {
                const int idx2 = DataIndexer::index("linecolor");
                const QVariant &v = item->metadata(idx2);
                if (v.isNull()) item->setMetadata(idx2, col);
            }
        }
        else addError("COLOR not within TRK, TRKSEG, TRKPT, WPT, RTE or RTEPT");
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
        QStringRef val = atts.value("version");
        if (!val.isEmpty()) mDataRoot->setMetadata(DataIndexer::index("version"), val.toString());
        val = atts.value("creator");
        if (!val.isEmpty()) mDataRoot->setMetadata(DataIndexer::index("creator"), val.toString());
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

        if (currentItem()==nullptr)			// must be within element
        {
            return (addError("EXTENSIONS not expected here"));
        }

        mWithinExtensions = true;			// just note for contents
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
        if (mCurrentRoute==nullptr)
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
        if (dynamic_cast<TrackDataWaypoint *>(mCurrentPoint)==nullptr)
        {						// check contained where expected
            return (addError("LINK not within WPT"));
        }

        QStringRef link = atts.value("link");
        if (link.isEmpty()) link = atts.value("href");
        if (!link.isEmpty()) mCurrentPoint->setMetadata(DataIndexer::indexWithNamespace(qName), link.toString());
        else addWarning("missing LINK/HREF attribute on LINK element");
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
            mCurrentTrack->addChildItem(mCurrentSegment);
            mCurrentSegment = nullptr;			// finished with temporary
        }

#ifdef DEBUG_IMPORT
        qDebug() << "got a TRK:" << mCurrentTrack->name();
#endif
        mDataRoot->addChildItem(mCurrentTrack);
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
        mCurrentTrack->addChildItem(mCurrentSegment);
        mCurrentSegment = nullptr;			// finished with temporary
        return (true);
    }
    else if (localName=="trkpt")			// end of a TRKPT element
    {
        if (dynamic_cast<TrackDataTrackpoint *>(mCurrentPoint)==nullptr)
        {
            return (addError("TRKPT element not started"));
        }

#ifdef DEBUG_IMPORT
        qDebug() << "got a TRKPT:" << mCurrentPoint->name();
#endif
        Q_ASSERT(mCurrentSegment!=nullptr || mCurrentTrack!=nullptr);
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
        mDataRoot->addChildItem(mCurrentRoute);
        mCurrentRoute = nullptr;			// finished with temporary
        return (true);
    }
    else if (localName=="rtept")			// end of a RTEPT element
    {
        if (dynamic_cast<TrackDataRoutepoint *>(mCurrentPoint)==nullptr)
        {						// check start element matched
            return (addError("RTEPT element not started"));
        }

#ifdef DEBUG_IMPORT
        qDebug() << "got a RTEPT:" << mCurrentPoint->name();
#endif
        Q_ASSERT(mCurrentRoute!=nullptr);
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
        if (tdw->isMediaType())
        {
            // Only do this check if the "link" metadata has not already
            // been set by a LINK tag.
            const int idx = DataIndexer::index("link");
            if (tdw->metadata(idx).isNull())
            {
                // An OsmAnd+ AV note is stored as a waypoint with a special name.
                // Using the GUI, it is possible to rename such a waypoint;  relying
                // on the visible name to locate the media file would then fail.
                // To get around this, we save the original name in the waypoint's
                // metadata under a special key which will not get overwritten;  this
                // will from then on be saved and loaded in the GPX file.
                tdw->setMetadata(idx, tdw->name());
            }
        }

        // If for some reason any element cannot be finalised or added to the
        // data tree, it must be cleaned up before returning with addError()
        // to skip the current element.  For example,
        //
        //    if (there is a problem with the waypoint)
        //    {
        //      delete mCurrentPoint; mCurrentPoint = nullptr;
        //      return (addError("Waypoint not complete"));
        //    }

        TrackDataFolder *folder = waypointFolder(tdw);
        Q_ASSERT(folder!=nullptr);

        // Clear the folder name metadata, it will be regenerated
        // when the file is exported.
        tdw->setMetadata("folder", QVariant());

        folder->addChildItem(tdw);			// add to destination folder
        mCurrentPoint = nullptr;			// finished with temporary
        return (true);
    }

    // If we get here, the element tag is not recognised as a container
    // or a value that is treated specially.  If the element contained
    // any textual data, then add it to the current element or file metadata
    // indexed by the literal element tag.

    const QString elementText = elementContents();	// get any current contents
    if (elementText.isEmpty()) return (true);		// ignore if there was none

    QByteArray key = qName;				// namespaced name of the element
    // Ultra GPS Logger tags waypoints with <description> instead of <desc>
    if (key=="description") key = "desc";
    const int idx = DataIndexer::indexWithNamespace(key);

    TrackDataItem *item = currentItem();		// find innermost current element
    if (item!=nullptr) item->setMetadata(idx, elementText);
    else if (mWithinMetadata) mDataRoot->setMetadata(idx, elementText);
    else addWarning("unrecognised "+localName.toUpper()+" not expected here");

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

bool GpxImporter::characters(const QStringRef &ch)
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

void GpxImporter::checkNamespace(const QStringRef &namespaceURI,
                                 const QStringRef &localName,
                                 const QStringRef &nsPrefix)
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
