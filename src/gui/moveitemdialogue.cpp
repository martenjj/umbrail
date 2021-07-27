
#include "moveitemdialogue.h"

#include <klocalizedstring.h>

#include "trackfiltermodel.h"


MoveItemDialogue::MoveItemDialogue(QWidget *pnt)
    : ItemSelectDialogue(pnt)
{
    setObjectName("MoveItemDialogue");

    setWindowTitle(i18nc("@title:window", "Move Item"));
    setButtons(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);

    setButtonEnabled(QDialogButtonBox::Ok, false);
    setButtonText(QDialogButtonBox::Ok, i18nc("@action:button", "Move"));
}


void MoveItemDialogue::setSource(const QList<TrackDataItem *> *items)
{
    trackModel()->setSource(items);
}
