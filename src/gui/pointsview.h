#ifndef POINTSVIEW_H
#define POINTSVIEW_H
 
#include <qtreeview.h>
#include "applicationdatainterface.h"


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
};
 
#endif							// POINTSVIEW_H
