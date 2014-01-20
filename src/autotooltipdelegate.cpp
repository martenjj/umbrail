
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
    if (e==NULL || view==NULL) return (false);

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
