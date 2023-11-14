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
#include <qfontdatabase.h>

#include <klocalizedstring.h>
#include <kiconloader.h>


static QLabel *createHintLabel(const QString &text, QWidget *parent)
{
    QLabel *hintLabel = new QLabel(text+"", parent);
    hintLabel->setFont(QFontDatabase::systemFont(QFontDatabase::SmallestReadableFont));
    hintLabel->setWordWrap(true);
    hintLabel->setAlignment(Qt::AlignLeft|Qt::AlignTop);

    // In this application there seems to be no need to force the label's
    // minimum width, as was needed in the Klipper configuration dialogue.

    return (hintLabel);
}


static QPixmap createPixmap(const QString &name, const QString &overlay1, const QString &overlay2)
{
    // Our overlays at the top left and top right corners.
    return (KIconLoader::global()->loadIcon(name, KIconLoader::NoGroup,
                                            KIconLoader::SizeMedium, KIconLoader::DefaultState,
                                            QStringList() << "" << "" << overlay1 << overlay2));
}


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

    // Home point
    QCheckBox *check = new QCheckBox(i18n("Home point"), vb);
    check->setChecked(flags & TrackData::HomePoint);
    mGroup->addButton(check, TrackData::HomePoint);
    glay->addWidget(check, 1, 1, Qt::AlignTop);

    QLabel *pix = new QLabel(this);
    pix->setPixmap(createPixmap("go-home", "", ""));
    glay->addWidget(pix, 1, 2, 2, 1, Qt::AlignTop|Qt::AlignRight);

    QLabel *hint = createHintLabel(i18n("The waypoint can be selected as \"Home\" or \"Work\" when exporting."), this);
    glay->addWidget(hint, 2, 1);

    glay->setRowMinimumHeight(3, DialogBase::verticalSpacing());

    // Not exported
    check = new QCheckBox(i18n("Not exported"), vb);
    check->setChecked(flags & TrackData::NoExport);
    mGroup->addButton(check, TrackData::NoExport);
    glay->addWidget(check, 4, 1, Qt::AlignTop);

    pix = new QLabel(this);
    pix->setPixmap(createPixmap("document-export", "process-stop", ""));
    glay->addWidget(pix, 4, 2, 2, 1, Qt::AlignTop|Qt::AlignRight);

    hint = createHintLabel(i18n("The waypoint will not be exported."), this);
    glay->addWidget(hint, 5, 1);

    glay->setRowMinimumHeight(6, DialogBase::verticalSpacing());

    // Newly imported
    check = new QCheckBox(i18n("Newly imported"), vb);
    check->setChecked(flags & TrackData::NewlyImported);
    mGroup->addButton(check, TrackData::NewlyImported);
    glay->addWidget(check, 7, 1, Qt::AlignTop);

    pix = new QLabel(this);
    pix->setPixmap(createPixmap("document-import", "", "emblem-new"));
    glay->addWidget(pix, 7, 2, 2, 1, Qt::AlignTop|Qt::AlignRight);

    hint = createHintLabel(i18n("The waypoint has been newly imported and may need to be verified or merged with an existing one."), this);
    glay->addWidget(hint, 8, 1);

    glay->setRowMinimumHeight(9, DialogBase::verticalSpacing());
    glay->setRowStretch(9, 1);
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
