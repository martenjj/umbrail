
#include "gpximporter.h"

#include <qxml.h>
#include <qcolor.h>
#include <qdebug.h>

#include <klocalizedstring.h>

#include "trackdata.h"
#include "dataindexer.h"
#include "errorreporter.h"


#define WAYPOINTS_FOLDER_NAME	"Waypoints"
#define NOTES_FOLDER_NAME	"Notes"

#undef DEBUG_IMPORT
#undef DEBUG_DETAILED


GpxImporter::GpxImporter()
    : ImporterBase()
{
    qDebug();
}


GpxImporter::~GpxImporter()
{
    qDebug() << "done";
}


TrackDataFile *GpxImporter::load(const QUrl &file)
{
    qDebug() << "from" << file;
    if (!ImporterBase::prepareLoadFile(file)) return (nullptr);

    // prepare to read into 'mDataRoot'

    mXmlIndent = 0;
    mXmlLocator = nullptr;
    mRestartTag.clear();
    mContainedChars.clear();

    mWithinMetadata = false;
    mWithinExtensions = false;
    mCurrentTrack = nullptr;
    mCurrentRoute = nullptr;
    mCurrentSegment = nullptr;
    mCurrentPoint = nullptr;

    mUndefinedNamespaces.clear();

    QXmlSimpleReader xmlReader;
    xmlReader.setContentHandler(this);
    xmlReader.setErrorHandler(this);
    xmlReader.setFeature("http://trolltech.com/xml/features/report-whitespace-only-CharData", false);

    QXmlInputSource xmlSource(mFile);

    // xml read
    bool ok = xmlReader.parse(xmlSource);
    if (!ok)
    {
        qDebug() << "XML parsing failed!";
        delete mDataRoot; mDataRoot = nullptr;
    }

    ImporterBase::finaliseLoadFile(file);
    return (mDataRoot);
}


QString GpxImporter::filter()
{
    return ("GPX files (*.gpx)");
}




QByteArray GpxImporter::indent() const
{
    QString ind = parsing() ? "  " : "! ";
    ind += QString("  ").repeated(mXmlIndent);
    ind.chop(1);					// allow for added qDebug() space
    return (ind.toLatin1());				// cannot do the 'constData() here
}							// (pointer into temporary!)


bool GpxImporter::parsing() const
{
    return (mRestartTag.isEmpty());
}


QXmlParseException GpxImporter::makeXmlException(const QString &message, const QString &restartTag)
{
    if (!restartTag.isEmpty()) mRestartTag = restartTag;

    int col = (mXmlLocator!=nullptr) ? mXmlLocator->columnNumber() : -1;
    int line = (mXmlLocator!=nullptr) ? mXmlLocator->lineNumber() : -1;
    return (QXmlParseException(message, col, line));
}


