//////////////////////////////////////////////////////////////////////////
//									//
//  Project:	Umbrail - GPX track viewer and editor			//
//									//
//////////////////////////////////////////////////////////////////////////
//									//
//  Copyright (c) 2014-2022 Jonathan Marten <jjm@keelhaul.me.uk>	//
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

#ifndef POINTICON_H
#define POINTICON_H

#include <qicon.h>

class QColor;
class TrackDataItem;

/**
 * @short Provide icons for the GUI and for plotting on the map.
 *
 * @see QIcon
 **/

class PointIcon
{
public:
    enum IconNamespace
    {
        NamespaceAuto,
        NamespaceUnknown,
        NamespaceColour,
        NamespaceSystem,
        NamespaceGarmin,
        NamespaceOsmand
    };

    ~PointIcon() = default;

    QString name() const			{ return (mName); }
    PointIcon::IconNamespace nsp() const	{ return (mNsp); }
    QIcon icon() const				{ return (mIcon); }
    bool isValid() const			{ return (!mIcon.isNull()); }
    QPixmap pixmap(int size) const		{ return (mIcon.pixmap(size)); }

    static QStringList allNames(PointIcon::IconNamespace nsp);
    static QString namespaceDisplayName(PointIcon::IconNamespace nsp);
    static QByteArray namespaceInternalName(PointIcon::IconNamespace nsp);
    static PointIcon::IconNamespace namespaceId(const QByteArray &nsn);

protected:
    // Only the PointIconProvider may construct a PointIcon.
    friend class PointIconProvider;
    explicit PointIcon(const QString &name, PointIcon::IconNamespace nsp, const TrackDataItem *item = nullptr);
    explicit PointIcon(const QString &name, const QColor &col);

private:
    QString mName;
    PointIcon::IconNamespace mNsp;
    QIcon mIcon;
};

#endif							// POINTICON_H
