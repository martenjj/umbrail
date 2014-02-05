// -*-mode:c++ -*-

#ifndef FILESVIEW_H
#define FILESVIEW_H
 
#include <qtreeview.h>

#include "trackdata.h"


class KConfigGroup;

class MainWindow;


class FilesView : public QTreeView
{
    Q_OBJECT

public:
    FilesView(QWidget *pnt = NULL);
    ~FilesView();

    void readProperties(const KConfigGroup &grp);
    void saveProperties(KConfigGroup &grp);

//    typedef QList<int> RowList;
//    FilesView::RowList selectedRows() const;
//    void selectRows(const QList<int> &rows);
//    void selectRows(int fromRow, int toRow);

    int selectedCount() const			{ return (mSelectedCount); }
    TrackData::Type selectedType() const	{ return (mSelectedType); }
    const TrackDataItem *selectedItem() const	{ return (mSelectedItem); }
    QList<TrackDataItem *> selectedItems() const;

public slots:
    void slotSelectAllSiblings();

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
};
 
#endif							// FILESVIEW_H
