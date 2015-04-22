
#include "gpximporter.h"

#include <math.h>

#include <qxml.h>
#include <qcolor.h>

#include <kdebug.h>
#include <klocale.h>
#include <kurl.h>

#include "trackdata.h"
#include "style.h"
#include "dataindexer.h"


#define WAYPOINT_FOLDER_NAME	"Waypoints"

#undef DEBUG_IMPORT
#undef DEBUG_DETAILED


GpxImporter::GpxImporter()
    : ImporterBase()
{
    kDebug();
}


GpxImporter::~GpxImporter()
{
    kDebug() << "done";
}


TrackDataFile *GpxImporter::load(const KUrl &file)
{
    kDebug() << "from" << file;
    if (!ImporterBase::prepareLoadFile(file)) return (NULL);

    // prepare to read into 'mDataRoot'

    mXmlIndent = 0;
    mXmlLocator = NULL;
    mRestartTag = QString::null;
    mContainedChars = QString::null;

    mWithinMetadata = false;
    mWithinExtensions = false;
    mCurrentTrack = NULL;
    mCurrentSegment = NULL;
    mCurrentPoint = NULL;
    mCurrentWaypoint = NULL;
    mWaypointFolder = NULL;

    QXmlSimpleReader xmlReader;
    xmlReader.setContentHandler(this);
    xmlReader.setErrorHandler(this);
    xmlReader.setFeature("http://trolltech.com/xml/features/report-whitespace-only-CharData", false);

    QXmlInputSource xmlSource(mFile);

    // xml read
    bool ok = xmlReader.parse(xmlSource);
    if (!ok)
    {
        kDebug() << "XML parsing failed!";
        delete mDataRoot; mDataRoot = NULL;
    }

    ImporterBase::finaliseLoadFile(file);
    return (mDataRoot);
}


QString GpxImporter::filter()
{
    return ("*.gpx|GPX files");
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

    int col = (mXmlLocator!=NULL) ? mXmlLocator->columnNumber() : -1;
    int line = (mXmlLocator!=NULL) ? mXmlLocator->lineNumber() : -1;
    return (QXmlParseException(message, col, line));
}


void GpxImporter::setDocumentLocator(QXmlLocator *locator)
{
    mXmlLocator = locator;
}


TrackDataItem *GpxImporter::currentItem() const
{
    TrackDataItem *item = mCurrentPoint;		// find innermost current element
    if (item==NULL) item = mCurrentSegment;
    if (item==NULL) item = mCurrentTrack;
    if (item==NULL) item = mCurrentWaypoint;
    return (item);
}


TrackDataFolder *GpxImporter::createFolder(const QString &path)
{
#ifdef DEBUG_DETAILED
    kDebug() << path;
#endif

    QStringList folders = path.split("/");
    Q_ASSERT(!folders.isEmpty());
    TrackDataItem *cur = mDataRoot;
    TrackDataFolder *foundFolder = NULL;

    for (QStringList::const_iterator it = folders.constBegin(); it!=folders.constEnd(); ++it)
    {
        const QString name = (*it);

        foundFolder = NULL;
        for (int i = 0; i<cur->childCount(); ++i)
        {
            TrackDataFolder *fold = dynamic_cast<TrackDataFolder *>(cur->childAt(i));
            if (fold!=NULL)				// this is a folder
            {
                if (fold->name()==name)			// see if name matches
                {
                    foundFolder = fold;			// yes, use this one
                }
            }
        }

        if (foundFolder==NULL)				// nothing existing found
        {
            kDebug() << "creating folder" << name << "under" << cur->name();
            foundFolder = new TrackDataFolder(name);
            cur->addChildItem(foundFolder);
        }

        cur = foundFolder;
    }

    return (foundFolder);
}


