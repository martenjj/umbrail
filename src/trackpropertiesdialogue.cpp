
#include "trackpropertiesdialogue.h"

#include <qlayout.h>
#include <qlabel.h>

#include <kdebug.h>
#include <klocale.h>
#include <kpagedialog.h>

#include "trackdata.h"
#include "trackproperties.h"


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

    QWidget *w = item->createPropertiesGeneralPage(items, this);
    mGeneralPage = qobject_cast<TrackItemGeneralPage *>(w);
    Q_ASSERT(mGeneralPage!=NULL);
    connect(mGeneralPage, SIGNAL(enableButtonOk(bool)), SLOT(enableButtonOk(bool)));
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



QString TrackPropertiesDialogue::newItemName() const
{
    return (mGeneralPage->newItemName());
}


KUrl TrackPropertiesDialogue::newFileUrl() const
{
    TrackFileGeneralPage *filePage = qobject_cast<TrackFileGeneralPage *>(mGeneralPage);
    if (filePage==NULL) return (KUrl());
    return (filePage->newFileUrl());
}
