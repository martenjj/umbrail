//////////////////////////////////////////////////////////////////////////
//									//
//  Project:	NavTracks						//
//  Edit:	24-Feb-14						//
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
     * @return the existing or a newly allocated index
     **/
    int index(const QString &nm);

    /**
     * Get the name allocated for an index.
     *
     * @param idx The index
     * @return the allocated name, or @c QString::null if the index is not allocated
     **/
    QString name(int idx) const;

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
