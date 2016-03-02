
#ifndef ITEMSELECTDIALOGUE_H
#define ITEMSELECTDIALOGUE_H


#include <kdialog.h>
#include "mainwindowinterface.h"

#include "trackdata.h"


class QTreeView;
class QItemSelection;
class TrackFilterModel;
class FilesModel;


class ItemSelectDialogue : public KDialog, public MainWindowInterface
{
    Q_OBJECT

public:
    TrackDataItem *selectedItem() const;
    void setSelectedItem(const TrackDataItem *item);

signals:
    void selectionChanged();

protected:
    explicit ItemSelectDialogue(QWidget *pnt = NULL);
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
