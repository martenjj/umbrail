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

#ifndef DMSCOORDINATE_H
#define DMSCOORDINATE_H

#include "abstractcoordinatehandler.h"


class QLineEdit;
class QComboBox;


class DMSCoordinateHandler : public AbstractCoordinateHandler
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID ACH_PLUGIN_IID)
    Q_INTERFACES(AbstractCoordinateHandler)

public:
    PLUGIN_EXPORT DMSCoordinateHandler(QObject *pnt = nullptr);
    virtual PLUGIN_EXPORT ~DMSCoordinateHandler() = default;

    PLUGIN_EXPORT QWidget *createWidget(QWidget *pnt = nullptr) override;
    PLUGIN_EXPORT bool hasAcceptableInput() const override;
    PLUGIN_EXPORT QString tabName() const override;

protected:
    void updateGUI(double lat, double lon) override;

private slots:
    void slotTextChanged();

private:
    void setDMS(double d, QLineEdit *deg, QLineEdit *min, QLineEdit *sec, QComboBox *sign);
    double getDMS(QLineEdit *deg, QLineEdit *min, QLineEdit *sec, QComboBox *sign) const;

private:
    QLineEdit *mLatitudeDeg;
    QLineEdit *mLatitudeMin;
    QLineEdit *mLatitudeSec;
    QComboBox *mLatitudeCombo;

    QLineEdit *mLongitudeDeg;
    QLineEdit *mLongitudeMin;
    QLineEdit *mLongitudeSec;
    QComboBox *mLongitudeCombo;
};

#endif							// DMSCOORDINATE_H
