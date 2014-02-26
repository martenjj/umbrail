// -*-mode:c++ -*-

#ifndef FILESVIEW_H
#define FILESVIEW_H
 
#include <qtreeview.h>

#include "trackdata.h"


class MainWindow;


class FilesView : public QTreeView
{
    Q_OBJECT

public:
    FilesView(QWidget *pnt = NULL);
    ~FilesView();

    void readProperties();
    void saveProperties();

    int selectedCount() const			{ return (mSelectedCount); }
    TrackData::Type selectedType() const	{ return (mSelectedType); }
    const TrackDataItem *selectedItem() const	{ return (mSelectedItem); }
    QList<TrackDataItem *> selectedItems() const;

    unsigned long selectionId() const		{ return (mSelectionId); }

public slots:
    void slotSelectAllSiblings();
    void slotClickedItem(const QModelIndex &index, unsigned int flags);

protected:
    void selectionChanged(const QItemSelection &sel, const QItemSelection &desel);
    virtual void contextMenuEvent(QContextMenuEvent *ev);

signals:
    void updateActionState();

private:
    MainWindow *mainWindow() const		{ return (mMainWindow); }

private:
    MainWindow *mMainWindow;

    int mSelectedCount;
    TrackData::Type mSelectedType;
    const TrackDataItem *mSelectedItem;
    unsigned long mSelectionId;
};
 
#endif							// FILESVIEW_H
