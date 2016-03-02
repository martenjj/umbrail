
#include "moveitemdialogue.h"

#include <klocale.h>

#include "trackfiltermodel.h"


MoveItemDialogue::MoveItemDialogue(QWidget *pnt)
    : ItemSelectDialogue(pnt)
{
    setObjectName("MoveItemDialogue");

    setCaption(i18nc("@title:window", "Move Item"));
    setButtons(KDialog::Ok|KDialog::Cancel);
    enableButtonOk(false);
    setButtonText(KDialog::Ok, i18nc("@action:button", "Move"));
}


void MoveItemDialogue::setSource(const QList<TrackDataItem *> *items)
{
    trackModel()->setSource(items);
}
