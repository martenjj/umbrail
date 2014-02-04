//////////////////////////////////////////////////////////////////////////
//									//
//  Project:	NavTracks						//
//  Edit:	02-Feb-14						//
//									//
//////////////////////////////////////////////////////////////////////////
//									//
//  Copyright (c) 2012-2014 Jonathan Marten <jjm@keelhaul.me.uk>	//
//  Home and download page:  http://www.keelhaul.me.uk/TBD/		//
//									//
//  This program is free software; you can redistribute it and/or	//
//  modify it under the terms of the GNU General Public License as	//
//  published by the Free Software Foundation; either version 2 of	//
//  the License, or (at your option) any later version.			//
//									//
//  It is distributed in the hope that it will be useful, but		//
//  WITHOUT ANY WARRANTY; without even the implied warranty of		//
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the	//
//  GNU General Public License for more details.			//
//									//
//  You should have received a copy of the GNU General Public		//
//  License along with this program; see the file COPYING for further	//
//  details.  If not, write to the Free Software Foundation, Inc.,	//
//  59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.		//
//									//
//////////////////////////////////////////////////////////////////////////

#include "variableunitdisplay.h"

#include <qcombobox.h>
#include <qlabel.h>

#include <kdebug.h>
#include <klocale.h>



static const double EARTH_RADIUS_KM = 6371;		// kilometres
static const double EARTH_RADIUS_MI = 3959;		// statute miles
static const double EARTH_RADIUS_NM = 3440;		// nautical miles



VariableUnitDisplay::VariableUnitDisplay(VariableUnitDisplay::DisplayType type, QWidget *pnt)
    : KHBox(pnt)
{
    kDebug() << "type" << type;

    setObjectName("VariableUnitDisplay");
    setSpacing(-1);					// default layout spacing

    mValue = 0.0;
    mPrecision = 1;

    mValueLabel = new QLabel("---", this);
    mValueLabel->setTextInteractionFlags(Qt::TextSelectableByMouse|Qt::TextSelectableByKeyboard);

    mUnitCombo = new QComboBox(this);
    // TODO: global preference
    // TODO: remember setting
    mUnitCombo->setCurrentIndex(0);

    switch (type)
    {
case VariableUnitDisplay::Distance:
        mUnitCombo->addItem(i18n("kilometres"), EARTH_RADIUS_KM);
        mUnitCombo->addItem(i18n("miles"), EARTH_RADIUS_MI);
        mUnitCombo->addItem(i18n("nautical miles"), EARTH_RADIUS_NM);
        mPrecision = 2;
        break;

case VariableUnitDisplay::Speed:
        mUnitCombo->addItem(i18n("km/h"), EARTH_RADIUS_KM);
        mUnitCombo->addItem(i18n("mph"), EARTH_RADIUS_MI);
        mUnitCombo->addItem(i18n("knots"), EARTH_RADIUS_NM);
        break;

default:
        break;
    }
    connect(mUnitCombo, SIGNAL(currentIndexChanged(int)), SLOT(slotUpdateDisplay()));

    setSizePolicy(QSizePolicy::Expanding, sizePolicy().verticalPolicy());
    setFocusProxy(mUnitCombo);
    setFocusPolicy(Qt::StrongFocus);
}



void VariableUnitDisplay::setValue(double v)
{
    mValue = v;
    slotUpdateDisplay();
}


void VariableUnitDisplay::slotUpdateDisplay()
{
    double v = mValue*mUnitCombo->itemData(mUnitCombo->currentIndex()).toDouble();
    mValueLabel->setText(QString::number(v, 'f', mPrecision));
}


void VariableUnitDisplay::showEvent(QShowEvent *ev)
{
    KHBox::showEvent(ev);

    QObject *pnt = parent();				// no GUI parent?
    if (pnt==NULL) return;				// should never happen

    QList<VariableUnitDisplay *> siblings = pnt->findChildren<VariableUnitDisplay *>();
    if (siblings.isEmpty()) return;			// should never happen
    if (siblings.first()!=this) return;			// only do this once per chain

    // Run through the sibling chain, and find the longest display
    // width of all their combo boxes.
    int maxSize = 0;
    for (QList<VariableUnitDisplay *>::iterator it = siblings.begin();
         it!=siblings.end(); ++it)
    {
        VariableUnitDisplay *vud = (*it);
        maxSize = qMax(maxSize, vud->mUnitCombo->width());
    }

    // Set all of the combo boxes to that as their minimum width.
    for (QList<VariableUnitDisplay *>::iterator it = siblings.begin();
         it!=siblings.end(); ++it)
    {
        VariableUnitDisplay *vud = (*it);
        vud->mUnitCombo->setMinimumWidth(maxSize);
    }
}
