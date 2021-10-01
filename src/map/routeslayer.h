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

#ifndef ROUTESLAYER_H
#define ROUTESLAYER_H
 
#include <layerbase.h>


class RoutesLayer : public LayerBase
{
    Q_OBJECT

public:
    explicit RoutesLayer(QWidget *pnt = nullptr);
    virtual ~RoutesLayer();

    qreal zValue() const override		{ return (3.0); }
    QString id() const override			{ return ("routes"); }
    QString name() const override		{ return (i18n("Routes")); }

protected:
    bool isApplicableItem(const TrackDataItem *item) const override;
    bool isDirectContainer(const TrackDataItem *item) const override;
    bool isIndirectContainer(const TrackDataItem *item) const override;

    void doPaintItem(const TrackDataItem *item, GeoPainter *painter, bool isSelected) const override;
    void doPaintDrag(const SelectionRun *run, GeoPainter *painter) const override;
};

#endif							// ROUTESLAYER_H
