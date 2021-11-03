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

#include "importerbase.h"

#include <string.h>
#include <errno.h>

#include <qfile.h>
#include <qdebug.h>

#include <klocalizedstring.h>

#include "trackdata.h"
#include "dataindexer.h"
#include "errorreporter.h"


#define DEBUG_IMPORT


ImporterBase::ImporterBase()
    : ImporterExporterBase()
{
    qDebug();
    mDataRoot = nullptr;
}


#ifdef DEBUG_IMPORT
static void dumpMetadata(const TrackDataItem *tdd, const QString &source)
{
    qDebug() << source.toLatin1().constData();
    for (int i = 0; i<DataIndexer::count(); ++i)
    {
        QVariant s =  tdd->metadata(i);
        if (s.isNull()) continue;
        qDebug() << "  " << i << DataIndexer::name(i) << "=" << s.toString();
    }
}
#endif


TrackDataFile *ImporterBase::load(const QUrl &file)
{
    qDebug() << "from" << file;
    reporter()->setFile(file);

    // Verify and open the load file
    if (!file.isLocalFile())
    {
        reporter()->setError(ErrorReporter::Fatal, i18n("Can only read local files"));
        return (nullptr);
    }

    QFile importFile(file.path());
    if (!importFile.open(QIODevice::ReadOnly))
    {
        reporter()->setError(ErrorReporter::Fatal, i18n("Cannot open file, %1", strerror(errno)));
        return (nullptr);
    }

    // Allocate the data root item.  If the read is successful
    // then it is returned to the caller which takes ownership of it.

    mDataRoot = new TrackDataFile;			// no need to set name here,
    mDataRoot->setFileName(file);			// this does from file's basename

    // Import from the file
    if (!loadFrom(&importFile))
    {
        qWarning() << "file load failed!";
        delete mDataRoot; mDataRoot = nullptr;
    }

    importFile.close();					// finished with reading file
    if (mDataRoot!=nullptr)				// read successfully and have data
    {
        // The metadata as read from the file is currently stored in mDataRoot.
        // This data is merged with the metadata of each child track, as a record
        // of the original source.  The metadata stored in mDataRoot is ignored
        // when the tree is added to the files model, it is updated to reflect
        // the current state when the file is saved.

#ifdef DEBUG_IMPORT
        dumpMetadata(mDataRoot, "metadata of file:");
#endif
        for (int i = 0; i<mDataRoot->childCount(); ++i)
        {
            TrackDataTrack *tdt = dynamic_cast<TrackDataTrack *>(mDataRoot->childAt(i));
            if (tdt==nullptr) continue;
#ifdef DEBUG_IMPORT
            dumpMetadata(tdt, QString("original metadata of track %1 \"%2\":").arg(i).arg(tdt->name()));
#endif

            if (tdt->metadata("creator").isNull())	// only if blank already
            {
                tdt->copyMetadata(mDataRoot, false);
#ifdef DEBUG_IMPORT
                dumpMetadata(tdt, QString("merged metadata of track %1 \"%2\":").arg(i).arg(tdt->name()));
#endif
            }
        }
    }

    return (mDataRoot);
}
