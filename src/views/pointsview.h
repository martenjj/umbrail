#ifndef POINTSVIEW_H
#define POINTSVIEW_H
 
#include <qtreeview.h>
#include "applicationdatainterface.h"


class QTimer;


class PointsView : public QTreeView, public ApplicationDataInterface
{
    Q_OBJECT

public:
    PointsView(QWidget *pnt = nullptr);
    virtual ~PointsView() = default;

    void readProperties();
    void saveProperties();

protected:
    virtual void contextMenuEvent(QContextMenuEvent *ev) override;

protected slots:
    void selectionChanged(const QItemSelection &sel, const QItemSelection &desel) override;

private slots:
    void slotCheckSelection();

private:
    QTimer *mSelectionTimer;
    bool mSelectionBusy;
};
 
#endif							// POINTSVIEW_H
