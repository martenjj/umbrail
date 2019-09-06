
#ifndef MOVEITEMDIALOGUE_H
#define MOVEITEMDIALOGUE_H


#include "itemselectdialogue.h"

class TrackDataItem;


class MoveItemDialogue : public ItemSelectDialogue
{
    Q_OBJECT

public:
    explicit MoveItemDialogue(QWidget *pnt = nullptr);
    virtual ~MoveItemDialogue()				{}

    void setSource(const QList<TrackDataItem *> *items);
};

#endif							// MOVEITEMDIALOGUE_H
