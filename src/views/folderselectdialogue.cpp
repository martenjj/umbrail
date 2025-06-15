//////////////////////////////////////////////////////////////////////////
//									//
//  Project:	Umbrail - GPX track viewer and editor			//
//									//
//////////////////////////////////////////////////////////////////////////
//									//
//  Copyright (c) 2014-2021 Jonathan Marten <jjm@keelhaul.me.uk>	//
//  Home and download page: <http://github.com/martenjj/umbrail>	//
//									//
//  This program is free software; you can redistribute it and/or	//
//  modify it under the terms of the GNU General Public License as	//
//  published by the Free Software Foundation, either version 3 of	//
//  the License or (at your option) any later version.			//
//									//
//  It is distributed in the hope that it will be useful, but		//
//  WITHOUT ANY WARRANTY;  without even the implied warranty of		//
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the	//
//  GNU General Public License for more details.			//
//									//
//  You should have received a copy of the GNU General Public License	//
//  along with this program;  see the file COPYING for further		//
//  details.  If not, see <http://gnu.org/licenses/gpl>.      		//
//									//
//////////////////////////////////////////////////////////////////////////

#include "folderselectdialogue.h"

#include <qdebug.h>
#include <qinputdialog.h>
#include <qpushbutton.h>

#include <klocalizedstring.h>

#include "destinationfiltermodel.h"


FolderSelectDialogue::FolderSelectDialogue(QWidget *pnt)
    : ItemSelectDialogue(pnt)
{
    setObjectName("FolderSelectDialogue");

    setWindowTitle(i18nc("@title:window", "Select Folder"));
    setButtons(QDialogButtonBox::Reset|QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
    setButtonText(QDialogButtonBox::Ok, i18nc("@action:button", "Select"));
    setButtonEnabled(QDialogButtonBox::Ok, false);
    setButtonText(QDialogButtonBox::Reset, i18nc("@action:button", "New Folder..."));
    setButtonIcon(QDialogButtonBox::Reset, QIcon::fromTheme("folder-new"));

    connect(buttonBox()->button(QDialogButtonBox::Reset), &QAbstractButton::clicked, this, &FolderSelectDialogue::slotNewFolder);
    connect(this, &ItemSelectDialogue::selectionChanged, this, &FolderSelectDialogue::slotUpdateButtonStates);

    destinationModel()->setMode(TrackData::Folder);	// can select folders

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

    // Create the new named folder under the selected item as parent.
    TrackDataContainer *parentItem = ASV(TrackDataContainer, selectedItem());
    Q_ASSERT(parentItem!=nullptr);
    emit newFolder(name, parentItem);

    // Then find and select the new added folder.
    TrackDataFolder *newFolder = TrackData::findFolderByPath(name, parentItem);
    Q_ASSERT(newFolder!=nullptr);
    setSelectedItem(newFolder);

    buttonBox()->button(QDialogButtonBox::Ok)->setFocus(Qt::OtherFocusReason);
}


void FolderSelectDialogue::slotUpdateButtonStates()
{
    const TrackDataItem *item = selectedItem();
    const bool isFolder = IS(TrackDataFolder, item);
    const bool isFile = IS(TrackDataFile, item);

    setButtonEnabled(QDialogButtonBox::Ok, isFolder);
    setButtonEnabled(QDialogButtonBox::Reset, (isFolder || isFile) && !isReadOnly());
}


void FolderSelectDialogue::setPath(const QString &path)
{
    qDebug() << path;

    TrackDataFolder *selFolder = TrackData::findFolderByPath(path, ItemIndexInterface::of(destinationModel())->rootItem());
    setSelectedItem(selFolder);				// empty path => NULL => clear selection
}
