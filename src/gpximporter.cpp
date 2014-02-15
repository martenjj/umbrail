
#include "gpximporter.h"

#include <errno.h>
#include <string.h>
#include <math.h>

#include <qxml.h>
#include <qcolor.h>

#include <kdebug.h>
#include <klocale.h>
#include <kurl.h>

#include "trackdata.h"
#include "style.h"



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

// TODO: move to ImporterBase
// verify/open file
    if (!file.isLocalFile())
    {
        setError(i18n("Can only read local files"));
        return (NULL);
    }

    QFile f(file.path());
    if (!f.open(QIODevice::ReadOnly))
    {
        setError(i18n("Cannot open file, %1", strerror(errno)));
        return (NULL);
    }

    if (!ImporterBase::prepareLoadFile(file)) return (NULL);

// prepare to read into 'mReadingFile'
//    emit statusMessage(i18n("Loading '%1'...", file.pathOrUrl()));

    mXmlIndent = 0;
    mXmlLocator = NULL;
    mRestartTag = QString::null;
    mContainedChars = QString::null;

    mCurrentTrack = NULL;
    mCurrentSegment = NULL;
    mCurrentPoint = NULL;
    mCurrentMetadata = NULL;

    QXmlSimpleReader xmlReader;
    xmlReader.setContentHandler(this);
    xmlReader.setErrorHandler(this);
    xmlReader.setFeature ("http://trolltech.com/xml/features/report-whitespace-only-CharData", false);

    QXmlInputSource xmlSource(&f);

// xml read
    bool ok = xmlReader.parse(xmlSource);
    if (!ok)
    {
        kDebug() << "XML parsing failed!";
        delete mReadingFile; mReadingFile = NULL;
    }

    f.close();
    ImporterBase::finaliseLoadFile(file);
    return (mReadingFile);
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




