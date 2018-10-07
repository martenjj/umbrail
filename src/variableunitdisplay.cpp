//////////////////////////////////////////////////////////////////////////
//									//
//  Project:	NavTracks						//
//  Edit:	07-Oct-18						//
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

#include <math.h>

#include <qlabel.h>
#include <qdebug.h>
#include <qboxlayout.h>

#include <klocalizedstring.h>
#include <ksharedconfig.h>
#include <kconfiggroup.h>

#include "units.h"


VariableUnitDisplay::VariableUnitDisplay(VariableUnitCombo::DisplayType type, QWidget *pnt)
    : QFrame(pnt)
{
    qDebug() << "type" << type;

    setObjectName("VariableUnitDisplay");

    QHBoxLayout *hb = new QHBoxLayout(this);
    hb->setMargin(0);

    mValue = 0.0;

    mValueLabel = new QLabel("---", this);
    mValueLabel->setTextInteractionFlags(Qt::TextSelectableByMouse|Qt::TextSelectableByKeyboard);
    hb->addWidget(mValueLabel);

    mUnitCombo = new VariableUnitCombo(type, this);
    hb->addWidget(mUnitCombo);
    connect(mUnitCombo, SIGNAL(currentIndexChanged(int)), SLOT(slotUpdateDisplay()));
    mComboIndex = -1;

    setSizePolicy(QSizePolicy::Expanding, sizePolicy().verticalPolicy());
    setFocusProxy(mUnitCombo);
    setFocusPolicy(Qt::StrongFocus);
}


VariableUnitDisplay::~VariableUnitDisplay()
{
    if (!mSaveId.isEmpty())
    {
        KConfigGroup grp = KSharedConfig::openConfig()->group(objectName());
        if (mComboIndex!=-1) grp.writeEntry(mSaveId, mComboIndex);
    }
}


void VariableUnitDisplay::setValue(double v)
{
    mValue = v;
    slotUpdateDisplay();
}


void VariableUnitDisplay::slotUpdateDisplay()
{
    // Save this for use in destructor, in case combo box
    // has been destroyed by then.
    mComboIndex = mUnitCombo->currentIndex();

    double v = mValue;
    if (isnan(v))					// blank value
    {
        mValueLabel->setText(QString());
        return;
    }

    const VariableUnitCombo::DisplayType type = mUnitCombo->type();
    const Units::Unit unit = mUnitCombo->unit();

    switch (type)
    {
case VariableUnitCombo::Bearing:
        {
            QString sign;
            QString degs(QChar(0xB0));
            switch (unit)
            {
case Units::BearingAbsolute:
                if (v<0) v += 360;
                break;

case Units::BearingRelative:
                sign = (v<0 ? "-" : "+");
                if (v<0) v = -v;
                break;

case Units::BearingNautical:
                sign = (v<0 ? "R " : "G ");
                if (v<0) v = -v;
                degs = "";
                break;

default:	qWarning() << "called for bearing with Units::Unit" << unit;
                Q_ASSERT(false);
            }

            mValueLabel->setText(sign+QString::number(v, 'f', mUnitCombo->precision())+degs);
        }
        break;

case VariableUnitCombo::Distance:
        v = Units::internalToLength(v, unit);
        mValueLabel->setText(QString::number(v, 'f', mUnitCombo->precision()));
        break;

case VariableUnitCombo::Elevation:
        v = Units::internalToElevation(v, unit);
        mValueLabel->setText(QString::number(v, 'f', mUnitCombo->precision()));
        break;

case VariableUnitCombo::Speed:
        v = Units::internalToSpeed(v, unit);
        mValueLabel->setText(QString::number(v, 'f', mUnitCombo->precision()));
        break;

default:
        qWarning() << "called for VariableUnitCombo::DisplayType" << type;
        Q_ASSERT(false);
    }
}



void VariableUnitDisplay::showEvent(QShowEvent *ev)
{
    QFrame::showEvent(ev);

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


void VariableUnitDisplay::setSaveId(const QString &id)
{
    mSaveId = id;
    if (!mSaveId.isEmpty())
    {
        const KConfigGroup grp = KSharedConfig::openConfig()->group(objectName());
        int idx = grp.readEntry(mSaveId, -1);
        if (idx!=-1) mUnitCombo->setCurrentIndex(idx);
    }
}
