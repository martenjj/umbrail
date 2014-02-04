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

#ifndef VARIABLEUNITDISPLAY_H
#define VARIABLEUNITDISPLAY_H



#include <khbox.h>


class QComboBox;
class QLabel;
class QShowEvent;



class VariableUnitDisplay : public KHBox
{
    Q_OBJECT

public:
    enum DisplayType
    {
        Distance,
        Speed
    };

    explicit VariableUnitDisplay(VariableUnitDisplay::DisplayType type, QWidget *pnt = NULL);

    virtual ~VariableUnitDisplay()			{}

    void setValue(double v);

protected:
    void showEvent(QShowEvent *ev);

protected slots:
    void slotUpdateDisplay();

private:
    QLabel *mValueLabel;
    QComboBox *mUnitCombo;
    int mPrecision;
    double mValue;
};

 
#endif							// VARIABLEUNITDISPLAY_H
