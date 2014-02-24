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

#include "dataindexer.h"

#include <kdebug.h>



static DataIndexer *sInstance = NULL;



DataIndexer::DataIndexer()
{
    kDebug();
    mNextIndex = 0;
}


DataIndexer *DataIndexer::self()
{
    if (sInstance==NULL) sInstance = new DataIndexer();
    return (sInstance);
}



int DataIndexer::index(const QString &nm, bool inExtension)
{
    QString nm1 = nm;
    if (inExtension) nm1 += "*";
    int idx = mIndexHash.value(nm1, -1);
    if (idx==-1)					// nothing allocated yet
    {
        idx = mNextIndex++;				// next integer value
        mIndexHash.insert(nm1, idx);
        kDebug() << "allocated index" << idx << "for" << nm1;
    }

    return (idx);					// existing or new index
}


int DataIndexer::indexAny(const QString &nm) const
{
    QString nm1 = nm;
    if (mIndexHash.contains(nm1)) return (mIndexHash.value(nm1));
    nm1 += "*";
    if (mIndexHash.contains(nm1)) return (mIndexHash.value(nm1));
    return (-1);
}



QString DataIndexer::name(int idx) const
{
    QString nm = mIndexHash.key(idx);
    if (nm.endsWith(QLatin1Char('*'))) nm.chop(1);
    return (nm);
}



bool DataIndexer::isExtension(int idx) const
{
    QString nm = mIndexHash.key(idx);
    return (nm.endsWith(QLatin1Char('*')));
}