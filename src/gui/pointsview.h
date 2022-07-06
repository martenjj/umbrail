#ifndef POINTSVIEW_H
#define POINTSVIEW_H
 
#include <qtreeview.h>
#include "applicationdatainterface.h"


class PointsView : public QTreeView, public ApplicationDataInterface
{
    Q_OBJECT

public:
    PointsView(QWidget *pnt = nullptr);
    virtual ~PointsView();

    void readProperties();
    void saveProperties();

//     typedef QList<int> RowList;
//     PointsView::RowList selectedRows() const;
//     void selectRows(const QList<int> &rows);
//     void selectRows(int fromRow, int toRow);

protected slots:
    virtual void selectionChanged(const QItemSelection &sel, const QItemSelection &desel) override;

protected:
    virtual void contextMenuEvent(QContextMenuEvent *ev) override;

signals:
    void updateActionState();
};
 
#endif							// POINTSVIEW_H
