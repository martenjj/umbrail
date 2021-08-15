// -*-mode:c++ -*-

#ifndef FILESVIEW_H
#define FILESVIEW_H
 
#include <qtreeview.h>
#include "applicationdatainterface.h"

#include "trackdata.h"


class FilesView : public QTreeView, public ApplicationDataInterface
{
    Q_OBJECT

public:
    FilesView(QWidget *pnt = nullptr);
    virtual ~FilesView();

    void readProperties();
    void saveProperties();

    int selectedCount() const			{ return (mSelectedCount); }
    TrackData::Type selectedType() const	{ return (mSelectedType); }
    const TrackDataItem *selectedItem() const	{ return (mSelectedItem); }
    QList<TrackDataItem *> selectedItems() const;
    QVector<const TrackDataAbstractPoint *> selectedPoints() const;

    void selectItem(const TrackDataItem *item, bool combine = false);
    void setMovePointsMode(bool on);

    unsigned long selectionId() const		{ return (mSelectionId); }

public slots:
    void slotSelectAllSiblings();
    void slotClickedItem(const QModelIndex &index, unsigned int flags);

    void slotCollapseAll();
    void slotExpandAll();

protected:
    void selectionChanged(const QItemSelection &sel, const QItemSelection &desel) override;
    void contextMenuEvent(QContextMenuEvent *ev) override;

signals:
    void updateActionState();

private:
    void expandItem(const QModelIndex &idx);

private:
    int mSelectedCount;
    TrackData::Type mSelectedType;
    const TrackDataItem *mSelectedItem;
    unsigned long mSelectionId;
};
 
#endif							// FILESVIEW_H