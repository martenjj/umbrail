//////////////////////////////////////////////////////////////////////////
//									//
//  Project:	NavTracks						//
//  Edit:	15-Sep-21						//
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

class QString;


/**
 * @short Maintains data indexes for file, track/segment and point metadata.
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
 * XML namespaces, either for known tags or "learned" while importing, are
 * automatically associated with the names to which they apply.
 *
 * @author Jonathan Marten
 **/

namespace DataIndexer
{
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
     * @return the allocated name, or a null @c QString if the index is not allocated
     **/
    QString name(int idx);

    /**
     * Get the number of indexes that are currently allocated.
     *
     * @return the number, or 0 if none are currently allocated
     **/
    int count();

    /**
     * Note that an XML namespace tag is associated with a name.
     *
     * @param nm The name
     * @param nsp The XML namespace tag
     * @return the existing or a newly allocated index
     *
     * @note This is intended to be used during GPX or KML import,
     * so that the namespace associated with a tag will be remembered
     * for export.
     **/
    int indexWithNamespace(const QString &nm, const QString &nsp);

    /**
     * Note that an XML namespace tag is associated with a name.
     *
     * @param nm The qualified name, in the format "namespace:name"
     * @return the existing or a newly allocated index
     **/
    int indexWithNamespace(const QString &qnm);

    /**
     * Get a full name including an XML namespace tag.
     *
     * @param nm The plain internal name
     * @return the name with an XML namespace prepended, if there is one,
     * otherwise the name unchanged.
     *
     * @note This is intended to be used during GPX or KML export,
     * so that the namespace that was previously found to be associated
     * with a tag will be used.
     **/
    QString nameWithNamespace(const QString &nm);

    /**
     * Get a full name including an XML namespace tag.
     *
     * @param idx The index
     * @return the name with an XML namespace prepended, if there is one,
     * otherwise the plain name.
     **/
    QString nameWithNamespace(int idx);

    /**
     * Check whether the tag should be in our application namespace.
     *
     * @param nm The plain internal name
     * @return @c true if the name should be namespaced as such
     **/
    bool isApplicationTag(const QString &nm);

    /**
     * Check whether the tag is internal to this application only.
     *
     * @param nm The plain internal name
     * @return @c true if this is an internal tag
     **/
    bool isInternalTag(const QString &nm);

    /**
     * Get our application namespace name
     *
     * @return the namespace name
     **/
    QString applicationNamespace();
}

 
#endif							// DATAINDEXER_H
