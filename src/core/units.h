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

#ifndef UNITS_H
#define UNITS_H


/**
 * @short Convert between internal measurements and user units.
 **/

namespace Units
{
    /**
     * Units for measurement
     *
     * @note This is a single enum comvering all types of measurement
     * (distance, elevation, speed etc), and also those which do not
     * correspond to a real measurement unit (bearing, time etc).  
     * This is so that a single @c VariableUnitCombo can return the
     * measurement unit regardless of what type of parameter it is
     * applied to.  An assert will happen if an inappropriate unit
     * is passed to a conversion function.
     *
     * @see VariableUnitCombo
     **/
    enum Unit
    {
        LengthMetres,
        LengthKilometres,
        LengthMiles,
        LengthNauticalMiles,

        ElevationMetres,
        ElevationFeet,

        BearingAbsolute,
        BearingRelative,
        BearingNautical,

        SpeedKilometresHour,
        SpeedMilesHour,
        SpeedKnots,
        SpeedMetresSecond,

        TimeAbsolute,
        TimeRelative
    };

    /**
     * Convert a length or distance from a user measurement to an internal unit.
     *
     * @param l The user measurement to convert
     * @param unit The unit to use for conversion
     * @return The converted length in internal measurement units
     *
     * @note The internal representation is a fraction of the earth's radius.
     **/
    double lengthToInternal(double l, Units::Unit unit);

    /**
     * Convert a length or distance from an internal unit to a user measurement.
     *
     * @param in The internal unit to convert
     * @param unit The unit to use for conversion
     * @return The converted length in the requested user units
     **/
    double internalToLength(double in, Units::Unit unit);

    /**
     * Convert an elevation from a user measurement to an internal unit.
     *
     * @param e The user measurement to convert
     * @param unit The unit to use for conversion
     * @return The converted length in internal elevation units
     *
     * @note The internal representation is metres.
     **/
    double elevationToInternal(double e, Units::Unit unit);

    /**
     * Convert an elevation from an internal unit to a user measurement.
     *
     * @param in The internal unit to convert
     * @param unit The unit to use for conversion
     * @return The converted elevation in the requested user units
     **/
    double internalToElevation(double in, Units::Unit unit);

    /**
     * Convert a speed from a user measurement to an internal unit.
     *
     * @param l The user measurement to convert
     * @param unit The unit to use for conversion
     * @return The converted speed in internal measurement units
     *
     * @note The internal representation (as in a GPX file) is metres per second.
     **/
    double speedToInternal(double s, Units::Unit unit);

    /**
     * Convert a speed from an internal unit to a user measurement.
     *
     * @param in The internal unit to convert
     * @param unit The unit to use for conversion
     * @return The converted speed in the requested user units
     **/
    double internalToSpeed(double in, Units::Unit unit);

    /**
     * Constant representing the earth's radius in kilometres.
     *
     * @note This is exposed here as a @c constexpr so that it can
     * be used in a static or constant initialisation.  For general
     * conversions of user supplied quantities, use the functions above.
     **/
    static constexpr double EARTH_RADIUS_KM = 6371.0;
}

#endif							// UNITS_H
