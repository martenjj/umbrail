
#ifndef MOVEITEMDIALOGUE_H
#define MOVEITEMDIALOGUE_H


#include <kdialog.h>
#include "mainwindowinterface.h"

#include "trackdata.h"


class QTreeView;
class QItemSelection;


class MoveItemDialogue : public KDialog, public MainWindowInterface
{
    Q_OBJECT

public:
    explicit MoveItemDialogue(QWidget *pnt = NULL);
    virtual ~MoveItemDialogue();

    void setSource(const QList<TrackDataItem *> *items);
    TrackDataItem *selectedDestination() const;
    void selectDestination(const TrackDataItem *item);

signals:
    void selectionChanged();

protected:
    void setMode(TrackData::Type mode);

protected slots:
    void slotSelectionChanged(const QItemSelection &sel, const QItemSelection &desel);

private:
    QTreeView *mTrackList;
};

#endif							// MOVEITEMDIALOGUE_H
