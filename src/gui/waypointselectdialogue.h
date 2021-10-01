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

#ifndef WAYPOINTSELECTDIALOGUE_H
#define WAYPOINTSELECTDIALOGUE_H

#include <kfdialog/dialogbase.h>

class QButtonGroup;


class WaypointSelectDialogue : public DialogBase
{
    Q_OBJECT

public:
    enum Selection
    {
        SelectWaypoints = 0x01,
        SelectRoutepoints = 0x02,
        SelectStops = 0x04,
        SelectAudioNotes = 0x08,
        SelectVideoNotes = 0x10,
        SelectPhotos = 0x20
    };
    Q_DECLARE_FLAGS(SelectionSet, Selection)

    WaypointSelectDialogue(QWidget *pnt = nullptr);
    virtual ~WaypointSelectDialogue() = default;

    void setSelection(WaypointSelectDialogue::SelectionSet sel);
    WaypointSelectDialogue::SelectionSet selection() const;

private:
    QButtonGroup *mGroup;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(WaypointSelectDialogue::SelectionSet)

#endif							// WAYPOINTSELECTDIALOGUE_H
