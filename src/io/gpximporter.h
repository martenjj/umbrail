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

#include "importerbase.h"
#include "errorreporter.h"

class TrackDataItem;
class TrackDataTrack;
class TrackDataRoute;
class TrackDataSegment;
class TrackDataAbstractPoint;
class TrackDataFolder;
class TrackDataWaypoint;

class QXmlStreamReader;
class QXmlStreamAttributes;


class GpxImporter : public ImporterBase
{
public:
    GpxImporter();
    virtual ~GpxImporter() = default;

    static QString filter();

    // ImporterBase
    bool loadFrom(QIODevice *dev) override;
    bool needsResave() const override;

protected:
    // All of these were originally return type 'bool' in QXmlContentHandler.
    // Although the return value is now never used, the return type is kept
    // so that the code sequence
    //
    //    if (there is a problem) return (addError("the error message"));
    //
    // can be used as a simpler alternative to
    //
    //    if (there is a problem)
    //    {
    //      addError("the error message");
    //      return;
    //    }
    //
    // with the slight cost of needing to end with an explicit 'return'.
    bool startElement(const QStringRef &namespaceURI, const QByteArray &localName, const QByteArray &qName, const QXmlStreamAttributes &atts);
    bool endElement(const QStringRef &namespaceURI, const QByteArray &localName, const QByteArray &qName);
    bool characters(const QStringRef &ch);
    bool startDocument(const QStringRef &version, const QStringRef &encoding);
    bool endDocument();

    // Again the equivalents of these were originally return type 'bool'
    // in QXmlErrorHandler.  To support the above, the return type is retained.
    bool addError(const QString &msg);
    bool addWarning(const QString &msg);
    bool addFatal(const QString &msg);

private:
    QByteArray indent() const;
    TrackDataItem *currentItem() const;
    TrackDataFolder *getFolder(const QString &path);
    TrackDataFolder *waypointFolder(const TrackDataWaypoint *tdw = nullptr);
    void getLatLong(TrackDataAbstractPoint *pnt, const QXmlStreamAttributes &atts, const QString &localName);
    QString elementContents();

    void addMessage(ErrorReporter::Severity severity, const QString &msg);

private:
    TrackDataTrack *mCurrentTrack;
    TrackDataRoute *mCurrentRoute;
    TrackDataSegment *mCurrentSegment;
    TrackDataAbstractPoint *mCurrentPoint;

    bool mWithinMetadata;
    bool mWithinExtensions;

    int mXmlIndent;

    // TODO: can possibly be a QStringRef
    QString mContainedChars;

    QStringList mUndefinedNamespaces;

    QXmlStreamReader *mXmlReader;
};

#endif							// GPXIMPORTER_H
