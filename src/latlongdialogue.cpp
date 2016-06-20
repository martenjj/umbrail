
#include "latlongdialogue.h"

#include <kdialog.h>
#include <klocale.h>
#include <kconfiggroup.h>
#include <kglobal.h>

#include "latlongwidget.h"


LatLongDialogue::LatLongDialogue(QWidget *pnt)
    : KDialog(pnt)
{
    setObjectName("LatLongDialogue");
    setCaption(i18n("Edit Position"));
    setButtons(KDialog::Ok|KDialog::Cancel);
    showButtonSeparator(false);

    mWidget = new LatLongWidget(this);
    connect(mWidget, SIGNAL(positionChanged(double, double)), SLOT(slotUpdateButtonState()));

    setMainWidget(mWidget);
    setMinimumWidth(400);

    KConfigGroup grp = KGlobal::config()->group(objectName());
    restoreDialogSize(grp);
}


LatLongDialogue::~LatLongDialogue()
{
    KConfigGroup grp = KGlobal::config()->group(objectName());
    saveDialogSize(grp);
}


void LatLongDialogue::setLatLong(double lat, double lon)
{
    mWidget->setLatLong(lat, lon);
    slotUpdateButtonState();				// verify acceptable values
}


void LatLongDialogue::slotUpdateButtonState()
{
    enableButtonOk(mWidget->hasAcceptableInput());
}


double LatLongDialogue::latitude() const
{
    return (mWidget->latitude());
}


double LatLongDialogue::longitude() const
{
    return (mWidget->longitude());
}
