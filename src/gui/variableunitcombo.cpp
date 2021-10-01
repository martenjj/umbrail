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

#include "variableunitcombo.h"

#include <qdebug.h>

#include <klocalizedstring.h>


VariableUnitCombo::VariableUnitCombo(VariableUnitCombo::DisplayType displayType, QWidget *pnt)
    : QComboBox(pnt)
{
    qDebug() << "type" << displayType;
    mType = displayType;
    mPrecision = 1;

    setObjectName("VariableUnitCombo");

    // TODO: global preference for default units
    // TODO: or get from file timezone if available
    setCurrentIndex(0);

    switch (displayType)
    {
case VariableUnitCombo::Distance:
        addItem(i18n("kilometres"), Units::LengthKilometres);
        addItem(i18n("miles"), Units::LengthMiles);
        addItem(i18n("nautical miles"), Units::LengthNauticalMiles);
        // synchronise this with index in precision() below
        addItem(i18n("metres"), Units::LengthMetres);
        mPrecision = 2;
        break;

case VariableUnitCombo::Speed:
        addItem(i18n("km/h"), Units::SpeedKilometresHour);
        addItem(i18n("mph"), Units::SpeedMilesHour);
        addItem(i18n("knots"), Units::SpeedKnots);
        // synchronise this with index in precision() below
        addItem(i18n("m/s"), Units::SpeedMetresSecond);
        break;

case VariableUnitCombo::Elevation:
        addItem(i18n("metres"), Units::ElevationMetres);
        addItem(i18n("feet"), Units::ElevationFeet);
        mPrecision = 0;
        break;

case VariableUnitCombo::Bearing:
        addItem(i18n("absolute"), Units::BearingAbsolute);
        addItem(i18n("relative"), Units::BearingRelative);
        addItem(i18n("nautical"), Units::BearingNautical);
        mPrecision = 0;
        break;

case VariableUnitCombo::Time:
        addItem(i18n("absolute"), Units::TimeAbsolute);
        addItem(i18n("relative"), Units::TimeRelative);
        break;

default:
        break;
    }
}


Units::Unit VariableUnitCombo::unit() const
{
    return (static_cast<Units::Unit>(itemData(currentIndex()).toInt()));
}


int VariableUnitCombo::precision() const
{						// force for distance/speed in metres
    if (mType==VariableUnitCombo::Distance && currentIndex()==3) return (0);
    if (mType==VariableUnitCombo::Speed && currentIndex()==3) return (1);
    return (mPrecision);
}
