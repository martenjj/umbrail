//////////////////////////////////////////////////////////////////////////
//									//
//  Project:	NavTracks						//
//  Edit:	26-Jun-18						//
//									//
//////////////////////////////////////////////////////////////////////////
//									//
//  Copyright (c) 2012-2018 Jonathan Marten <jjm@keelhaul.me.uk>	//
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


#include "units.h"

#include <qdebug.h>


static constexpr double EARTH_RADIUS_MI = 3959.0;	// statute miles
static constexpr double EARTH_RADIUS_NM = 3440.0;	// nautical miles

static constexpr double ELEVATION_METRE = 1.0;		// metres
static constexpr double ELEVATION_FOOT = 3.2808;	// feet


double Units::lengthToInternal(double l, Units::Unit unit)
{
    switch (unit)
    {
case LengthMetres:		return (l/(1000.0*EARTH_RADIUS_KM));
case LengthKilometres:		return (l/EARTH_RADIUS_KM);
case LengthMiles:		return (l/EARTH_RADIUS_MI);
case LengthNauticalMiles:	return (l/EARTH_RADIUS_NM);
default:			qWarning() << "called with Units::Unit" << unit;
				Q_ASSERT(false);
    }
}


double Units::internalToLength(double in, Units::Unit unit)
{
    switch (unit)
    {
case LengthMetres:		return (in*(1000.0*EARTH_RADIUS_KM));
case LengthKilometres:		return (in*EARTH_RADIUS_KM);
case LengthMiles:		return (in*EARTH_RADIUS_MI);
case LengthNauticalMiles:	return (in*EARTH_RADIUS_NM);
default:			qWarning() << "called with Units::Unit" << unit;
				Q_ASSERT(false);
    }
}


double Units::elevationToInternal(double e, Units::Unit unit)
{
    switch (unit)
    {
case ElevationMetres:	return (e/ELEVATION_METRE);
case ElevationFeet:	return (e/ELEVATION_FOOT);
default:		qWarning() << "called with Units::Unit" << unit;
			Q_ASSERT(false);
    }
}


double Units::internalToElevation(double in, Units::Unit unit)
{
    switch (unit)
    {
case ElevationMetres:	return (in*ELEVATION_METRE);
case ElevationFeet:	return (in*ELEVATION_FOOT);
default:		qWarning() << "called with Units::Unit" << unit;
			Q_ASSERT(false);
    }
}


double Units::speedToInternal(double s, Units::Unit unit)
{
    switch (unit)
    {
case SpeedKilometresHour:	return (s*1000.0/3600.0);
case SpeedMilesHour:		return ((s/0.621371)*1000.0*3600.0);
case SpeedKnots:		return ((s/0.539957)*1000.0*3600.0);
case SpeedMetresSecond:		return (s);
default:			qWarning() << "called with Units::Unit" << unit;
				Q_ASSERT(false);
    }
}


double Units::internalToSpeed(double in, Units::Unit unit)
{
    switch (unit)
    {
case SpeedKilometresHour:	return (in/1000.0*3600.0);
case SpeedMilesHour:		return ((in*0.621371)/1000.0*3600.0);
case SpeedKnots:		return ((in*0.539957)/1000.0*3600.0);
case SpeedMetresSecond:		return (in);
default:			qWarning() << "called with Units::Unit" << unit;
				Q_ASSERT(false);
    }
}
