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

#include "folderselectwidget.h"

#include <qboxlayout.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qdebug.h>

#include <klocalizedstring.h>

#include "folderselectdialogue.h"


FolderSelectWidget::FolderSelectWidget(QWidget *pnt)
    : QFrame(pnt),
      ApplicationDataInterface(pnt)
{
    setObjectName("FolderSelectWidget");

    QHBoxLayout *hb = new QHBoxLayout(this);
    hb->setMargin(0);

    mDestFolder = new QLineEdit(this);
    mDestFolder->setReadOnly(true);
    mDestFolder->setPlaceholderText(i18n("Select..."));
    hb->addWidget(mDestFolder);

    QPushButton *reqButton = new QPushButton(QIcon::fromTheme("folder"), "", this);
    reqButton->setToolTip(i18n("Select or create a folder"));
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
    const QString entry = mDestFolder->text();
    return (!entry.isEmpty() ? entry : mDefaultFolder);
}


void FolderSelectWidget::setFolderPath(const QString &path, bool asDefault)
{
    if (asDefault)
    {
        mDefaultFolder = path;				// remember if no entry made
        mDestFolder->clear();
        mDestFolder->setPlaceholderText(path);
    }
    else
    {
        mDestFolder->setText(path);
    }
}
