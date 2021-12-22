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

#include "dataindexer.h"

#include <qhash.h>
#include <qdebug.h>


// These are not POD, but they will not be used
// until the application is fully initialised
// so should be safe.
static QHash<QByteArray,int> sIndexHash;
static QHash<int,QByteArray> sNamespaceHash;
static QHash<QByteArray,QByteArray> sUriHash;

// The XML namespace prefix used by this application.
// The namespace URI is only used by and is set in GpxExporter.
static const char *sApplicationNamespace = PROJECT_NAME;

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


int DataIndexer::index(const QByteArray &nm)
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


QByteArray DataIndexer::name(int idx)
{
    return (sIndexHash.key(idx));
}


int DataIndexer::indexWithNamespace(const QByteArray &nm, const QByteArray &nsp)
{
    const int idx = index(nm);				// look up index as before
    const QByteArray &curnsp = sNamespaceHash.value(idx);
							// get existing namespace
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


int DataIndexer::indexWithNamespace(const QByteArray &qnm)
{
    const int pos = qnm.indexOf(':');
    if (pos==-1) return (index(qnm));			// not a qualified name
							// split up qualified name
    return (indexWithNamespace(qnm.mid(pos+1), qnm.left(pos)));
}


QByteArray DataIndexer::nameWithNamespace(const QByteArray &nm)
{
    return (nameWithNamespace(index(nm)));
}


QByteArray DataIndexer::nameWithNamespace(int idx)
{
    const QByteArray &nm = name(idx);			// plain name for index
    const QByteArray &nsp = sNamespaceHash.value(idx);	// get existing namespace

    if (nsp.isEmpty()) return (nm);			// no associated namespace
    return (nsp+':'+nm);				// name with namespace
}


bool DataIndexer::isApplicationTag(const QByteArray &nm)
{
    const int idx = index(nm);				// look up index
    const QByteArray &nsp = sNamespaceHash.value(idx);	// get existing namespace
    return (nsp==sApplicationNamespace);		// check whether it is our own
}


bool DataIndexer::isInternalTag(const QByteArray &nm)
{
    return (nm=="name" || nm=="latitude" || nm=="longitude");
}


QByteArray DataIndexer::applicationNamespace()
{
    return (sApplicationNamespace);
}


int DataIndexer::count()
{
    return (sIndexHash.count());
}


void DataIndexer::setUriForNamespace(const QByteArray &nsp, const QByteArray &namespaceURI)
{
    if (sUriHash.contains(nsp))				// see if already associated
    {
        const QByteArray &curURI = sUriHash[nsp];	// duplication should never happen
        if (curURI!=namespaceURI) qWarning() << "ignoring namespace URI" << namespaceURI << "because tag" << nsp << "already has" << curURI;
        return;						// do nothing in any case
    }

    qDebug() << "namespace URI" << namespaceURI << "for tag" << nsp;
    sUriHash.insert(nsp, namespaceURI);
}


QByteArray DataIndexer::uriForNamespace(const QByteArray &nsp)
{
    return (sUriHash.value(nsp));
}


QList<QByteArray> DataIndexer::namespacesWithUri()
{
    return (sUriHash.keys());
}