TrackDataDisplayable *GpxImporter::currentItem() const
{
    TrackDataDisplayable *item = mCurrentPoint;		// find innermost current element
    if (item==NULL) item = mCurrentSegment;
    if (item==NULL) item = mCurrentTrack;
    return (item);
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

    else if (localName=="gpx")				// start of a GPX element
    {
        if (mCurrentMetadata==NULL) mCurrentMetadata = new TrackDataMeta(QString::null);

        int i = atts.index("version");
        if (i>=0) mCurrentMetadata->setData(atts.localName(i), atts.value(i));
        i = atts.index("creator");
        if (i>=0) mCurrentMetadata->setData(atts.localName(i), atts.value(i));
    }
    else if (localName=="metadata")			// start of a METADATA element
    {
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
        mCurrentPoint = new TrackDataPoint(QString::null);

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
    else if (localName=="ele" || localName=="hdop" || localName=="speed")
    {							// start of a point parameter element
        if (mCurrentPoint==NULL)
        {						// check properly nested
            return (error(makeXmlException(localName.toUpper()+" start not within TRKPT", localName)));
        }
    }
    else if (localName=="time")
    {							// start of a TIME element
        if (mCurrentPoint==NULL && mCurrentMetadata==NULL)
        {						// check properly nested
            return (error(makeXmlException(localName.toUpper()+" start not within TRKPT or METADATA", localName)));
        }
    }
    else if (localName=="wpt")				// start of an WPT element
    {
        if (mCurrentTrack==NULL)			// check properly nested
        {
            return (warning(makeXmlException("WPT element ignored", "wpt")));
        }
        else
        {
            return (error(makeXmlException("WPT start within track", "wpt")));
        }
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

    if (localName==mRestartTag) mRestartTag.clear();	// found tag, can now restart

// handle end of element here even if it had some errors

    if (localName=="trk")				// end of a TRK element
    {
        if (mCurrentTrack==NULL)			// check must have started
        {
            return (error(makeXmlException("TRK element not started", "trk")));
        }

        if (mCurrentSegment!=NULL)			// segment not closed
        {						// (may be an implied one)
#ifdef DEBUG_IMPORT
            kDebug() << "got implied TRKSEG:" << mCurrentSegment->description();
#endif
            mCurrentTrack->addChildItem(mCurrentSegment);
            mCurrentSegment = NULL;			// finished with temporary
        }

#ifdef DEBUG_IMPORT
        kDebug() << "got a TRK:" << mCurrentTrack->description();
#endif
        mReadingFile->addChildItem(mCurrentTrack);
        mCurrentTrack = NULL;				// finished with temporary
    }
    else if (localName=="trkseg")			// end of a TRKSEG element
    {
        if (mCurrentSegment==NULL)			// check must have started
        {
            return (error(makeXmlException("TRKSEG element not started", "trkseg")));
        }

#ifdef DEBUG_IMPORT
        kDebug() << "got a TRKSEG:" << mCurrentSegment->description();
#endif
        mCurrentTrack->addChildItem(mCurrentSegment);
        mCurrentSegment = NULL;				// finished with temporary
    }
    else if (localName=="trkpt")			// end of a TRKPT element
    {
        if (mCurrentPoint==NULL)			// check must have started
        {
            return (error(makeXmlException("TRKPT element not started", "trkpt")));
        }

#ifdef DEBUG_IMPORT
        kDebug() << "got a TRKPT:" << mCurrentPoint->description();
#endif
        Q_ASSERT(mCurrentSegment!=NULL || mCurrentTrack!=NULL);
        if (mCurrentSegment!=NULL) mCurrentSegment->addChildItem(mCurrentPoint);
        else mCurrentTrack->addChildItem(mCurrentPoint);
        mCurrentPoint = NULL;				// finished with temporary
    }

    if (!parsing()) return (true);

// handle end of element only if it was error free

    if (localName=="ele")				// end of an ELE element
    {
        if (mCurrentPoint==NULL)			// check properly nested
        {
            return (error(makeXmlException("ELE end not within TRKPT")));
        }

        mCurrentPoint->setElevation(mContainedChars.toDouble());
    }
    else if (localName=="time")				// end of a TIME element
    {
        if (mCurrentPoint!=NULL)			// check properly nested
        {
            mCurrentPoint->setTime(QDateTime::fromString(mContainedChars, Qt::ISODate));
        }
        else if (mCurrentMetadata!=NULL)
        {
            mCurrentMetadata->setData(localName, mContainedChars);
        }
        else return (error(makeXmlException("TIME end not within TRKPT or METADATA")));
    }
    else if (localName=="name")				// end of a NAME element
    {							// may belong to any container
        TrackDataDisplayable *item = currentItem();	// find innermost current element
        if (item!=NULL) item->setName(mContainedChars);	// assign its name
        else if (mCurrentMetadata!=NULL) mCurrentMetadata->setData(localName, mContainedChars);
        else warning(makeXmlException("NAME not within TRK, TRKSEG, TKKPT or METADATA"));
    }
    else if (localName=="desc")				// end of a DESC element
    {							// may belong to any container
        TrackDataDisplayable *item = currentItem();	// find innermost current element
        if (item!=NULL) item->setDesc(mContainedChars);	// assign the description
        else if (mCurrentMetadata!=NULL) mCurrentMetadata->setData(localName, mContainedChars);
        else warning(makeXmlException("DESC not within TRK, TRKSEG, TKKPT or METADATA"));
    }
    else if (localName=="hdop")				// end of a HDOP element
    {							// not currently interpreted
        if (mCurrentPoint==NULL)			// check properly nested
        {
            return (error(makeXmlException("HDOP end not within TRKPT")));
        }

        mCurrentPoint->setHdop(mContainedChars);
    }
    else if (localName=="speed")			// end of a SPEED element
    {							// not currently interpreted
        if (mCurrentPoint==NULL)			// check properly nested
        {
            return (error(makeXmlException("SPEED end not within TRKPT")));
        }

        mCurrentPoint->setSpeed(mContainedChars);
    }

    else if (localName=="color")			// end of a COLOR element
    {							// should be within EXTENSIONS
        TrackDataDisplayable *item = currentItem();	// find innermost current element
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

    return (true);
}


bool GpxImporter::endDocument()
{
    --mXmlIndent;
#ifdef DEBUG_DETAILED
    qDebug() << indent().constData() << "END DOCUMENT" << endl;
#endif

    if (mCurrentTrack!=NULL)				// check terminated
    {
        return (error(makeXmlException(QString("TRK not terminated"))));
    }

    if (mCurrentSegment!=NULL)				// check terminated
    {
        return (error(makeXmlException(QString("TRKSEG not terminated"))));
    }

    if (mCurrentPoint!=NULL)				// check terminated
    {
        return (error(makeXmlException(QString("TRKPT not terminated"))));
    }

    kDebug() << "setting metadata" << mCurrentMetadata->toString();
    mReadingFile->setMetadata(mCurrentMetadata);	// add metadata to file record

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
