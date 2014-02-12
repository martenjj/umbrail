//////////////////////////////////////////////////////////////////////////
//									//
//  Project:	NavTracks						//
//  Edit:	11-Feb-14						//
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

#ifndef STYLE_H
#define STYLE_H


#include <qcolor.h>
#include <qdebug.h>


/**
 * @short A drawing/display style for map objects.
 *
 * It may be applied to any file, track or segment, or used as the application's
 * default global setting.
 *
 * @author Jonathan Marten
 **/

class Style
{

public:
    /**
     * Special colour value meaning "inherit from parent".
     *
     * @note This special value is used because a null @c QColor() is not
     * allowed here as an initializer.  Internally, an inherited colour is
     * stored as a null (invalid) @c QColor.
     **/
    static const Qt::GlobalColor InheritColour = Qt::transparent;

    /**
     * A special value meaning a null style, i.e. a style with
     * everything inherited.
     **/
    static const Style null;

    /**
     * Create a new empty style object.
     *
     * The line colour is initialised to be inherited.
     **/
    Style();

    /**
     * Destructor.
     **/
    ~Style()						{}

    /**
     * Create (if necessary) and retrieve the application default style object.
     *
     * @return the style object.
     **/
    static Style *globalStyle();

    /**
     * Set the line colour for this style object.  A colour of @c InheritColour
     * means that the colour is to be inherited from the parent.
     *
     * @p col The colour to be set
     **/
    void setLineColour(const QColor &col);

    /**
     * Retrieve the line colour.
     *
     * @return the line colour, or @c Inherit if to be inherited from the parent.
     **/
    QColor lineColour() const				{ return (mLineColour); }

    /**
     * Check whether an explicit line colour has been set.
     *
     * @return @c true if a colour has been set.
     **/
    bool hasLineColour() const				{ return (mLineColour.isValid()); }

    /**
     * See if this style is empty, i.e. inherits everything
     * from its parent style.
     *
     * @return true if this style is empty
     **/
    bool isEmpty() const;
     
    /**
     * See if this style is equivalent to another.
     *
     * @param other The other style
     * @return @c true if the styles are equivalent
     **/
    bool operator==(const Style &other) const;

    /**
     * Get a printable representation of the style.
     *
     * @return the string representation
     **/
    QString toString() const;

private:
    QColor mLineColour;
};

 

/**
 * Print the representation of a style to a debug stream.
 *
 * @param str The debug stream
 * @param style The style to print
 * @return the same debug stream
 **/
extern QDebug operator<<(QDebug str, const Style &style);



#endif							// STYLE_H
