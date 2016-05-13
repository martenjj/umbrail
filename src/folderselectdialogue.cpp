
#include "folderselectdialogue.h"

#include <qdebug.h>
#include <QRegExpValidator>

#include <klocalizedstring.h>
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

    setWindowTitle(i18nc("@title:window", "Select Folder"));
    setButtons(QDialogButtonBox::Yes|QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
    setButtonText(QDialogButtonBox::Ok, i18nc("@action:button", "Select"));
    setButtonEnabled(QDialogButtonBox::Ok, false);
    setButtonText(QDialogButtonBox::Yes, i18nc("@action:button", "New Folder..."));
    setButtonIcon(QDialogButtonBox::Yes, QIcon::fromTheme("folder-new"));

    connect(buttonBox()->button(QDialogButtonBox::Yes), SIGNAL(clicked()), SLOT(slotNewFolder()));
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
    buttonBox()->button(QDialogButtonBox::Ok)->setFocus(Qt::OtherFocusReason);
}


void FolderSelectDialogue::slotUpdateButtonStates()
{
    const TrackDataItem *item = selectedItem();
    const bool isFolder = (dynamic_cast<const TrackDataFolder *>(item)!=NULL);
    const bool isFile = (dynamic_cast<const TrackDataFile *>(item)!=NULL);

    setButtonEnabled(QDialogButtonBox::Ok, isFolder);
    setButtonEnabled(QDialogButtonBox::Yes, (isFolder || isFile));
}


void FolderSelectDialogue::setPath(const QString &path)
{
    qDebug() << path;

    TrackDataFolder *selFolder = TrackData::findFolderByPath(path, filesController()->model()->rootFileItem());
    setSelectedItem(selFolder);				// empty path => NULL => clear selection
}