void GpxImporter::setDocumentLocator(QXmlLocator *locator)
{
    mXmlLocator = locator;
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
#ifdef DEBUG_DETAILED
    qDebug() << path;
#endif

    QStringList folders = path.split("/");
    Q_ASSERT(!folders.isEmpty());
    TrackDataItem *cur = mDataRoot;
    TrackDataFolder *foundFolder = nullptr;

    for (QStringList::const_iterator it = folders.constBegin(); it!=folders.constEnd(); ++it)
    {
        const QString name = (*it);			// look for existing subfolder
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
    //  If the waypoint is specified and has a folder defined, then that
    // folder is used.  Otherwise, an appropriately named top level folder
    // is used, or created if necessary.

    if (tdw!=nullptr)					// a waypoint is specified
    {
        const QVariant path = tdw->metadata("folder");	// its folder, if it has one
        if (!path.isNull()) return (getFolder(path.toString()));
    }
							// find or create folder
    return (getFolder(tdw->isMediaType() ? NOTES_FOLDER_NAME : WAYPOINTS_FOLDER_NAME));
}


void GpxImporter::getLatLong(TrackDataAbstractPoint *pnt, const QXmlAttributes &atts, const QString &localName)
{
    double lat = NAN;					// coordinates found
    double lon = NAN;
    for (int i = 0; i<atts.count(); ++i)
    {
        QString attrName = atts.localName(i);
        QString attrValue = atts.value(i);
        if (attrName=="lat") lat = attrValue.toDouble();
        else if (attrName=="lon") lon = attrValue.toDouble();
        else warning(makeXmlException("unexpected attribute "+attrName.toUpper()+" on "+localName.toUpper()+" element"));
    }

    if (!ISNAN(lat) && !ISNAN(lon)) pnt->setLatLong(lat, lon);
    else warning(makeXmlException("missing LAT/LON on "+localName.toUpper()+" element"));
}


bool GpxImporter::startDocument()
{
    qDebug() << "start document";
#ifdef DEBUG_DETAILED
    qDebug() << endl << indent().constData() << "START DOCUMENT";
#endif
    ++mXmlIndent;
    return (true);
}


bool GpxImporter::startElement(const QString &namespaceURI, const QString &localName,
                               const QString &qName, const QXmlAttributes &atts)
{
#ifdef DEBUG_DETAILED
    qDebug() << indent().constData() << "START" << localName;
#endif

    if (namespaceURI.isEmpty())				// element with undefined namespace
    {
        if (!mUndefinedNamespaces.contains(qName))	// only report each one once
        {
            QStringList nameParts = qName.split(':');
            warning(makeXmlException(QString("Undefined namespace '%1' for element '%2'")
                                     .arg(nameParts.at(0))
                                     .arg(nameParts.at(1).toUpper())));
            mUndefinedNamespaces.append(qName);
        }
    }

    for (int i = 0; i<atts.count(); ++i)
    {
#ifdef DEBUG_DETAILED
        qDebug() << indent().constData() << "+" << atts.localName(i) << "=" << atts.value(i);
#endif
    }

    ++mXmlIndent;
    if (!parsing()) return (true);

    if (localName=="gpx")				// start of a GPX element
    {
        int i = atts.index("version");
        if (i>=0) mDataRoot->setMetadata(DataIndexer::self()->index(atts.localName(i)), atts.value(i));
        i = atts.index("creator");
        if (i>=0) mDataRoot->setMetadata(DataIndexer::self()->index(atts.localName(i)), atts.value(i));
    }
    else if (localName=="metadata")			// start of a METADATA element
    {
        if (mWithinMetadata || currentItem()!=nullptr)	// check not nested
        {
            return (error(makeXmlException("nested METADATA elements", "metadata")));
        }

        mWithinMetadata = true;				// just note for contents
    }
    else if (localName=="extensions")			// start of an EXTENSIONS element
    {
        if (mWithinExtensions)				// check not nested
        {
            return (error(makeXmlException("nested EXTENSIONS elements", "extensions")));
        }

        if (currentItem()==nullptr)			// must be within element
        {
            return (error(makeXmlException("EXTENSIONS not within TRK, TRKSEG, TRKPT or WPT", "extensions")));
        }

        mWithinExtensions = true;			// just note for contents
    }
    else if (localName=="trk")				// start of a TRK element
    {
        if (mCurrentTrack!=nullptr)			// check not nested
        {
            return (error(makeXmlException("nested TRK elements", "trk")));
        }
							// start new track
        mCurrentTrack = new TrackDataTrack;
    }
    else if (localName=="rte")				// start of a RTE element
    {
        if (mCurrentRoute!=nullptr)			// check not nested
        {
            return (error(makeXmlException("nested RTE elements", "rte")));
        }
							// start new track
        mCurrentRoute = new TrackDataRoute;
    }
    else if (localName=="trkseg")			// start of a TRKSEG element
    {
        if (mCurrentSegment!=nullptr)			// check not nested
        {
            return (error(makeXmlException("nested TRKSEG elements", "trkseg")));
        }

        if (mCurrentTrack==nullptr)			// check properly nested
        {
            return (error(makeXmlException("TRKSEG start not within TRK", "trkseg")));
        }
							// start new segment
        mCurrentSegment = new TrackDataSegment;
    }
    else if (localName=="trkpt")			// start of a TRKPT element
    {
        if (mCurrentPoint!=nullptr)			// check not nested
        {
            return (error(makeXmlException("nested TRKPT elements", "trkpt")));
        }

        if (mCurrentSegment==nullptr)			// no current segment yet
        {
            if (mCurrentTrack==nullptr)			// must be within track, though
            {
                return (error(makeXmlException("TRKPT start not within TRKSEG or TRK", "trkpt")));
            }

            warning(makeXmlException("TRKPT start not within TRKSEG"));
							// start new implied segment
            mCurrentSegment = new TrackDataSegment;
        }

        mCurrentPoint = new TrackDataTrackpoint;	// start new point item
        getLatLong(mCurrentPoint, atts, localName);	// get coordinates
    }
    else if (localName=="ele")				// start of an ELEvation element
    {
        if (mCurrentPoint==nullptr)
        {						// check properly nested
            return (error(makeXmlException(localName.toUpper()+" start not within TRKPT or WPT", localName)));
        }
    }
    else if (localName=="time")
    {							// start of a TIME element
        if (mCurrentPoint==nullptr && mCurrentTrack==nullptr && !mWithinMetadata)
        {						// check properly nested
            warning(makeXmlException(localName.toUpper()+" start not within TRKPT, WPT or METADATA"));
        }
    }
    else if (localName=="wpt")				// start of an WPT element
    {
        if (mCurrentTrack!=nullptr || mCurrentRoute!=nullptr)
        {
            return (error(makeXmlException("WPT start within TRK or RTE", "wpt")));
        }

        if (mCurrentPoint!=nullptr)			// check not nested
        {
            return (error(makeXmlException("nested WPT element", "wpt")));
        }

        mCurrentPoint = new TrackDataWaypoint;		// start new waypoint item
        getLatLong(mCurrentPoint, atts, localName);	// get coordinates
    }
    else if (localName=="rtept")			// start of an RTEPT element
    {
        if (mCurrentRoute==nullptr)
        {
            return (error(makeXmlException("RTEPT start not within RTE", "rtept")));
        }

        if (mCurrentPoint!=nullptr)			// check not nested
        {
            return (error(makeXmlException("nested RTEPT element", "rtept")));
        }

        mCurrentPoint = new TrackDataRoutepoint;	// start new route point item
        getLatLong(mCurrentPoint, atts, localName);	// get coordinates
    }
    else if (localName=="link")				// start of a LINK element
    {
        if (dynamic_cast<TrackDataWaypoint *>(mCurrentPoint)==nullptr)
        {						// check contained where expected
            return (error(makeXmlException("LINK start not within WPT", "link")));
        }

        QString link = atts.value("link");
        if (link.isEmpty()) link = atts.value("href");
        if (!link.isEmpty()) mCurrentPoint->setMetadata(DataIndexer::self()->index("link"), link);
        else warning(makeXmlException("missing LINK/HREF attribute on LINK element"));
    }

    mContainedChars.clear();				// clear element contents
    return (true);
}


bool GpxImporter::endElement(const QString &namespaceURI, const QString &localName, const QString &qName)
{
    --mXmlIndent;
#ifdef DEBUG_DETAILED
    qDebug() << indent().constData() << "END  " << localName;
#endif

    const bool canRestart = (localName==mRestartTag);	// found tag, can now restart
    if (canRestart) mRestartTag.clear();		// restart tag now found

// handle end of element here even if it had some errors

    if (localName=="gpx") return (true);		// end of the GPX element
    else if (localName=="metadata")			// end of a METADATA element
    {
#ifdef DEBUG_DETAILED
        qDebug() << "got end of METADATA";
#endif
        mWithinMetadata = false;
        return (true);
    }
    else if (localName=="extensions")			// end of an EXTENSIONS element
    {
#ifdef DEBUG_DETAILED
        qDebug() << "got end of EXTENSIONS";
#endif
        mWithinExtensions = false;
        return (true);
    }
    else if (localName=="trk")				// end of a TRK element
    {
        if (mCurrentTrack==nullptr)			// check must have started
        {
            return (error(makeXmlException("TRK element not started")));
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
            return (error(makeXmlException("TRKSEG element not started")));
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
        if (mCurrentPoint==nullptr)			// check must have started
        {
            return (error(makeXmlException("TRKPT element not started")));
        }

        if (dynamic_cast<TrackDataTrackpoint *>(mCurrentPoint)==nullptr)
        {						// check start element matched
            return (error(makeXmlException("TRKPT end did not start element")));
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
            return (error(makeXmlException("RTE element not started")));
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
        if (mCurrentPoint==nullptr)			// check must have started
        {
            return (error(makeXmlException("RTEPT element not started")));
        }

        if (dynamic_cast<TrackDataRoutepoint *>(mCurrentPoint)==nullptr)
        {						// check start element matched
            return (error(makeXmlException("RTEPT end did not start element")));
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
        if (mCurrentPoint==nullptr)			// check must have started
        {
            return (error(makeXmlException("WPT element not started")));
        }

#ifdef DEBUG_IMPORT
        qDebug() << "got a WPT:" << mCurrentPoint->name();
#endif

        TrackDataWaypoint *tdw = dynamic_cast<TrackDataWaypoint *>(mCurrentPoint);
        if (tdw==nullptr) return (error(makeXmlException("WPT end did not start element")));

        if (tdw->isMediaType())
        {
            // Only do this check if the "link" metadata has not already
            // been set by a LINK tag.
            const int idx = DataIndexer::self()->index("link");
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

        TrackDataFolder *folder = waypointFolder(tdw);
        Q_ASSERT(folder!=nullptr);

        // Clear the folder name metadata, will regenerate on export
        tdw->setMetadata(DataIndexer::self()->index("folder"), QString(""));

        folder->addChildItem(tdw);			// add to destination folder
        mCurrentPoint = nullptr;			// finished with temporary
        return (true);
    }

    if (canRestart) return (true);			// end tag now handled
    if (!parsing()) return (true);			// still ignoring until restart

// handle end of element only if it was error free

    if (localName=="ele")				// end of an ELE element
    {
        const double ele = elementContents().toDouble();
        TrackDataAbstractPoint *p = dynamic_cast<TrackDataAbstractPoint *>(currentItem());

        // The explicit use of QVariant(double) seems to be needed, otherwise there is
        // an ambiguous overload:
        //
        //   gpximporter.cpp:547: error: call of overloaded 'setMetadata(int, const double&)' is ambiguous
        //   In file included from gpximporter.cpp:10:
        //   trackdata.h:207: note: candidate 'void TrackDataItem::setMetadata(int, const QColor&)'
        //   trackdata.h:208: note: candidate 'void TrackDataItem::setMetadata(int, const QVariant&)'
        //
        // I don't understand why, because there is no explicit conversion defined from
        // double to QColor and an implicit conversion from double to any sort of integer
        // type should not be allowed.
        if (p!=nullptr) p->setMetadata(DataIndexer::self()->index(localName), QVariant(ele));
        else return (error(makeXmlException("ELE end not within TRKPT or WPT")));
    }
    else if (localName=="time")				// end of a TIME element
    {							// may belong to any element
        // The time spec of the decoded date/time is UTC, which is what we want.
        const QDateTime dt = QDateTime::fromString(elementContents(), Qt::ISODate);

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
            //
            if (mWithinMetadata) item = mDataRoot;	// but can be in metadata
        }

        if (item!=nullptr) item->setMetadata(DataIndexer::self()->index(localName), dt);
        else return (error(makeXmlException("TIME end not within TRK, TRKPT, WPT or METADATA")));
    }
    else if (localName=="name")				// end of a NAME element
    {							// may belong to any container
        TrackDataItem *item = currentItem();		// find innermost current element
        if (item!=nullptr) item->setName(elementContents(), true);	// assign its name
        else if (mWithinMetadata) mDataRoot->setMetadata(DataIndexer::self()->index(localName), elementContents());
        else warning(makeXmlException("NAME end not within TRK, TRKSEG, TRKPT, WPT, RTE, RTEPT or METADATA"));
    }
    else if (localName=="color")			// end of a COLOR element
    {							// should be within EXTENSIONS
        TrackDataItem *item = currentItem();		// find innermost current element
        if (item==nullptr) return (error(makeXmlException("COLOR end not within TRK, TRKSEG, TRKPT or WPT")));

        QString rgbString = elementContents();
        if (!rgbString.startsWith('#')) rgbString.prepend('#');
        QColor col(rgbString);
        if (!col.isValid()) return (error(makeXmlException("invalid value for COLOR")));

        // The COLOR attribute will only set our internal LINECOLOR/POINTCOLOR
        // attributes if they are not already set.
        if (dynamic_cast<const TrackDataAbstractPoint *>(item)!=nullptr)
        {						// colour for a point
            const QVariant &v = item->metadata("pointcolor");
            if (v.isNull()) item->setMetadata(DataIndexer::self()->index("pointcolor"), col);
        }
        else						// colour for a line/container
        {
            const QVariant &v = item->metadata("linecolor");
            if (v.isNull()) item->setMetadata(DataIndexer::self()->index("linecolor"), col);
        }
    }
    else if (localName=="category")			// end of a CATEGORY element
    {
        TrackDataWaypoint *item = dynamic_cast<TrackDataWaypoint *>(currentItem());

        if (item!=nullptr) item->setMetadata(DataIndexer::self()->index(localName), elementContents());
        else warning(makeXmlException("CATEGORY end not within WPT"));
    }
    else if (localName=="type")				// end of a TYPE element
    {
        TrackDataItem *item = currentItem();

        if (dynamic_cast<TrackDataWaypoint *>(item)!=nullptr)
        {
            // For a waypoint, a synonym for CATEGORY but only if
            // there is no CATEGORY already.
            int idx = DataIndexer::self()->index("category");
            if (item->metadata(idx).isNull()) item->setMetadata(idx, elementContents());
        }
        else if (dynamic_cast<TrackDataTrack *>(item)!=nullptr || dynamic_cast<TrackDataSegment *>(item)!=nullptr)
        {
            // For a track or segment, normal metadata.
            item->setMetadata(DataIndexer::self()->index(localName), elementContents());
        }
        else warning(makeXmlException("TYPE end not within WPT, TRK or TRKSEG"));
    }
    else if (localName=="Category" && qName.startsWith("gpxx:"))
    {							// ignore this, covered in CATEGORY/TYPE
        (void) elementContents();
    }
    else						// Unknown tag, save as metadata
    {
        if (!hasElementContents()) return (true);	// if there is anything to save

        TrackDataItem *item = currentItem();		// find innermost current element

        // Ultra GPS Logger tags waypoints with <description> instead of <desc>
        QString key = localName;
        if (key=="description") key = "desc";
        int idx = DataIndexer::self()->index(key);

        if (item!=nullptr) item->setMetadata(idx, elementContents());
        else if (mWithinMetadata) mDataRoot->setMetadata(idx, elementContents());
        else warning(makeXmlException("unrecognised "+localName.toUpper()+" end not within TRK, TRKSEG, TRKPT, WPT or METADATA"));
    }

    return (true);
}


bool GpxImporter::endDocument()
{
    --mXmlIndent;
#ifdef DEBUG_DETAILED
    qDebug() << indent().constData() << "END DOCUMENT" << endl;
#endif

    if (currentItem()!=nullptr)				// check terminated
    {
        return (error(makeXmlException(QString("Point or container not terminated"))));
    }

    if (mWithinMetadata || mWithinExtensions)		// check terminated
    {
        return (error(makeXmlException(QString("METADATA or EXTENSIONS not terminated"))));
    }

    if (!mUndefinedNamespaces.isEmpty())
    {
        warning(makeXmlException("Undefined XML namespaces, re-save file to correct"));
    }

    qDebug() << "end document";
    return (true);
}


bool GpxImporter::characters(const QString &ch)
{
#ifdef DEBUG_DETAILED
    qDebug() << indent().constData() << "=" << ch;
#endif
    if (!parsing()) return (true);

    mContainedChars = ch.trimmed();			// save for element end
    return (true);
}


bool GpxImporter::error(const QXmlParseException &ex)
{
    reporter()->setError(ErrorReporter::Error, ex.message(), ex.lineNumber());
    return (true);
}

bool GpxImporter::fatalError(const QXmlParseException &ex)
{
    reporter()->setError(ErrorReporter::Fatal, ex.message(), ex.lineNumber());
    return (false);
}

bool GpxImporter::warning(const QXmlParseException &ex)
{
    reporter()->setError(ErrorReporter::Warning, ex.message(), ex.lineNumber());
    return (true);
}


bool GpxImporter::needsResave() const
{
    return (!mUndefinedNamespaces.isEmpty());
}
