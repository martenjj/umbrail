
#ifndef ITEMSELECTDIALOGUE_H
#define ITEMSELECTDIALOGUE_H


#include <dialogbase.h>

#include "mainwindowinterface.h"

#include "trackdata.h"


class QTreeView;
class QItemSelection;
class TrackFilterModel;
class FilesModel;


class ItemSelectDialogue : public DialogBase, public MainWindowInterface
{
    Q_OBJECT

public:
    TrackDataItem *selectedItem() const;
    void setSelectedItem(const TrackDataItem *item);

signals:
    void selectionChanged();

protected:
    explicit ItemSelectDialogue(QWidget *pnt = nullptr);
    virtual ~ItemSelectDialogue();

    TrackFilterModel *trackModel() const;
    FilesModel *filesModel() const;

protected slots:
    void slotSelectionChanged(const QItemSelection &sel, const QItemSelection &desel);

private slots:
    void slotExpandTree();

private:
    QTreeView *mTrackList;
};

#endif							// ITEMSELECTDIALOGUE_H
