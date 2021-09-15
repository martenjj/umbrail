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

#include "dataindexer.h"

#include <qhash.h>
#include <qdebug.h>


// These are not POD, but they will not be used
// until the application is fully initialised
// so should be safe.
static QHash<QString,int> sIndexHash;
static QHash<int,QString> sNamespaceHash;

// The XML namespace prefix used by this application.
// The namespace URI is only used by and is set in GpxExporter.
static const char *sApplicationNamespace = "navtracks";

// Private tags which are used by this application.
// They are namespaced with our prefix when exporting.
static const char *sApplicationTags[] =
{
    "status",
    "source",
    "stop",
    "folder",
    "linecolor",
    "pointcolor",
    "bearingline",
    "rangering",
    nullptr
};


int DataIndexer::index(const QString &nm)
{
    if (nm.contains(':'))				// only plain names allowed here
    {
        qWarning() << "called for name" << nm << "with namespace";
        return (indexWithNamespace(nm));		// should not recurse
    }

    int idx = sIndexHash.value(nm, -1);
    if (idx==-1)					// nothing allocated yet
    {
        idx = sIndexHash.count();			// next integer value
        sIndexHash.insert(nm, idx);
        qDebug() << "allocated index" << idx << "for" << nm;

        // See if this name belongs to our application namespace.
        // If so, associate the namespace now so that it is not
        // overridden by "legacy" files being loaded.
        for (const char **tag = &sApplicationTags[0]; *tag!=nullptr; ++tag)
        {
            if (nm==*tag)
            {
                sNamespaceHash.insert(idx, sApplicationNamespace);
                qDebug() << "associated application namespace";
                break;
            }
        }
    }

    return (idx);					// existing or new index
}


QString DataIndexer::name(int idx)
{
    return (sIndexHash.key(idx));
}


int DataIndexer::indexWithNamespace(const QString &nm, const QString &nsp)
{
    const int idx = index(nm);				// look up index as before
    const QString &curnsp = sNamespaceHash.value(idx);	// get existing namespace

    if (curnsp.isEmpty())				// no existing namespace
    {
        if (!nsp.isEmpty())				// and a new one provided
        {
            sNamespaceHash.insert(idx, nsp);
            qDebug() << "associated namespace" << nsp << "for tag" << nm;
        }
    }
    else						// there is an existing namespace
    {
        if (nsp!=curnsp)				// if not the same as current
        {						// then ignore the new namespace
            qWarning() << "ignoring namespace" << nsp << "because tag" << nm << "already has" << curnsp;
        }
    }

    return (idx);
}


int DataIndexer::indexWithNamespace(const QString &qnm)
{
    const int pos = qnm.indexOf(':');
    if (pos==-1) return (index(qnm));			// not a qualified name
							// split up qualified name
    return (indexWithNamespace(qnm.mid(pos+1), qnm.left(pos)));
}


QString DataIndexer::nameWithNamespace(const QString &nm)
{
    return (nameWithNamespace(index(nm)));
}


QString DataIndexer::nameWithNamespace(int idx)
{
    const QString &nm = name(idx);			// plain name for index
    const QString &nsp = sNamespaceHash.value(idx);	// get existing namespace

    if (nsp.isEmpty()) return (nm);			// no associated namespace
    return (nsp+':'+nm);				// name with namespace
}


bool DataIndexer::isApplicationTag(const QString &nm)
{
    const int idx = index(nm);				// look up index
    const QString &nsp = sNamespaceHash.value(idx);	// get existing namespace
    return (nsp==sApplicationNamespace);		// check whether it is our own
}


bool DataIndexer::isInternalTag(const QString &nm)
{
    return (nm=="name" || nm=="latitude" || nm=="longitude");
}


QString DataIndexer::applicationNamespace()
{
    return (sApplicationNamespace);
}


int DataIndexer::count()
{
    return (sIndexHash.count());
}
