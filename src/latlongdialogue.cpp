
#include "latlongdialogue.h"

#include <klocalizedstring.h>

#include "latlongwidget.h"


LatLongDialogue::LatLongDialogue(QWidget *pnt)
    : DialogBase(pnt)
{
    setObjectName("LatLongDialogue");
    setWindowTitle(i18n("Edit Position"));
    setButtons(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);

    mWidget = new LatLongWidget(this);
    connect(mWidget, SIGNAL(positionChanged(double, double)), SLOT(slotUpdateButtonState()));

    setMainWidget(mWidget);
    setMinimumWidth(400);
    setMaximumWidth(440);
}


void LatLongDialogue::setLatLong(double lat, double lon)
{
    mWidget->setLatLong(lat, lon);
    slotUpdateButtonState();				// verify acceptable values
}


void LatLongDialogue::slotUpdateButtonState()
{
    setButtonEnabled(QDialogButtonBox::Ok, mWidget->hasAcceptableInput());
}


double LatLongDialogue::latitude() const
{
    return (mWidget->latitude());
}


double LatLongDialogue::longitude() const
{
    return (mWidget->longitude());
}
