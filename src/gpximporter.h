
#ifndef GPXIMPORTER_H
#define GPXIMPORTER_H
 

#include <QXmlDefaultHandler>

#include "importerbase.h"


class TrackDataItem;
class TrackDataTrack;
class TrackDataSegment;
class TrackDataTrackpoint;
class TrackDataFolder;
class TrackDataWaypoint;




class GpxImporter : public ImporterBase, public QXmlDefaultHandler
{
public:
    GpxImporter();
    virtual ~GpxImporter();

    static QString filter();

    // ImporterBase
    TrackDataFile *load(const KUrl &file);

    // QXmlContentHandler
    void setDocumentLocator(QXmlLocator *locator);
    bool startDocument();
    bool startElement(const QString &namespaceURI, const QString &localName, const QString &qName, const QXmlAttributes &atts);
    bool characters(const QString &ch);
    bool endElement(const QString &namespaceURI, const QString &localName, const QString &qName);
    bool endDocument();

    // QXmlErrorHandler
    bool error(const QXmlParseException &ex);
    bool fatalError(const QXmlParseException &ex);
    bool warning(const QXmlParseException &ex);

private:
    QByteArray indent() const;
    inline bool parsing() const;
    QXmlParseException makeXmlException(const QString &message, const QString &restartTag = QString::null);
    TrackDataItem *currentItem() const;
    TrackDataFolder *waypointFolder();

private:
    TrackDataTrack *mCurrentTrack;
    TrackDataSegment *mCurrentSegment;
    TrackDataTrackpoint *mCurrentPoint;
    TrackDataWaypoint *mCurrentWaypoint;
    TrackDataFolder *mWaypointFolder;

    bool mWithinMetadata;
    bool mWithinExtensions;

    int mXmlIndent;
    QString mRestartTag;
    const QXmlLocator *mXmlLocator;
    QString mContainedChars;
};

#endif							// GPXIMPORTER_H
