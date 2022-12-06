//////////////////////////////////////////////////////////////////////////
//									//
//  Project:	Umbrail - GPX track viewer and editor			//
//									//
//////////////////////////////////////////////////////////////////////////
//									//
//  Copyright (c) 2014-2022 Jonathan Marten <jjm@keelhaul.me.uk>	//
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

#include "flagseditdialogue.h"

#include <qlabel.h>
#include <qgridlayout.h>
#include <qpushbutton.h>
#include <qbuttongroup.h>
#include <qcheckbox.h>

#include <klocalizedstring.h>
#include <kiconloader.h>


FlagsEditDialogue::FlagsEditDialogue(TrackData::WaypointFlags flags, QWidget *pnt)
    : DialogBase(pnt)
{
    setObjectName("FlagsEditDialogue");
    setModal(true);
    setButtons(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
    setWindowTitle(i18n("Waypoint Flags"));

    QWidget *vb = new QWidget(this);
    QGridLayout *glay = new QGridLayout(vb);
    glay->setColumnMinimumWidth(0, DialogBase::horizontalSpacing());
    glay->setColumnStretch(1, 1);
    setMainWidget(vb);

    mGroup = new QButtonGroup(vb);
    mGroup->setExclusive(false);

    QCheckBox *check = new QCheckBox(i18n("Home point"), vb);
    check->setChecked(flags & TrackData::HomePoint);
    mGroup->addButton(check, TrackData::HomePoint);
    glay->addWidget(check, 1, 1);

    QLabel *pix = new QLabel(this);
    pix->setPixmap(QIcon::fromTheme("go-home").pixmap(KIconLoader::SizeSmall));
    glay->addWidget(pix, 1, 2);

    check = new QCheckBox(i18n("Not exported"), vb);
    check->setChecked(flags & TrackData::NoExport);
    mGroup->addButton(check, TrackData::NoExport);
    glay->addWidget(check, 2, 1);

    pix = new QLabel(this);
    pix->setPixmap(QIcon::fromTheme("process-stop").pixmap(KIconLoader::SizeSmall));
    glay->addWidget(pix, 2, 2);

    check = new QCheckBox(i18n("Newly imported"), vb);
    check->setChecked(flags & TrackData::NewlyImported);
    mGroup->addButton(check, TrackData::NewlyImported);
    glay->addWidget(check, 3, 1);

    pix = new QLabel(this);
    pix->setPixmap(QIcon::fromTheme("document-import").pixmap(KIconLoader::SizeSmall));
    glay->addWidget(pix, 3, 2);

    glay->setRowStretch(4, 1);
}


TrackData::WaypointFlags FlagsEditDialogue::flags() const
{
    TrackData::WaypointFlags f = TrackData::NoFlags;
    const QList<QAbstractButton *> buts = mGroup->buttons();
    for (QAbstractButton *but : buts)
    {
        if (but->isChecked()) f |= static_cast<TrackData::WaypointFlag>(mGroup->id(but));
    }

    return (f);
}
