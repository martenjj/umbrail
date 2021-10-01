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

#include "valueslider.h"

#include <qlayout.h>
#include <qtoolbutton.h>
#include <qspinbox.h>
#include <qlabel.h>
#include <qslider.h>
#include <qdebug.h>

#include <klocalizedstring.h>


ValueSlider::ValueSlider(QWidget *pnt, int min, int max, bool haveStdButt, int stdValue)
    : QWidget(pnt)
{
    mLayout = new QHBoxLayout(this);
    mLayout->setMargin(0);

    mValue = mStdValue = stdValue;
    mStdButt = nullptr;

// TODO: scale steps and slider value so that dragging with mouse moves in SingleStep
// instead of 1

    mSlider = new QSlider(Qt::Horizontal, this);	// slider
    mSlider->setRange(min, max);
    mSlider->setTickPosition(QSlider::TicksBelow);
    mSlider->setTickInterval(qMax(qRound((max-min)/10.0), 1));
    mSlider->setSingleStep(qMax(qRound((max-min)/20.0), 1));
    mSlider->setPageStep(qMax(qRound((max-min)/10.0), 1));
    mSlider->setMinimumWidth(140);
    mSlider->setValue(mValue);				// initial value
    mLayout->addWidget(mSlider, 1);

    mSpinbox = new QSpinBox(this);			// spin box
    mSpinbox->setRange(min, max);
    mSpinbox->setSingleStep(1);
    mSpinbox->setValue(mValue);				// initial value
    mLayout->addWidget(mSpinbox);

    if (haveStdButt)
    {
        mStdButt = new QToolButton(this);		// reset button
        mStdButt->setIcon(QIcon::fromTheme("edit-undo"));
        mStdButt->setToolTip(i18n("Reset this setting to its standard value, %1", stdValue));
        mLayout->addWidget(mStdButt);
    }

    connect(mSlider, &QAbstractSlider::valueChanged, this, &ValueSlider::slotSliderSpinboxChange);
    connect(mSpinbox, QOverload<int>::of(&QSpinBox::valueChanged), this, &ValueSlider::slotSliderSpinboxChange);
    if (mStdButt!=nullptr) connect(mStdButt, &QAbstractButton::clicked, this, &ValueSlider::slotRevertValue);

    setFocusProxy(mSlider);
    setFocusPolicy(Qt::StrongFocus);
}


void ValueSlider::setValue(int val)
{
    if (val==mValue) return;				// avoid recursive signals
    mValue = val;

    int spin = mSpinbox->value();
    if (spin!=val)
    {
        QSignalBlocker block(mSpinbox);
        mSpinbox->setValue(val);			// track in spin box
    }

    int slid = mSlider->value();
    if (slid!=val)
    {
        QSignalBlocker block(mSlider);
        mSlider->setValue(val);				// track in slider
    }
}


int ValueSlider::value() const
{
    return (mValue);
}


void ValueSlider::slotSliderSpinboxChange(int val)
{
    setValue(val);
    emit settingChanged(val);
}


void ValueSlider::slotRevertValue()
{							// only connected if button exists
    slotSliderSpinboxChange(mStdValue);
}
