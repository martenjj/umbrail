
#ifndef VALUESLIDER_H
#define VALUESLIDER_H

#include <qwidget.h>


class QHBoxLayout;
class QToolButton;
class QSpinBox;
class QSlider;


/**
 * A slider combined with a spin box, providing the possibility of either
 * selecting a value with the slider or entering a precise value in the
 * spin box.  There can also optionally be a 'reset' button which returns
 * the setting to a default value.
 *
 * Based on KScanSlider from kooka/libkscan/kscancontrols.cpp
 */

class ValueSlider : public QWidget
{
    Q_OBJECT

public:
    ValueSlider(QWidget *pnt, int min, int max, bool haveStdButt = false, int stdValue = 0);
    virtual ~ValueSlider()					{}

    void setValue(int val);
    int value() const;

    QSpinBox *spinBox() const { return (mSpinbox); }

protected:
    QHBoxLayout *mLayout;

protected slots:
    void slotSliderSpinboxChange(int val);
    void slotRevertValue();

signals:
    void settingChanged(int val);
    void returnPressed();

private:
    QSlider *mSlider;
    QSpinBox *mSpinbox;
    QToolButton *mStdButt;

    int mValue;
    int mStdValue;
};

#endif							// VALUESLIDER_H
