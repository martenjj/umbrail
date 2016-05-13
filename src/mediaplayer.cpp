//////////////////////////////////////////////////////////////////////////
//									//
//  Project:	NavTracks						//
//  Edit:	13-May-16						//
//									//
//////////////////////////////////////////////////////////////////////////
//									//
//  Copyright (c) 2012-2015 Jonathan Marten <jjm@keelhaul.me.uk>	//
//  Home and download page:  http://www.keelhaul.me.uk/TBD/		//
//									//
//  This program is free software; you can redistribute it and/or	//
//  modify it under the terms of the GNU General Public License as	//
//  published by the Free Software Foundation; either version 2 of	//
//  the License, or (at your option) any later version.			//
//									//
//  It is distributed in the hope that it will be useful, but		//
//  WITHOUT ANY WARRANTY; without even the implied warranty of		//
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the	//
//  GNU General Public License for more details.			//
//									//
//  You should have received a copy of the GNU General Public		//
//  License along with this program; see the file COPYING for further	//
//  details.  If not, write to the Free Software Foundation, Inc.,	//
//  59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.		//
//									//
//////////////////////////////////////////////////////////////////////////

#include "mediaplayer.h"

#include <qfile.h>
#include <qdebug.h>
#include <qmimetype.h>
#include <qmimedatabase.h>
#include <qfiledialog.h>

#include <kmessagebox.h>
#include <krun.h>
#include <klocalizedstring.h>

#include <kio/job.h>

#ifdef HAVE_PHONON
#include <Phonon/MediaObject>
#include <Phonon/AudioOutput>
#endif

#include <dialogbase.h>
#include <recentsaver.h>

#include "trackdata.h"
#include "settings.h"
#include "videoviewer.h"
#include "photoviewer.h"


static QUrl findMediaFile(const TrackDataWaypoint *item, TrackData::WaypointType expectedType)
{
    if (item==NULL) return (QUrl());
    if (expectedType!=TrackData::WaypointAny && item->waypointType()!=expectedType)
    {
        qWarning() << "waypoint" << item->name() << "is not type" << expectedType;
        return (QUrl());
    }

    QString n = item->metadata("link");			// first try saved media name
    if (n.isEmpty()) n = item->metadata("media");	// compatibility with old metadata
    if (n.isEmpty()) n = item->name();			// then the waypoint name
    qDebug() << "item" << item->name() << "link" << n;

    QUrl file = QUrl::fromUserInput(n, Settings::audioNotesDirectory(), QUrl::AssumeLocalFile);
    if (!QFile::exists(file.path()))
    {
        KMessageBox::error(NULL,
                           i18n("Media file not found:<br><filename>%1</filename>", file.toDisplayString()),
                           i18n("Cannot play media file"));
        return (QUrl());
    }

    qDebug() << "media file" << file;
    return (file);
}


void MediaPlayer::playAudioNote(const TrackDataWaypoint *item)
{
    QUrl file = findMediaFile(item, TrackData::WaypointAudioNote);
    if (!file.isValid()) return;

    // TODO: selectable external player output with a config setting,
    // see krepton//src/sounds.cpp

#ifdef HAVE_PHONON
    Phonon::MediaObject *mediaObject = new Phonon::MediaObject;
    mediaObject->setCurrentSource(file);
    QObject::connect(mediaObject, SIGNAL(finished()), mediaObject, SLOT(deleteLater()));

    Phonon::AudioOutput *audioOutput = new Phonon::AudioOutput(Phonon::NoCategory);
    Phonon::createPath(mediaObject, audioOutput);
    mediaObject->play();
#else
    KMessageBox::error(NULL, i18n("Phonon is not available"), i18n("Cannot play media file"));
#endif
}


void MediaPlayer::playVideoNote(const TrackDataWaypoint *item)
{
    QUrl file = findMediaFile(item, TrackData::WaypointVideoNote);
    if (!file.isValid()) return;

    // TODO: selectable external player output with a config setting,
    // see krepton//src/sounds.cpp

#ifdef HAVE_PHONON
    VideoViewer *v = new VideoViewer(file, NULL);
    v->setWindowTitle(i18nc("@title:window", "Video %1", file.fileName()));
    v->show();
#else
    KMessageBox::error(NULL, i18n("Phonon is not available"), i18n("Cannot play media file"));
#endif
}



void MediaPlayer::viewPhotoNote(const TrackDataWaypoint *item)
{
    QUrl file = findMediaFile(item, TrackData::WaypointPhoto);
    if (!file.isValid()) return;

    PhotoViewer *v = new PhotoViewer(file, NULL);
    v->setWindowTitle(i18nc("@title:window", "Photo %1", file.fileName()));
    v->show();
}


void MediaPlayer::openMediaFile(const TrackDataWaypoint *item)
{
    QUrl file = findMediaFile(item, TrackData::WaypointAny);
    if (file.isEmpty()) return;

    QList<QUrl> urls;
    urls << file;
    KRun::displayOpenWithDialog(urls, NULL);
}


void MediaPlayer::saveMediaFile(const TrackDataWaypoint *item)
{
    QUrl sourceUrl = findMediaFile(item, TrackData::WaypointAny);
    if (!sourceUrl.isValid()) return;

    QMimeDatabase db;
    QMimeType mime = db.mimeTypeForUrl(sourceUrl);

    RecentSaver saver("saveMedia");
    QUrl destUrl = QFileDialog::getSaveFileUrl(NULL,					// parent
                                               i18n("Save Media As"),			// caption
                                               saver.recentUrl(sourceUrl.fileName()),	// dir
                                               mime.filterString(),  			// filter
                                               NULL,					// selectedFilter,
                                               QFileDialog::Options(),			// options
                                               QStringList("file"));			// supportedSchemes

    if (!destUrl.isValid()) return;			// didn't get a file name
    saver.save(destUrl);
    qDebug() << destUrl;

    KIO::file_copy(sourceUrl, destUrl, -1);
}
