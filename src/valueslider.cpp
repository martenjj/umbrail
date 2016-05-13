
#include "valueslider.h"

#include <qlayout.h>
#include <qtoolbutton.h>
#include <qspinbox.h>
#include <qlabel.h>
#include <qslider.h>

#include <klocalizedstring.h>
#include <kdebug.h>


ValueSlider::ValueSlider(QWidget *pnt, int min, int max, bool haveStdButt, int stdValue)
    : QWidget(pnt)
{
    mLayout = new QHBoxLayout(this);
    mLayout->setMargin(0);

    mValue = mStdValue = stdValue;
    mStdButt = NULL;

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

    connect(mSlider, SIGNAL(valueChanged(int)), SLOT(slotSliderSpinboxChange(int)));
    connect(mSpinbox, SIGNAL(valueChanged(int)), SLOT(slotSliderSpinboxChange(int)));
    if (mStdButt!=NULL) connect(mStdButt, SIGNAL(clicked()), SLOT(slotRevertValue()));

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
        mSpinbox->blockSignals(true);
        mSpinbox->setValue(val);			// track in spin box
        mSpinbox->blockSignals(false);
    }

    int slid = mSlider->value();
    if (slid!=val)
    {
        mSlider->blockSignals(true);
        mSlider->setValue(val);				// track in slider
        mSlider->blockSignals(false);
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
