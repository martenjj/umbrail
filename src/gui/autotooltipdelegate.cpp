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

#include "autotooltipdelegate.h"

#include <qabstractitemview.h>
#include <qevent.h>
#include <qtooltip.h>

// Automatic tool tips for truncated items,
// from http://www.mimec.org/node/337
bool AutoToolTipDelegate::helpEvent(QHelpEvent *e,
                                    QAbstractItemView *view,
                                    const QStyleOptionViewItem &option,
                                    const QModelIndex &index)
{
    if (e==nullptr || view==nullptr) return (false);

    if (e->type()==QEvent::ToolTip)
    {
        if (!index.data(Qt::ToolTipRole).isValid())	// not tip already from model
        {
            QRect rect = view->visualRect(index);	// displayed rectangle
            QSize size = sizeHint(option, index);	// size hint rectangle
            if (rect.width()<size.width())		// display truncated?
            {
                QVariant tooltip = index.data(Qt::DisplayRole);
                if (tooltip.canConvert<QString>())
                {					// show the full string
                    QToolTip::showText(e->globalPos(), tooltip.toString(), view);
                    return (true);
                }
            }

            if (!QStyledItemDelegate::helpEvent(e, view, option, index)) QToolTip::hideText();
            return (true);
        }
    }

    return (QStyledItemDelegate::helpEvent(e, view, option, index));
}
