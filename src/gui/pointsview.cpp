
#include "pointsview.h"

#include <qheaderview.h>
#include <qsortfilterproxymodel.h>
#include <qevent.h>
#include <qmenu.h>
#include <qdebug.h>
#include <qitemselectionmodel.h>
#include <qtimer.h>

#include <klocalizedstring.h>
#include <kxmlguiwindow.h>
#include <kxmlguifactory.h>

#include "settings.h"
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

    mSelectionTimer = new QTimer(this);
    mSelectionTimer->setSingleShot(true);
    mSelectionTimer->setInterval(50);
    connect(mSelectionTimer, &QTimer::timeout, this, &PointsView::slotCheckSelection);
    mSelectionBusy = false;
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


void PointsView::contextMenuEvent(QContextMenuEvent *ev)
{
    KXmlGuiWindow *xmlwin = qobject_cast<KXmlGuiWindow *>(mainWidget());
    Q_ASSERT(xmlwin!=nullptr);
    QMenu *popup = static_cast<QMenu *>(xmlwin->factory()->container("pointsview_contextmenu", xmlwin));
    if (popup!=nullptr) popup->exec(ev->globalPos());
}


void PointsView::slotCheckSelection()
{
    // A selection from the FilesView is passed to us via the
    // KLinkItemSelectionModel.  However, because its master model
    // (the FilesModel) has only one column then that selection
    // only selects the first column - our column count and
    // selectionBehavior() are ignored.  This slot is called after
    // a timeout when the selection has beenb changed, and adjusts
    // the current selection to ensure that full rows are selected.
    // Recursive invocation is guarded by mSelectionBusy.

    QItemSelectionModel *selMod = selectionModel();
    const QItemSelection oldSel = selMod->selection();

    QItemSelection newSel;
    for (const QItemSelectionRange &r : oldSel)
    {
        // Accept only those current selections that start in column 0.
        if (r.left()==0) newSel.append(r);
    }

    mSelectionBusy = true;
    // This must be called unconditionally so that the selection is
    // correctly cleared, if that is what is happening.
    selMod->select(newSel, QItemSelectionModel::Clear|QItemSelectionModel::Select|QItemSelectionModel::Rows);
    mSelectionBusy = false;
}


void PointsView::selectionChanged(const QItemSelection &sel, const QItemSelection &desel)
{
    QAbstractItemView::selectionChanged(sel, desel);
    if (!mSelectionBusy) mSelectionTimer->start();
}
