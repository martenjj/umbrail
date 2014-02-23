//////////////////////////////////////////////////////////////////////////
//									//
//  Project:	NavTracks						//
//  Edit:	23-Feb-14						//
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

#ifndef DATAINDEXER_H
#define DATAINDEXER_H


#include <qhash.h>


/**
 * @short Maintains data indexes for file, track/segment and point data.
 *
 * Depending on the contents of the GPX file, arbitrary data and attributes
 * may need to be attached to any TrackDataItem.  The logical way to do this
 * would be a hash of <name,value> pairs as part of the item - however, this
 * would have a significant memory and string overhead especially as there
 * will normally only be a small number of names but with most of them
 * common between items.
 *
 * In a similar way to X server atoms, this class manages a mapping between
 * attribute/element names and small integer indexes.  Using the index, the
 * value can be stored in a more compact vector within the item.
 *
 * @author Jonathan Marten
 **/

class DataIndexer
{

public:
    /**
     * Get the singleton instance, creating it if necessary.
     *
     * @return the instance
     **/
    static DataIndexer *self();

    /**
     * Get the index for an attribute or element name, allocating it if necessary.
     *
     * @param nm The name
     * @param inExtension Whether this name belongs within a GPX @c &lt;extensions&gt; element
     * @return the index
     *
     * @note Two names, one belonging within an extension and one without, can
     * be allocated, and will have different index numbers.
     **/
    int index(const QString &nm, bool inExtension = false);

    /**
     * Get the index for an attribute or element name, whether it is an
     * extensions name or not.  The name is not allocated if it does not
     * exist already.
     *
     * @param nm The name
     * @return The index, or -1 if it is not allocated
     **/
    int indexAny(const QString &nm) const;

    /**
     * Get the name allocated for an index.
     *
     * @param idx The index
     * @return the allocated name, or QString::null if the index is not allocated
     **/
    QString name(int idx) const;

    /**
     * Check whether an allocated name belongs within an @c &lt;extensions&gt; element.
     *
     * @param idx The index
     * @return @c true if the name is an extension
     **/
    bool isExtension(int idx) const;

    /**
     * Get the number of indexes that are currently allocated.
     *
     * @return the number, or 0 if none are currently allocated
     **/
    int count() const				{ return (mNextIndex); }

private:
    DataIndexer();
    ~DataIndexer()				{}

private:
    QHash<QString,int> mIndexHash;
    int mNextIndex;
};

 
#endif							// DATAINDEXER_H
