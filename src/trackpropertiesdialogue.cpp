
#include "trackpropertiesdialogue.h"

#include <qlayout.h>
#include <qlabel.h>

#include <kdebug.h>
#include <klocale.h>
#include <kpagedialog.h>

#include "trackdata.h"


TrackPropertiesDialogue::TrackPropertiesDialogue(const QList<TrackDataItem *> &items, QWidget *pnt)
    : KPageDialog(pnt)
{
    setObjectName("TrackPropertiesDialogue");

    setModal(true);
    setButtons(KDialog::Ok|KDialog::Cancel);
    setFaceType(KPageDialog::Tabbed);
    showButtonSeparator(false);

    Q_ASSERT(!items.isEmpty());
    TrackDataItem *item = items.first();

    QWidget *w;
    w = item->createPropertiesGeneralPage(items, this);
    connect(w, SIGNAL(enableButtonOk(bool)), SLOT(enableButtonOk(bool)));
    addPage(w, i18nc("@title:tab", "General"));
//    w = item->createPropertiesStylePage(items, this);
//    addPage(w, i18nc("@title:tab", "Style"));








    setMinimumSize(320,380);
    restoreDialogSize(KGlobal::config()->group(objectName()));
}


TrackPropertiesDialogue::~TrackPropertiesDialogue()
{
    KConfigGroup grp = KGlobal::config()->group(objectName());
    saveDialogSize(grp);
}