TrackDataFolder *GpxImporter::waypointFolder(const TrackDataWaypoint *tdw)
{
    // Strategy for locating the folder:
    //
    //  - If the waypoint is specified and has a folder defined, use that
    //
    //  - If only one top-level folder exists, regardless of its name, use that
    //
    //  - If more than one such folder exists and one of them is named
    //    "Waypoints", use that
    //
    //  - Otherwise, create a new folder "Waypoints" and use that

    if (tdw!=NULL)					// a waypoint is specified
    {
        const QString path = tdw->metadata("folder");	// its folder, if it has one
        if (!path.isEmpty()) return (createFolder(path));
    }

    if (mWaypointFolder==NULL)				// not allocated/found yet
    {
        TrackDataFolder *foundFolder = NULL;
        int folderCount = 0;
        for (int i = 0; i<mDataRoot->childCount(); ++i)
        {
            TrackDataFolder *fold = dynamic_cast<TrackDataFolder *>(mDataRoot->childAt(i));
            if (fold!=NULL)				// this is a folder
            {
                ++folderCount;
                if (fold->name()==WAYPOINT_FOLDER_NAME) foundFolder = fold;
                else if (foundFolder==NULL) foundFolder = fold;
            }
        }

        if (folderCount!=1)				// no or multiple folders
        {
            if (foundFolder==NULL)			// nothing found during search
            {						// create new folder now
                foundFolder = createFolder("Waypoints");
            }
        }

        mWaypointFolder = foundFolder;
    }

    return (mWaypointFolder);
}


bool GpxImporter::startDocument()
{
    kDebug() << "start document";
#ifdef DEBUG_DETAILED
    qDebug() << endl << indent().constData() << "START DOCUMENT";
#endif
    ++mXmlIndent;
    return (true);
}


bool GpxImporter::startElement(const QString &namespaceURI, const QString &localName, const QString &qName, const QXmlAttributes &atts)
{
#ifdef DEBUG_DETAILED
    qDebug() << indent().constData() << "START" << localName;
#endif

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
        if (mWithinMetadata || mCurrentTrack!=NULL)	// check not nested
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

        if (currentItem()==NULL)			// must be within element
        {
            return (error(makeXmlException("EXTENSIONS not within TRK, TRKSEG, TRKPT or WPT", "extensions")));
        }

        mWithinExtensions = true;			// just note for contents
    }
    else if (localName=="trk")				// start of a TRK element
    {
        if (mCurrentTrack!=NULL)			// check not nested
        {
            return (error(makeXmlException("nested TRK elements", "trk")));
        }
							// start new track
        mCurrentTrack = new TrackDataTrack(QString::null);
    }
    else if (localName=="trkseg")			// start of a TRKSEG element
    {
        if (mCurrentSegment!=NULL)			// check not nested
        {
            return (error(makeXmlException("nested TRKSEG elements", "trkseg")));
        }

        if (mCurrentTrack==NULL)			// check properly nested
        {
            return (error(makeXmlException("TRKSEG start not within TRK", "trkseg")));
        }
							// start new segment
        mCurrentSegment = new TrackDataSegment(QString::null);
    }
    else if (localName=="trkpt")			// start of a TRKPT element
    {
        if (mCurrentPoint!=NULL)			// check not nested
        {
            return (error(makeXmlException("nested TRKPT elements", "trkpt")));
        }

        if (mCurrentSegment==NULL)			// no current segment yet
        {
            if (mCurrentTrack==NULL)			// must be within track, though
            {
                return (error(makeXmlException("TRKPT start not within TRKSEG or TRK", "trkpt")));
            }

            warning(makeXmlException("TRKPT start not within TRKSEG"));
							// start new implied segment
            mCurrentSegment = new TrackDataSegment(QString::null);
        }
							// start new point
        mCurrentPoint = new TrackDataTrackpoint(QString::null);

        double lat = NAN;				// get coordinates
        double lon = NAN;
        for (int i = 0; i<atts.count(); ++i)
        {
            QString attrName = atts.localName(i);
            QString attrValue = atts.value(i);
            if (attrName=="lat") lat = attrValue.toDouble();
            else if (attrName=="lon") lon = attrValue.toDouble();
            else warning(makeXmlException(attrName.toUpper()+" unexpected attribute on TRKPT element"));
        }

        if (!isnan(lat) && !isnan(lon)) mCurrentPoint->setLatLong(lat, lon);
        else warning(makeXmlException("missing lat/lon on TRKPT element"));
    }
    else if (localName=="ele")				// start of an ELEvation element
    {
        if (mCurrentPoint==NULL && mCurrentWaypoint==NULL)
        {						// check properly nested
            return (error(makeXmlException(localName.toUpper()+" start not within TRKPT or WPT", localName)));
        }
    }
    else if (localName=="time")
    {							// start of a TIME element
        if (mCurrentPoint==NULL && !mWithinMetadata)
        {						// check properly nested
            warning(makeXmlException(localName.toUpper()+" start not within TRKPT or METADATA"));
        }
    }
    else if (localName=="wpt")				// start of an WPT element
    {
        if (mCurrentTrack!=NULL)
        {
            return (error(makeXmlException("WPT start within track", "wpt")));
        }

        if (mCurrentWaypoint!=NULL)			// check not nested
        {
            return (error(makeXmlException("nested WPT elements", "wpt")));
        }
							// start new track
        mCurrentWaypoint = new TrackDataWaypoint(QString::null);

        double lat = NAN;				// get coordinates
        double lon = NAN;
        for (int i = 0; i<atts.count(); ++i)
        {
            QString attrName = atts.localName(i);
            QString attrValue = atts.value(i);
            if (attrName=="lat") lat = attrValue.toDouble();
            else if (attrName=="lon") lon = attrValue.toDouble();
            else warning(makeXmlException(attrName.toUpper()+" unexpected attribute on WPT element"));
        }

        if (!isnan(lat) && !isnan(lon)) mCurrentWaypoint->setLatLong(lat, lon);
        else warning(makeXmlException("missing lat/lon on WPT element"));
    }

    mContainedChars = QString::null;			// clear contained data
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
        kDebug() << "got end of METADATA";
