
#include "pointsview.h"

#include <qheaderview.h>
#include <qsortfilterproxymodel.h>
#include <qevent.h>
#include <qmenu.h>
#include <qdebug.h>

#include <klocalizedstring.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kxmlguiwindow.h>
#include <kxmlguifactory.h>

#include "autotooltipdelegate.h"


// #define GROUP_POINTSVIEW	"PointsView"
// #define CONFIG_COLSTATES	"ColumnStates"



PointsView::PointsView(QWidget *pnt)
    : QTreeView(pnt),
      ApplicationDataInterface(pnt)
{
    qDebug();

    setObjectName("PointsView");

    setRootIsDecorated(false);
    setSortingEnabled(true);
    setAlternatingRowColors(true);
    setUniformRowHeights(true);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setAllColumnsShowFocus(true);

    setHeaderHidden(false);
    header()->setStretchLastSection(true);
    header()->setDefaultSectionSize(100);
    header()->setSectionResizeMode(QHeaderView::Interactive);
    header()->setSortIndicator(0, Qt::AscendingOrder);

    setItemDelegate(new AutoToolTipDelegate(this));
}


PointsView::~PointsView()
{
    qDebug() << "done";
}


void PointsView::readProperties()
{
//     KConfigGroup ourGroup = grp.config()->group(GROUP_POINTSVIEW);
//     qDebug() << "from" << ourGroup.name();
// 
//     QString colStates = ourGroup.readEntry(CONFIG_COLSTATES, QString());
//     if (!colStates.isEmpty())
//     {
//         qDebug() << "have col states size" << QByteArray::fromHex(colStates.toLocal8Bit()).size();
//         qDebug() << "restored?" << header()->restoreState(QByteArray::fromHex(colStates.toLocal8Bit()));
//     }
}


void PointsView::saveProperties()
{
//     KConfigGroup ourGroup = grp.config()->group(GROUP_POINTSVIEW);
//     qDebug() << "to" << ourGroup.name();
// 
//     const int viewportWidth = viewport()->width();
//     const int headerLength = header()->length();
//     if (headerLength>(viewportWidth+10))
//     {
//         qDebug() << "header length" << headerLength << "inconsistent with viewport width" << viewportWidth;
//         qDebug() << "not saving column states";
//         return;
//     }
// 
//     qDebug() << "save col states size" << header()->saveState().size();
//     ourGroup.writeEntry(CONFIG_COLSTATES, header()->saveState().toHex());
//     ourGroup.sync();
}


void PointsView::selectionChanged(const QItemSelection &sel,
                                  const QItemSelection &desel)
{
    qDebug() << "$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$" << sel << desel;

    QTreeView::selectionChanged(sel, desel);		// visually update
    emit updateActionState();				// actions and status
}


#if 0
// Map the selection in the proxy model to the base model.
// This contains one model index for each selected item (all
// columns), so consider only those for the first column.
// Return the row index in the base model of each.
PointsView::RowList PointsView::selectedRows() const
{
    PointsView::RowList result;

    // Could also do this with
    //
    //   QModelIndexList indexes = mProxyModel->mapSelectionToSource(
    //                               selectionModel()->selection()).indexes();
    //
    // but, since we have to iterate over the returned list anyway,
    // it should be more efficient to filter the list for column 0
    // first and then do the model mapping.

    QAbstractProxyModel *pm = static_cast<QAbstractProxyModel *>(model());
    QModelIndexList indexes = selectedIndexes();
    for (QModelIndexList::const_iterator it = indexes.constBegin();
         it!=indexes.constEnd(); ++it)
    {
        QModelIndex idx = (*it);
        if (idx.column()==0) result.append(pm->mapToSource(idx).row());
    }

    return (result);
}


void PointsView::selectRows(const QList<int> &rows)
{
    qDebug() << rows;

    selectionModel()->reset();
    if (!rows.isEmpty())
    {
        QAbstractProxyModel *pm = static_cast<QAbstractProxyModel *>(model());
        QAbstractItemModel *sm = pm->sourceModel();

        for (int i = 0; i<rows.count(); ++i)
        {
            qDebug() << "selecting row" << rows[i];
            selectionModel()->select(pm->mapFromSource(sm->index(rows[i], 0)),
                                     QItemSelectionModel::Select|QItemSelectionModel::Rows);
        }

        scrollTo(pm->mapFromSource(sm->index(rows.first(), 0)));
    }
}


void PointsView::selectRows(int fromRow, int toRow)
{
    qDebug() << fromRow << "-" << toRow;

    QAbstractProxyModel *pm = static_cast<QAbstractProxyModel *>(model());
    QAbstractItemModel *sm = pm->sourceModel();

    selectionModel()->reset();
    for (int i = fromRow; i<=toRow; ++i)
    {
        qDebug() << "selecting row" << i;
        selectionModel()->select(pm->mapFromSource(sm->index(i, 0)),
                                 QItemSelectionModel::Select|QItemSelectionModel::Rows);
    }

    scrollTo(pm->mapFromSource(sm->index(fromRow, 0)));
}
#endif



void PointsView::contextMenuEvent(QContextMenuEvent *ev)
{
    qDebug() << "$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$";

    KXmlGuiWindow *xmlwin = qobject_cast<KXmlGuiWindow *>(mainWidget());
    Q_ASSERT(xmlwin!=nullptr);
    QMenu *popup = static_cast<QMenu *>(xmlwin->factory()->container("pointsview_contextmenu", xmlwin));
    if (popup!=NULL) popup->exec(ev->globalPos());
}
