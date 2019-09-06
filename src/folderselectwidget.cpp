
#include "folderselectwidget.h"

#include <qboxlayout.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qdebug.h>

#include <klocalizedstring.h>

#include "mainwindow.h"
#include "folderselectdialogue.h"


FolderSelectWidget::FolderSelectWidget(QWidget *pnt)
    : QFrame(pnt),
      MainWindowInterface(pnt)
{
    setObjectName("FolderSelectWidget");

    QHBoxLayout *hb = new QHBoxLayout(this);
    hb->setMargin(0);

    mDestFolder = new QLineEdit(this);
    mDestFolder->setReadOnly(true);
    mDestFolder->setPlaceholderText(i18n("Select..."));
    hb->addWidget(mDestFolder);

    QPushButton *reqButton = new QPushButton(QIcon::fromTheme("folder"), "", this);
    connect(reqButton, SIGNAL(clicked(bool)), SLOT(slotSelectFolder()));
    hb->addWidget(reqButton);

    hb->setStretch(0, 1);
}


void FolderSelectWidget::slotSelectFolder()
{
    FolderSelectDialogue d(this);
    d.setPath(mDestFolder->text());
    if (!d.exec()) return;

    const TrackDataFolder *selectedFolder = dynamic_cast<const TrackDataFolder *>(d.selectedItem());
    if (selectedFolder==nullptr) mDestFolder->clear();
    else mDestFolder->setText(selectedFolder->path());

    emit folderChanged(folderPath());
}


QString FolderSelectWidget::folderPath() const
{
    return (mDestFolder->text());
}
