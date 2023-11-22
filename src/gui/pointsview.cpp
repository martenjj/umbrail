
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
