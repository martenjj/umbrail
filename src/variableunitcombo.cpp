//////////////////////////////////////////////////////////////////////////
//									//
//  Project:	NavTracks						//
//  Edit:	13-May-16						//
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

#include "variableunitcombo.h"

#include <qdebug.h>

#include <klocalizedstring.h>


static const double EARTH_RADIUS_KM = 6371;		// kilometres
static const double EARTH_RADIUS_MI = 3959;		// statute miles
static const double EARTH_RADIUS_NM = 3440;		// nautical miles

static const double ELEVATION_METRE = 1;		// metres
static const double ELEVATION_FEET = 3.2808;		// feet


VariableUnitCombo::VariableUnitCombo(VariableUnitCombo::DisplayType displayType, QWidget *pnt)
    : QComboBox(pnt)
{
    qDebug() << "type" << displayType;
    mType = displayType;
    mPrecision = 1;

    setObjectName("VariableUnitCombo");

    // TODO: global preference for default units
    setCurrentIndex(0);

    switch (displayType)
    {
case VariableUnitCombo::Distance:
        addItem(i18n("kilometres"), EARTH_RADIUS_KM);
        addItem(i18n("miles"), EARTH_RADIUS_MI);
        addItem(i18n("nautical miles"), EARTH_RADIUS_NM);
        // synchronise this with index in precision() below
        addItem(i18n("metres"), EARTH_RADIUS_KM*1000);
        mPrecision = 2;
        break;

case VariableUnitCombo::Speed:
        addItem(i18n("km/h"), EARTH_RADIUS_KM);
        addItem(i18n("mph"), EARTH_RADIUS_MI);
        addItem(i18n("knots"), EARTH_RADIUS_NM);
        // synchronise this with index in precision() below
        addItem(i18n("m/s"), EARTH_RADIUS_KM*1000/3600);
        break;

case VariableUnitCombo::Elevation:
        addItem(i18n("metres"), ELEVATION_METRE);
        addItem(i18n("feet"), ELEVATION_FEET);
        mPrecision = 0;
        break;

case VariableUnitCombo::Bearing:
        addItem(i18n("absolute"), VariableUnitCombo::BrgAbsolute);
        addItem(i18n("relative"), VariableUnitCombo::BrgRelative);
        addItem(i18n("nautical"), VariableUnitCombo::BrgNautical);
        mPrecision = 0;
        break;

case VariableUnitCombo::Time:
        addItem(i18n("absolute"), VariableUnitCombo::TimeAbsolute);
        addItem(i18n("relative"), VariableUnitCombo::TimeRelative);
        break;

default:
        break;
    }
}


VariableUnitCombo::~VariableUnitCombo()
{
}


double VariableUnitCombo::factor() const
{
    return (itemData(currentIndex()).toDouble());
}


int VariableUnitCombo::precision() const
{						// force for distance/speed in metres
    if (mType==VariableUnitCombo::Distance && currentIndex()==3) return (0);
    if (mType==VariableUnitCombo::Speed && currentIndex()==3) return (1);
    return (mPrecision);
}


double VariableUnitCombo::distanceFromMetres(double m)
{
    return (m/(1000*EARTH_RADIUS_KM));
}