#endif
        mWithinMetadata = false;
        return (true);
    }
    else if (localName=="extensions")			// end of an EXTENSIONS element
    {
#ifdef DEBUG_DETAILED
        kDebug() << "got end of EXTENSIONS";
#endif
        mWithinExtensions = false;
        return (true);
    }
    else if (localName=="trk")				// end of a TRK element
    {
        if (mCurrentTrack==NULL)			// check must have started
        {
            return (error(makeXmlException("TRK element not started", "trk")));
        }

        if (mCurrentSegment!=NULL)			// segment not closed
        {						// (may be an implied one)
#ifdef DEBUG_IMPORT
            kDebug() << "got implied TRKSEG:" << mCurrentSegment->name();
#endif
            mCurrentTrack->addChildItem(mCurrentSegment);
            mCurrentSegment = NULL;			// finished with temporary
        }

#ifdef DEBUG_IMPORT
        kDebug() << "got a TRK:" << mCurrentTrack->name();
#endif
        mDataRoot->addChildItem(mCurrentTrack);
        mCurrentTrack = NULL;				// finished with temporary
        return (true);
    }
    else if (localName=="trkseg")			// end of a TRKSEG element
    {
        if (mCurrentSegment==NULL)			// check must have started
        {
            return (error(makeXmlException("TRKSEG element not started", "trkseg")));
        }

#ifdef DEBUG_IMPORT
        kDebug() << "got a TRKSEG:" << mCurrentSegment->name();
#endif
        mCurrentTrack->addChildItem(mCurrentSegment);
        mCurrentSegment = NULL;				// finished with temporary
        return (true);
    }
    else if (localName=="trkpt")			// end of a TRKPT element
    {
        if (mCurrentPoint==NULL)			// check must have started
        {
            return (error(makeXmlException("TRKPT element not started", "trkpt")));
        }

#ifdef DEBUG_IMPORT
        kDebug() << "got a TRKPT:" << mCurrentPoint->name();
#endif
        Q_ASSERT(mCurrentSegment!=NULL || mCurrentTrack!=NULL);
        if (mCurrentSegment!=NULL) mCurrentSegment->addChildItem(mCurrentPoint);
        else mCurrentTrack->addChildItem(mCurrentPoint);
        mCurrentPoint = NULL;				// finished with temporary
        return (true);
    }
    else if (localName=="wpt")				// end of a WPT element
    {
        if (mCurrentWaypoint==NULL)			// check must have started
        {
            return (error(makeXmlException("WPT element not started", "Wpt")));
        }

#ifdef DEBUG_IMPORT
        kDebug() << "got a WPT:" << mCurrentWaypoint->name();
#endif

        TrackDataFolder *folder = waypointFolder(mCurrentWaypoint);
        Q_ASSERT(folder!=NULL);
        folder->addChildItem(mCurrentWaypoint);
        // Clear the folder name metadata, will regenerate on export
        mCurrentWaypoint->setMetadata(DataIndexer::self()->index("folder"), QString::null);
        mCurrentWaypoint = NULL;				// finished with temporary
        return (true);
    }

    if (canRestart) return (true);			// end tag now handled
    if (!parsing()) return (true);			// still ignoring until restart

