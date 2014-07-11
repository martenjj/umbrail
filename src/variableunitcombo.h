//////////////////////////////////////////////////////////////////////////
//									//
//  Project:	NavTracks						//
//  Edit:	11-Jul-14						//
//									//
//////////////////////////////////////////////////////////////////////////
//									//
//  Copyright (c) 2012-2014 Jonathan Marten <jjm@keelhaul.me.uk>	//
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

#ifndef VARIABLEUNITCOMBO_H
#define VARIABLEUNITCOMBO_H


#include <qcombobox.h>


class VariableUnitCombo : public QComboBox
{
    Q_OBJECT

public:
    enum DisplayType
    {
        Distance,
        Speed,
        Elevation,
        Bearing
    };

    enum BearingType
    {
        Absolute,
        Relative,
        Nautical
    };

    explicit VariableUnitCombo(VariableUnitCombo::DisplayType displayType, QWidget *pnt = NULL);
    virtual ~VariableUnitCombo();

    VariableUnitCombo::DisplayType type() const		{ return (mType); }
    double factor() const;
    int precision() const;

private:
    int mPrecision;
    VariableUnitCombo::DisplayType mType;
};

 
#endif							// VARIABLEUNITCOMBO_H
