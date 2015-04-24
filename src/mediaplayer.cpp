//////////////////////////////////////////////////////////////////////////
//									//
//  Project:	NavTracks						//
//  Edit:	23-Apr-15						//
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

#include <kdebug.h>
#include <klocale.h>
#include <kprocess.h>
#include <kmessagebox.h>

#ifdef HAVE_PHONON
#include <Phonon/MediaObject>
#include <Phonon/AudioOutput>
#endif

#include "trackdata.h"
#include "settings.h"


void MediaPlayer::playAudioNote(const TrackDataWaypoint *item)
{
    if (item==NULL) return;

    if (item->waypointType()!=TrackData::WaypointAudioNote)
    {
        kWarning() << "waypoint" << item->name() << "is not an AudioNote";
        return;
    }

    QString n = item->metadata("media");		// first try saved media name
    if (n.isEmpty()) n = item->name();			// then the waypoint name

    kDebug() << item->name() << n;

    QFile mediaFile(KUrl(Settings::audioNotesDirectory()+"/"+n).path());
    if (!mediaFile.exists())
    {
        KMessageBox::error(NULL,
                           i18n("Media file not found:<br><filename>%1</filename>", mediaFile.fileName()),
                           i18n("Cannot play media file"));
        return;
    }
    kDebug() << "playing" << mediaFile.fileName();

    // TODO: selectable external player output with a config setting,
    // see krepton//src/sounds.cpp

#ifdef HAVE_PHONON
    Phonon::MediaObject *mediaObject = new Phonon::MediaObject;
    mediaObject->setCurrentSource(mediaFile.fileName());
    QObject::connect(mediaObject, SIGNAL(finished()), mediaObject, SLOT(deleteLater()));

    Phonon::AudioOutput *audioOutput = new Phonon::AudioOutput(Phonon::NoCategory);
    Phonon::createPath(mediaObject, audioOutput);
    mediaObject->play();
#else
    KMessageBox::error(NULL, i18n("Phonon is not available"), i18n("Cannot play media file"));
#endif
}
