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

#include "project.h"

#include <qdebug.h>

//#include <kurl.h>
#include <klocalizedstring.h>


Project::Project()
{
    qDebug();
    clear();
}


Project::~Project()
{
}


void Project::setModified(bool mod)
{
    qDebug() << "mod=" << mod;
    mModified = mod;
}


void Project::setFileName(const QUrl &file)
{
    qDebug() << "file=" << file;
    mSaveFile = file;
}


QString Project::save(KConfig *conf)
{
    qDebug();
    return (QString::null);
}


QString Project::load(const KConfig *conf)
{
    qDebug();
    return (QString::null);
}


QString Project::name(bool onlyIfValid) const
{
    if (mSaveFile.isValid()) return (mSaveFile.fileName());
    else return (onlyIfValid ? QString::null : i18n("Untitled"));
}


void Project::clear()
{
    mSaveFile = QUrl();
    mModified = false;
}
