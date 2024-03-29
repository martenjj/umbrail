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

#include "mediaplayer.h"

#include <qfile.h>
#include <qdebug.h>
#include <qmimetype.h>
#include <qmimedatabase.h>
#include <qfiledialog.h>

#include <kmessagebox.h>
#include <klocalizedstring.h>

#include <kio/job.h>
#include <kio/applicationlauncherjob.h>
#include <kio/jobuidelegatefactory.h>

#ifdef HAVE_PHONON
#include <phonon/mediaobject.h>
#include <phonon/audiooutput.h>
#endif

#include <kfdialog/dialogbase.h>
#include <kfdialog/recentsaver.h>

#include "trackdata.h"
#include "settings.h"
#include "videoviewer.h"
#include "photoviewer.h"


static QUrl findMediaFile(const TrackDataWaypoint *item, TrackData::WaypointType expectedType)
{
    if (item==nullptr) return (QUrl());
    if (expectedType!=TrackData::WaypointAny && item->waypointType()!=expectedType)
    {
        qWarning() << "waypoint" << item->name() << "is not type" << expectedType;
        return (QUrl());
    }

    QVariant n = item->metadata("link");		// first try saved media name
    if (n.isNull()) n = item->metadata("media");	// compatibility with old metadata
    if (n.isNull()) n = item->name();			// then the waypoint name
    qDebug() << "item" << item->name() << "link" << n.toString();

    QUrl file(n.toString());
    if (file.isRelative()) file = Settings::audioNotesDirectory().resolved(file);
    qDebug() << "->" << file;
    if (file.isLocalFile())				// can check for existence here
    {
        if (!QFile::exists(file.path()))
        {
            KMessageBox::error(nullptr,
                               i18n("Media file not found:<br><filename>%1</filename>", file.toDisplayString()),
                               i18n("Cannot play media file"));
            return (QUrl());
        }
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
    QObject::connect(mediaObject, &Phonon::MediaObject::finished, mediaObject, &QObject::deleteLater);

    Phonon::AudioOutput *audioOutput = new Phonon::AudioOutput(Phonon::NoCategory);
    Phonon::createPath(mediaObject, audioOutput);
    mediaObject->play();
#else
    KMessageBox::error(nullptr, i18n("Phonon is not available"), i18n("Cannot play media file"));
#endif
}


void MediaPlayer::playVideoNote(const TrackDataWaypoint *item)
{
    QUrl file = findMediaFile(item, TrackData::WaypointVideoNote);
    if (!file.isValid()) return;

    // TODO: selectable external player output with a config setting,
    // see krepton//src/sounds.cpp

#ifdef HAVE_PHONON
    VideoViewer *v = new VideoViewer(file, nullptr);
    v->setWindowTitle(i18nc("@title:window", "Video %1", file.fileName()));
    v->show();
#else
    KMessageBox::error(nullptr, i18n("Phonon is not available"), i18n("Cannot play media file"));
#endif
}



void MediaPlayer::viewPhotoNote(const TrackDataWaypoint *item)
{
    QUrl file = findMediaFile(item, TrackData::WaypointPhoto);
    if (!file.isValid()) return;

    PhotoViewer *v = new PhotoViewer(file, nullptr);
    v->setWindowTitle(i18nc("@title:window", "Photo %1", file.fileName()));
    v->show();
}


void MediaPlayer::openMediaFile(const TrackDataWaypoint *item)
{
    QUrl file = findMediaFile(item, TrackData::WaypointAny);
    if (file.isEmpty()) return;

    auto *job = new KIO::ApplicationLauncherJob(nullptr);
    job->setUiDelegate(KIO::createDefaultJobUiDelegate(KJobUiDelegate::AutoHandlingEnabled, nullptr));

    QList<QUrl> urls;
    urls << file;
    job->setUrls(urls);

    job->start();
}


void MediaPlayer::saveMediaFile(const TrackDataWaypoint *item)
{
    QUrl sourceUrl = findMediaFile(item, TrackData::WaypointAny);
    if (!sourceUrl.isValid()) return;

    QMimeDatabase db;
    QMimeType mime = db.mimeTypeForUrl(sourceUrl);

    RecentSaver saver("saveMedia");
    QUrl destUrl = QFileDialog::getSaveFileUrl(nullptr,					// parent
                                               i18n("Save Media As"),			// caption
                                               saver.recentUrl(sourceUrl.fileName()),	// dir
                                               mime.filterString(),  			// filter
                                               nullptr,					// selectedFilter,
                                               QFileDialog::Options(),			// options
                                               QStringList("file"));			// supportedSchemes

    if (!destUrl.isValid()) return;			// didn't get a file name
    saver.save(destUrl);
    qDebug() << destUrl;

    KIO::file_copy(sourceUrl, destUrl, -1);
}
