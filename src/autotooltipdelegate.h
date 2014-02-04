// -*-mode:c++ -*-

#ifndef AUTOTOOLTIPDELEGATEVIEW_H
#define AUTOTOOLTIPDELEGATEVIEW_H
 
#include <qstyleditemdelegate.h>


class AutoToolTipDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    AutoToolTipDelegate(QObject *pnt) : QStyledItemDelegate(pnt) {};
    virtual ~AutoToolTipDelegate() {};

public slots:
    bool helpEvent(QHelpEvent *e, QAbstractItemView *view,
                   const QStyleOptionViewItem &option,
                   const QModelIndex &index);
};

#endif							// AUTOTOOLTIPDELEGATEVIEW_H
