//////////////////////////////////////////////////////////////////////////
//									//
//  Project:	NavTracks						//
//  Edit:	18-May-16						//
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

#include "style.h"

#include <qdebug.h>

#include "settings.h"


static Style *sGlobalStyle = NULL;

const Style Style::null = Style();


Style::Style()
{
    mLineColour = QColor();
    mPointColour = QColor();
}


Style *Style::globalStyle()
{
    if (sGlobalStyle==NULL)
    {
        sGlobalStyle = new Style();
        qDebug() << "created globalStyle instance";

        // Read the application settings into the global style object
        sGlobalStyle->setLineColour(Settings::lineColour());
        sGlobalStyle->setPointColour(Settings::pointColour());
    }

    return (sGlobalStyle);
}


void Style::setLineColour(const QColor &col)
{
    if (col==Qt::transparent) mLineColour = QColor();
    else mLineColour = col;
}


void Style::setPointColour(const QColor &col)
{
    if (col==Qt::transparent) mPointColour = QColor();
    else mPointColour = col;
}


bool Style::isEmpty() const
{
    return (!hasLineColour() && !hasPointColour());
}


bool Style::operator==(const Style &other) const
{
    if (isEmpty()) return (other.isEmpty());		// two null styles are equal
    if (other.isEmpty()) return (false);		// valid never equals null
    return (mLineColour==other.mLineColour &&		// compare colours for equality
            mPointColour==other.mPointColour);
}


QString Style::toString() const
{
    return (QString("[linecol=%1%2 pointcol=%3%4]")
            .arg(QString::number(mLineColour.rgba(), 16), 8, QChar('0'))
            .arg(!hasLineColour() ? " inherit" : "")
            .arg(QString::number(mPointColour.rgba(), 16), 8, QChar('0'))
            .arg(!hasPointColour() ? " inherit" : ""));
}


QDebug operator<<(QDebug str, const Style &style)
{
    str << style.toString();
    return (str);
}
