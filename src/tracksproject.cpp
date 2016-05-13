//////////////////////////////////////////////////////////////////////////
//									//
//  Project:	Utility library						//
//  Edit:	13-May-16						//
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

#include "tracksproject.h"

#include <qdebug.h>

#include <kconfig.h>
#include <kconfiggroup.h>


#define FILES_GROUP		"Files"


TracksProject::TracksProject()
    : Project()
{
    qDebug();
}


TracksProject::~TracksProject()
{
}


QString TracksProject::save(KConfig *conf)
{
    qDebug();

    KConfigGroup grp = conf->group(FILES_GROUP);
    for (int i = 0; i<mFileList.count(); ++i)
    {
        const KUrl file = mFileList[i];
        qDebug() << file;
        grp.writeEntry(QString::number(i), file);
    }

    return (Project::save(conf));
}


QString TracksProject::load(const KConfig *conf)
{
    qDebug();

    const KConfigGroup grp = conf->group(FILES_GROUP);
    for (int i = 0; ; ++i)
    {
        KUrl file = grp.readEntry(QString::number(i), KUrl());
        if (!file.isValid()) break;
        qDebug() << file;
        mFileList.append(file);
    }

    return (Project::load(conf));
}


void TracksProject::clear()
{
    Project::clear();
    mFileList.clear();
}


void TracksProject::addFile(const KUrl &file)
{
    qDebug() << file;

    if (mFileList.contains(file)) mFileList.removeAll(file);
    mFileList.append(file);
    setModified();
}
