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

#ifndef GPXIMPORTER_H
#define GPXIMPORTER_H

#include <QXmlDefaultHandler>

#include "importerbase.h"

class TrackDataItem;
class TrackDataTrack;
class TrackDataRoute;
class TrackDataSegment;
class TrackDataAbstractPoint;
class TrackDataFolder;
class TrackDataWaypoint;


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
    TrackDataFolder *waypointFolder(const TrackDataWaypoint *tdw = nullptr);
    void getLatLong(TrackDataAbstractPoint *pnt, const QXmlAttributes &atts, const QString &localName);

    bool hasElementContents() const		{ return (!mContainedChars.isEmpty()); }
    QString elementContents()			{ QString cc = mContainedChars; mContainedChars.clear(); return (cc); }

private:
    TrackDataTrack *mCurrentTrack;
    TrackDataRoute *mCurrentRoute;
    TrackDataSegment *mCurrentSegment;
    TrackDataAbstractPoint *mCurrentPoint;

    bool mWithinMetadata;
    bool mWithinExtensions;

    int mXmlIndent;
    QString mRestartTag;
    const QXmlLocator *mXmlLocator;
    QString mContainedChars;

    QStringList mUndefinedNamespaces;
};

#endif							// GPXIMPORTER_H
