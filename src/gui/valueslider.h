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
