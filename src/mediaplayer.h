//////////////////////////////////////////////////////////////////////////
//									//
//  Project:	NavTracks						//
//  Edit:	17-Jun-15						//
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

#ifndef MEDIAPLAYER_H
#define MEDIAPLAYER_H


class TrackDataWaypoint;

// TODO: needs to be a class, so that it can have the main window as parent
namespace MediaPlayer
{
    void playAudioNote(const TrackDataWaypoint *item);
    void playVideoNote(const TrackDataWaypoint *item);
    void viewPhotoNote(const TrackDataWaypoint *item);

    void openMediaFile(const TrackDataWaypoint *item);
    void saveMediaFile(const TrackDataWaypoint *item);
};

#endif							// MEDIAPLAYER_H
