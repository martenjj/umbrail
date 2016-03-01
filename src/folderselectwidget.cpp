
#include "folderselectwidget.h"

#include <qboxlayout.h>
#include <qpushbutton.h>
#include <qlineedit.h>

#include <kdebug.h>
#include <klocale.h>
#include <kicon.h>

#include "mainwindow.h"
#include "folderselectdialogue.h"


FolderSelectWidget::FolderSelectWidget(MainWindow *mw, QWidget *pnt)
    : QFrame(pnt)
{
    setObjectName("FolderSelectWidget");

    mMainWindow = mw;

    QHBoxLayout *hb = new QHBoxLayout(this);
    hb->setMargin(0);

    mDestFolder = new QLineEdit(this);
    mDestFolder->setReadOnly(true);
    mDestFolder->setPlaceholderText(i18n("Select..."));
    hb->addWidget(mDestFolder);

    QPushButton *reqButton = new QPushButton(KIcon("folder"), QString::null, this);
    connect(reqButton, SIGNAL(clicked(bool)), SLOT(slotSelectFolder()));
    hb->addWidget(reqButton);

    hb->setStretch(0, 1);
}


void FolderSelectWidget::slotSelectFolder()
{
    FolderSelectDialogue *d = new FolderSelectDialogue(mMainWindow, this);
    d->setDestinationPath(mDestFolder->text());
    if (!d->exec()) return;

    const TrackDataFolder *selectedFolder = dynamic_cast<const TrackDataFolder *>(d->selectedDestination());
    if (selectedFolder==NULL) mDestFolder->clear();
    else mDestFolder->setText(selectedFolder->path());

    emit folderChanged(folderPath());
}


QString FolderSelectWidget::folderPath() const
{
    return (mDestFolder->text());
}
