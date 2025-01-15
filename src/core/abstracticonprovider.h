//////////////////////////////////////////////////////////////////////////
//									//
//  Project:	Umbrail - GPX track viewer and editor			//
//									//
//////////////////////////////////////////////////////////////////////////
//									//
//  Copyright (c) 2014-2024 Jonathan Marten <jjm@keelhaul.me.uk>	//
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

#ifndef ABSTRACTICONPROVIDER_H
#define ABSTRACTICONPROVIDER_H

#include <qobject.h>

#include "pointicon.h"


class QIcon;
class TrackDataItem;


class AbstractIconProvider
{
public:
    virtual ~AbstractIconProvider() = default;

    virtual const char *internalName() const = 0;
    virtual QString displayName() const = 0;
    virtual QByteArray metadataKey() const = 0;

    virtual QStringList allIconNames() = 0;

    virtual bool createIcon(QIcon *icon, const QString &name,
                            const QVariant &colour, const QVariant &shape) = 0;

    PointIcon::IconNamespace namespaceId() const	{ return (mNsp); }
    virtual bool supportsShape() const			{ return (false); }

protected:
    AbstractIconProvider();

private:
    PointIcon::IconNamespace mNsp;
};

#endif							// ABSTRACTICONPROVIDER_H
