
#ifndef GPXIMPORTER_H
#define GPXIMPORTER_H
 

#include <QXmlDefaultHandler>

#include "importerbase.h"


class TrackDataItem;
class TrackDataTrack;
class TrackDataRoute;
class TrackDataSegment;
class TrackDataTrackpoint;
class TrackDataFolder;
class TrackDataWaypoint;
class TrackDataRoutepoint;




class GpxImporter : public ImporterBase, public QXmlDefaultHandler
{
public:
    GpxImporter();
    virtual ~GpxImporter();

    static QString filter();

    // ImporterBase
    TrackDataFile *load(const QUrl &file) override;
    bool needsResave() const override;

    // QXmlContentHandler
    void setDocumentLocator(QXmlLocator *locator) override;
    bool startDocument() override;
    bool startElement(const QString &namespaceURI, const QString &localName, const QString &qName, const QXmlAttributes &atts) override;
    bool characters(const QString &ch) override;
    bool endElement(const QString &namespaceURI, const QString &localName, const QString &qName) override;
    bool endDocument() override;

    // QXmlErrorHandler
    bool error(const QXmlParseException &ex) override;
    bool fatalError(const QXmlParseException &ex) override;
    bool warning(const QXmlParseException &ex) override;

private:
    QByteArray indent() const;
    inline bool parsing() const;
    QXmlParseException makeXmlException(const QString &message, const QString &restartTag = QString());
    TrackDataItem *currentItem() const;
    TrackDataFolder *getFolder(const QString &path);
    TrackDataFolder *waypointFolder(const TrackDataWaypoint *tdw = NULL);

    bool hasElementContents() const		{ return (!mContainedChars.isEmpty()); }
    QString elementContents()			{ QString cc = mContainedChars; mContainedChars.clear(); return (cc); }

private:
    TrackDataTrack *mCurrentTrack;
    TrackDataRoute *mCurrentRoute;
    TrackDataSegment *mCurrentSegment;
    TrackDataTrackpoint *mCurrentPoint;
    TrackDataWaypoint *mCurrentWaypoint;
    TrackDataRoutepoint *mCurrentRoutepoint;

    bool mWithinMetadata;
    bool mWithinExtensions;

    int mXmlIndent;
    QString mRestartTag;
    const QXmlLocator *mXmlLocator;
    QString mContainedChars;

    QStringList mUndefinedNamespaces;
};

#endif							// GPXIMPORTER_H
