#ifndef POINTSVIEW_H
#define POINTSVIEW_H
 
#include <qtreeview.h>
#include "applicationdatainterface.h"


class TrackDataItem;


class PointsView : public QTreeView, public ApplicationDataInterface
{
    Q_OBJECT

public:
    PointsView(QWidget *pnt = nullptr);
    virtual ~PointsView();

    void readProperties();
    void saveProperties();

public slots:
    void slotSelectPoints(unsigned long selectionId);

protected slots:
    virtual void selectionChanged(const QItemSelection &sel, const QItemSelection &desel) override;

protected:
    virtual void contextMenuEvent(QContextMenuEvent *ev) override;

signals:
    void pointsSelectionChanged(const QList<const TrackDataItem *> &items);

private:
    bool mSelectionInhibit;
};
 
#endif							// POINTSVIEW_H
