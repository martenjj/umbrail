
#include "pointsview.h"

#include <qheaderview.h>
#include <qsortfilterproxymodel.h>
#include <qevent.h>
#include <qmenu.h>
#include <qdebug.h>
#include <qitemselectionmodel.h>

#include <klocalizedstring.h>
#include <kxmlguiwindow.h>
#include <kxmlguifactory.h>

#include "settings.h"
#include "pointsmodel.h"
#include "autotooltipdelegate.h"
#include "trackdata.h"


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

    mSelectionInhibit = false;
}


PointsView::~PointsView()
{
    qDebug() << "done";
}


void PointsView::readProperties()
{
    QString colStates = Settings::pointsViewColumnStates();
    if (!colStates.isEmpty())
    {
        header()->restoreState(QByteArray::fromHex(colStates.toLatin1()));
    }
}


void PointsView::saveProperties()
{
    // Not sure why this workaround was needed,
    // but it doesn't seem to happen now.
    const int viewportWidth = viewport()->width();
    const int headerLength = header()->length();
    if (headerLength>(viewportWidth+10))
    {
        qWarning() << "header length" << headerLength << "inconsistent with viewport width" << viewportWidth;
        qWarning() << "not saving column states";
        return;
    }

    Settings::setPointsViewColumnStates(header()->saveState().toHex());
}


void PointsView::selectionChanged(const QItemSelection &sel,
                                  const QItemSelection &desel)
{
    QTreeView::selectionChanged(sel, desel);		// visually update

    QAbstractProxyModel *intermediateModel = qobject_cast<QAbstractProxyModel *>(model());
    Q_ASSERT(intermediateModel!=nullptr);
    PointsModel *pointsModel = qobject_cast<PointsModel *>(intermediateModel->sourceModel());
    Q_ASSERT(pointsModel!=nullptr);

    QList<const TrackDataItem *> selectedPoints;
    const QModelIndexList selectedIndexes = selectionModel()->selectedRows();
    qDebug() << "selected" << selectedIndexes.count();

    for (const QModelIndex &idx : selectedIndexes)
    {
        const TrackDataItem *item = pointsModel->itemAt(intermediateModel->mapToSource(idx).row());
        if (item!=nullptr) selectedPoints.append(item);
    }

    // Tell the tree view to make the same selection.
    if (!mSelectionInhibit) emit pointsSelectionChanged(selectedPoints);
    // There is no need for an updateActionState() or any signal
    // connection, FilesView::selectionChanged() will do that.
}


void PointsView::contextMenuEvent(QContextMenuEvent *ev)
{
    KXmlGuiWindow *xmlwin = qobject_cast<KXmlGuiWindow *>(mainWidget());
    Q_ASSERT(xmlwin!=nullptr);
    QMenu *popup = static_cast<QMenu *>(xmlwin->factory()->container("pointsview_contextmenu", xmlwin));
    if (popup!=nullptr) popup->exec(ev->globalPos());
}


void PointsView::slotSelectPoints(unsigned long selectionId)
{
    qDebug() << "selection ID is now" << selectionId;

    QAbstractProxyModel *intermediateModel = qobject_cast<QAbstractProxyModel *>(model());
    Q_ASSERT(intermediateModel!=nullptr);
    PointsModel *pointsModel = qobject_cast<PointsModel *>(intermediateModel->sourceModel());
    Q_ASSERT(pointsModel!=nullptr);

    const int num = pointsModel->rowCount(QModelIndex());
    QItemSelectionModel *selModel = selectionModel();

    // This slot is intended to be called via the interconnection signal
    // filesViewSelectionChanged().  So as not to recursively call each
    // other, emitting the pointsViewSelectionChanged() signal is blocked
    // while the selection is changed.  FilesView::selectionChanged() will
    // still be called to update the selection ID and GUI actions.
    mSelectionInhibit = true;

    selModel->clear();
    for (int i = 0; i<num; ++i)
    {
        const TrackDataItem *item = pointsModel->itemAt(i);
        if (item->selectionId()!=selectionId) continue;

        QModelIndex idx = pointsModel->index(i, 0);
        selModel->select(intermediateModel->mapFromSource(idx), QItemSelectionModel::Select|QItemSelectionModel::Rows);
    }

    mSelectionInhibit = false;
}
