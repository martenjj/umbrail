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

#ifndef VARIABLEUNITCOMBO_H
#define VARIABLEUNITCOMBO_H

#include <qcombobox.h>

#include "units.h"


class VariableUnitCombo : public QComboBox
{
    Q_OBJECT

public:
    enum DisplayType
    {
        Distance,
        Speed,
        Elevation,
        Bearing,
        Time
    };

    explicit VariableUnitCombo(VariableUnitCombo::DisplayType displayType, QWidget *pnt = nullptr);
    virtual ~VariableUnitCombo() = default;

    VariableUnitCombo::DisplayType type() const		{ return (mType); }
    Units::Unit unit() const;
    int precision() const;

private:
    int mPrecision;
    VariableUnitCombo::DisplayType mType;
};

#endif							// VARIABLEUNITCOMBO_H
