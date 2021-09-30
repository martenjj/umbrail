
#include "folderselectdialogue.h"

#include <qdebug.h>
#include <qinputdialog.h>
#include <qpushbutton.h>

#include <klocalizedstring.h>
#include <kconfiggroup.h>
#include <kmessagebox.h>

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

    connect(buttonBox()->button(QDialogButtonBox::Yes), &QAbstractButton::clicked, this, &FolderSelectDialogue::slotNewFolder);
    connect(this, &ItemSelectDialogue::selectionChanged, this, &FolderSelectDialogue::slotUpdateButtonStates);

    trackModel()->setMode(TrackData::Folder);		// can select folders

    slotUpdateButtonStates();
}


void FolderSelectDialogue::slotNewFolder()
{
    // Qt5 regression: There is no way to set a validator on a QInputDialog
    // in text mode, and no way to create a subclass to do so.  This is because
    // there is no access to the text edit or button box buttons.
    // TODO: maybe can do so by querying the object tree
    //QRegExpValidator *val = new QRegExpValidator(QRegExp("^[^/]+$"), this);
    QString name = QInputDialog::getText(this,						// parent
                                         i18nc("@title:window", "New Folder"),		// title
                                         i18nc("@label:textbox", "Folder name:"));	// label
    if (name.isEmpty()) return;
    if (name.contains('/')) return;

    TrackDataItem *item = selectedItem();
    TrackDataFolder *foundFolder = TrackData::findFolderByPath(name, item);
    if (foundFolder!=nullptr)
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
    executeCommand(cmd);

    // select the added folder
    TrackDataFolder *newFolder = TrackData::findFolderByPath(name, item);
    Q_ASSERT(newFolder!=nullptr);
    setSelectedItem(newFolder);

    buttonBox()->button(QDialogButtonBox::Ok)->setFocus(Qt::OtherFocusReason);
}


void FolderSelectDialogue::slotUpdateButtonStates()
{
    const TrackDataItem *item = selectedItem();
    const bool isFolder = (dynamic_cast<const TrackDataFolder *>(item)!=nullptr);
    const bool isFile = (dynamic_cast<const TrackDataFile *>(item)!=nullptr);

    setButtonEnabled(QDialogButtonBox::Ok, isFolder);
    setButtonEnabled(QDialogButtonBox::Yes, (isFolder || isFile) && !isReadOnly());
}


void FolderSelectDialogue::setPath(const QString &path)
{
    qDebug() << path;

    TrackDataFolder *selFolder = TrackData::findFolderByPath(path, filesController()->model()->rootFileItem());
    setSelectedItem(selFolder);				// empty path => NULL => clear selection
}