// handle end of element only if it was error free

    if (localName=="ele")				// end of an ELE element
    {
        const double ele = mContainedChars.toDouble();
        if (mCurrentPoint!=NULL) mCurrentPoint->setElevation(ele);
        else if (mCurrentWaypoint!=NULL) mCurrentWaypoint->setElevation(ele);
        else return (error(makeXmlException("ELE end not within TRKPT or WPT")));
    }
    else if (localName=="time")				// end of a TIME element
    {
        if (mCurrentPoint!=NULL)			// check properly nested
        {
            // The time spec of the decoded date/time is UTC, which is what we want.
            mCurrentPoint->setTime(QDateTime::fromString(mContainedChars, Qt::ISODate));
        }
        else
        {
            // GPSbabel does not enclose TIME within METADATA:
            //
            // <?xml version="1.0" encoding="UTF-8"?>
            // <gpx version="1.0" ... >
            // <time>2010-04-18T16:28:47Z</time>
            // <bounds minlat="46.827816667" minlon="8.370250000" maxlat="46.850700000" maxlon="8.391166667"/>
            // <wpt> ...
            //
            if (!mWithinMetadata) warning(makeXmlException("TIME end not within TRKPT or METADATA"));
            mDataRoot->setMetadata(DataIndexer::self()->index(localName), mContainedChars);
        }
    }
    else if (localName=="name")				// end of a NAME element
    {							// may belong to any container
        TrackDataItem *item = currentItem();		// find innermost current element
        if (item!=NULL) item->setName(mContainedChars);	// assign its name
        else if (mWithinMetadata) mDataRoot->setMetadata(DataIndexer::self()->index(localName), mContainedChars);
        else warning(makeXmlException("NAME not within TRK, TRKSEG, TKKPT, WPT or METADATA"));
    }
    else if (localName=="color")			// end of a COLOR element
    {							// should be within EXTENSIONS
        TrackDataItem *item = currentItem();		// find innermost current element
        if (item==NULL)
        {
            return (error(makeXmlException("COLOR end not within TRK, TRKSEG or TKKPT")));
        }

        bool ok;
        unsigned int rgb = mContainedChars.toUInt(&ok, 16);
        if (!ok)
        {
            return (error(makeXmlException("invalid value for COLOR", localName)));
        }

        Style s = *item->style();
        s.setLineColour(QColor(rgb));
        item->setStyle(s);
    }
    else						// Unknown tag, save as metadata
    {
        TrackDataItem *item = currentItem();		// find innermost current element
        int idx = DataIndexer::self()->index(localName);
        if (item!=NULL) item->setMetadata(idx, mContainedChars);
        else if (mWithinMetadata) mDataRoot->setMetadata(idx, mContainedChars);
        else warning(makeXmlException("unrecognised "+localName.toUpper()+" end not within TRK, TRKSEG, TKKPT, WPT or METADATA"));
    }

    return (true);
}


bool GpxImporter::endDocument()
{
    --mXmlIndent;
#ifdef DEBUG_DETAILED
    qDebug() << indent().constData() << "END DOCUMENT" << endl;
#endif

    if (currentItem()!=NULL)				// check terminated
    {
        return (error(makeXmlException(QString("TRK, TRKSEG, TRKPT or WPT not terminated"))));
    }

    if (mWithinMetadata || mWithinExtensions)		// check terminated
    {
        return (error(makeXmlException(QString("METADATA or EXTENSIONS not terminated"))));
    }

    kDebug() << "end document";
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
    setError(i18n("XML error at line %1 - %2", ex.lineNumber(), ex.message()));
    return (true);
}

bool GpxImporter::fatalError(const QXmlParseException &ex)
{
    setError(i18n("XML fatal at line %1 - %2", ex.lineNumber(), ex.message()));
    return (false);
}

bool GpxImporter::warning(const QXmlParseException &ex)
{
    setError(i18n("XML warning at line %1 - %2", ex.lineNumber(), ex.message()));
    return (true);
}
