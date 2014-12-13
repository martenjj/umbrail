
#ifndef MOVEITEMDIALOGUE_H
#define MOVEITEMDIALOGUE_H


#include <kdialog.h>


class QTreeView;
class QItemSelection;
class TrackDataItem;
class FilesController;



class MoveItemDialogue : public KDialog
{
    Q_OBJECT

public:
    explicit MoveItemDialogue(FilesController *fc, QWidget *pnt = NULL);
    virtual ~MoveItemDialogue();

    void setSource(const QList<TrackDataItem *> *items);
    TrackDataItem *selectedDestination() const;

protected slots:
    void slotSelectionChanged(const QItemSelection &sel, const QItemSelection &desel);

private:
    QTreeView *mTrackList;
};

#endif							// MOVEITEMDIALOGUE_H
