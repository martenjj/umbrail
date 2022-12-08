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

#include "waypointselectdialogue.h"

#include <qlabel.h>
#include <qgridlayout.h>
#include <qpushbutton.h>
#include <qbuttongroup.h>
#include <qcheckbox.h>

#include <klocalizedstring.h>
#include <kiconloader.h>


WaypointSelectDialogue::WaypointSelectDialogue(QWidget *pnt)
    : DialogBase(pnt)
{
    setObjectName("WaypointSelectDialogue");

    setModal(true);
    setButtons(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
    setWindowTitle(i18n("Plot Points"));

    QWidget *vb = new QWidget(this);
    QGridLayout *glay = new QGridLayout(vb);
    glay->setColumnMinimumWidth(0, DialogBase::horizontalSpacing());
    glay->setColumnStretch(1, 1);
    setMainWidget(vb);

    QLabel *l = new QLabel(i18n("Show points on plot:"), vb);
    glay->addWidget(l, 0, 0, 1, -1);

    mGroup = new QButtonGroup(vb);
    mGroup->setExclusive(false);

    QCheckBox *check = new QCheckBox(i18n("Waypoints"), vb);
    mGroup->addButton(check, SelectWaypoints);
    glay->addWidget(check, 1, 1);

    QLabel *pix = new QLabel(this);
    pix->setPixmap(QIcon::fromTheme("favorites").pixmap(KIconLoader::SizeSmall));
    glay->addWidget(pix, 1, 2);

    check = new QCheckBox(i18n("Route points"), vb);
    mGroup->addButton(check, SelectRoutepoints);
    glay->addWidget(check, 2, 1);

    pix = new QLabel(this);
    pix->setPixmap(QIcon::fromTheme("flag").pixmap(KIconLoader::SizeSmall));
    glay->addWidget(pix, 2, 2);

    check = new QCheckBox(i18n("Stops"), vb);
    mGroup->addButton(check, SelectStops);
    glay->addWidget(check, 3, 1);

    pix = new QLabel(this);
    pix->setPixmap(QIcon::fromTheme("media-playback-stop").pixmap(KIconLoader::SizeSmall));
    glay->addWidget(pix, 3, 2);

    check = new QCheckBox(i18n("Audio Notes"), vb);
    mGroup->addButton(check, SelectAudioNotes);
    glay->addWidget(check, 4, 1);

    pix = new QLabel(this);
    pix->setPixmap(QIcon::fromTheme("speaker").pixmap(KIconLoader::SizeSmall));
    glay->addWidget(pix, 4, 2);

    check = new QCheckBox(i18n("Video Notes"), vb);
    mGroup->addButton(check, SelectVideoNotes);
    glay->addWidget(check, 5, 1);

    pix = new QLabel(this);
    pix->setPixmap(QIcon::fromTheme("mixer-video").pixmap(KIconLoader::SizeSmall));
    glay->addWidget(pix, 5, 2);

    check = new QCheckBox(i18n("Photos"), vb);
    mGroup->addButton(check, SelectPhotos);
    glay->addWidget(check, 6, 1);

    pix = new QLabel(this);
    pix->setPixmap(QIcon::fromTheme("image-x-generic").pixmap(KIconLoader::SizeSmall));
    glay->addWidget(pix, 6, 2);

    setMinimumSize(QSize(260, 250));
}


void WaypointSelectDialogue::setSelection(WaypointSelectDialogue::SelectionSet sel)
{
    const QList<QAbstractButton *> buts = mGroup->buttons();
    for (QAbstractButton *but : buts)
    {
        WaypointSelectDialogue::Selection buttonSel = static_cast<WaypointSelectDialogue::Selection>(mGroup->id(but));
        but->setChecked(sel & buttonSel);
    }
}


WaypointSelectDialogue::SelectionSet WaypointSelectDialogue::selection() const
{
    WaypointSelectDialogue::SelectionSet sel = WaypointSelectDialogue::SelectionSet();
    const QList<QAbstractButton *> buts = mGroup->buttons();
    for (QAbstractButton *but : buts)
    {
        if (but->isChecked()) sel |= static_cast<WaypointSelectDialogue::Selection>(mGroup->id(but));
    }

    return (sel);
}
