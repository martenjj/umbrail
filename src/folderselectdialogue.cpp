
#include "folderselectdialogue.h"

#include <QRegExpValidator>

#include <kdebug.h>
#include <klocale.h>
#include <kglobal.h>
#include <kconfiggroup.h>
#include <kinputdialog.h>
#include <kmessagebox.h>
#include <kpushbutton.h>

#include "mainwindow.h"
#include "filesmodel.h"
#include "trackfiltermodel.h"
#include "trackdata.h"
#include "commands.h"


FolderSelectDialogue::FolderSelectDialogue(QWidget *pnt)
    : ItemSelectDialogue(pnt)
{
    setObjectName("FolderSelectDialogue");

    setCaption(i18nc("@title:window", "Select Folder"));
    setButtons(KDialog::User1|KDialog::Ok|KDialog::Cancel);
    setButtonText(KDialog::Ok, i18nc("@action:button", "Select"));
    enableButtonOk(false);
    setButtonText(KDialog::User1, i18nc("@action:button", "New Folder..."));
    setButtonIcon(KDialog::User1, QIcon::fromTheme("folder-new"));

    connect(this, SIGNAL(user1Clicked()), SLOT(slotNewFolder()));
    connect(this, SIGNAL(selectionChanged()), SLOT(slotUpdateButtonStates()));

    trackModel()->setMode(TrackData::Folder);		// can select folders

    slotUpdateButtonStates();
}


void FolderSelectDialogue::slotNewFolder()
{
    QRegExpValidator *val = new QRegExpValidator(QRegExp("^[^/]+$"), this);
    QString name = KInputDialog::getText(i18nc("@title:window", "New Folder"),
                                         i18nc("@label:textbox", "Folder name:"),
                                         QString::null, NULL, this, val);
    if (name.isEmpty()) return;

    TrackDataItem *item = selectedItem();
    TrackDataFolder *foundFolder = item->findChildFolder(name);
    if (foundFolder!=NULL)
    {
        KMessageBox::sorry(this,
                           i18n("A folder named <resource>%1</resource> already exists here.", name),
                           i18n("Folder Exists"));
        return;
    };

    AddContainerCommand *cmd = new AddContainerCommand(filesController());
    cmd->setText(i18n("New Folder"));
    cmd->setData(TrackData::Folder, item);
    cmd->setName(name);
    mainWindow()->executeCommand(cmd);

// select the added folder

    TrackDataFolder *newFolder = item->findChildFolder(name);
    Q_ASSERT(newFolder!=NULL);

    setSelectedItem(newFolder);
    button(KDialog::Ok)->setFocus(Qt::OtherFocusReason);
}


void FolderSelectDialogue::slotUpdateButtonStates()
{
    const TrackDataItem *item = selectedItem();
    const bool isFolder = (dynamic_cast<const TrackDataFolder *>(item)!=NULL);
    const bool isFile = (dynamic_cast<const TrackDataFile *>(item)!=NULL);

    enableButtonOk(isFolder);
    enableButton(KDialog::User1, (isFolder || isFile));
}


void FolderSelectDialogue::setPath(const QString &path)
{
    kDebug() << path;

    TrackDataFolder *selFolder = TrackData::findFolderByPath(path, filesController()->model()->rootFileItem());
    setSelectedItem(selFolder);				// empty path => NULL => clear selection
}
