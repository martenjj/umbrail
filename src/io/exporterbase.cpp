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

#include "exporterbase.h"

#include <string.h>
#include <errno.h>

#include <qdebug.h>
#include <qsavefile.h>
#include <qbuffer.h>
#include <qguiapplication.h>
#include <qclipboard.h>
#include <qmimedata.h>

#include <klocalizedstring.h>

#include "trackdata.h"
#include "errorreporter.h"



ExporterBase::ExporterBase()
    : ImporterExporterBase()
{
    qDebug();
    mSelectionId = 0;					// none set yet, export all
}


void ExporterBase::setSelectionId(unsigned long id)
{
    qDebug() << "to" << id;
    mSelectionId = id;					// export just selected items
}


bool ExporterBase::isSelected(const TrackDataItem *item) const
{
    if (!(mOptions & ImporterExporterBase::SelectionOnly)) return (true);
							// all items, not just selection
    const int num = item->childCount();
    if (num>0)						// is this a container?
    {
        // See whether this is actually a container which includes the selected
        // items.  If so, then do not consider the container itself as selected.
        //
        // This is not a trivial operation (needing to look at the selection
        // state of all the child items), but it will only be called once for
        // each parent container (track, segment, folder or route) in the file.
        // There should not be too many of those, and the usual !SelectionOnly
        // case is checked above first.
        for (int i = 0; i<num; ++i)			// look at all children
        {
            const TrackDataItem *childItem = item->childAt(i);
            if (childItem->selectionId()==mSelectionId) return (false);
        }						// a child item is selected
    }

    return (item->selectionId()==mSelectionId);
}


bool ExporterBase::save(const QUrl &file, const TrackDataFile *item, ImporterExporterBase::Options options)
{
    qDebug() << "to" << file << "options" << options;
    mOptions = options;
    reporter()->setFile(file);

    if (options & ImporterExporterBase::ToClipboard)
    {							// output to the clipboard
        QBuffer buf;
        if (!buf.open(QIODevice::WriteOnly))
        {
            reporter()->setError(ErrorReporter::Fatal, i18n("Cannot write to clipboard buffer"));
            return (false);
        }

        if (!saveTo(&buf, item)) return (false);

        buf.close();					// finished writing to buffer
        const QByteArray data = buf.data();
        qDebug() << "clipboard data size" << data.size();
        //qDebug() << "clipboard data" << data;

        // Prepare clipboard data with appropriate MIME type
        QMimeData *mime = new QMimeData;
        mime->setData("application/x-gpx+xml", data);

        // Set data on clipboard
        QClipboard *clip = QGuiApplication::clipboard();
        clip->setMimeData(mime);
    }
    else						// output to a real file
    {
        // verify/open save file
        if (!file.isLocalFile())
        {
            reporter()->setError(ErrorReporter::Fatal, i18n("Can only save to local files"));
            return (false);
        }

        QSaveFile mSaveFile;

        mSaveFile.setFileName(file.path());
        if (!mSaveFile.open(QIODevice::WriteOnly))
        {
            reporter()->setError(ErrorReporter::Fatal, i18n("Cannot open file, %1", strerror(errno)));
            return (false);
        }

        if (!saveTo(&mSaveFile, item))
        {
            reporter()->setError(ErrorReporter::Fatal, mSaveFile.errorString());
            mSaveFile.cancelWriting();
            return (false);
        }

        mSaveFile.commit();
    }

    return (true);					// export was successful
}
