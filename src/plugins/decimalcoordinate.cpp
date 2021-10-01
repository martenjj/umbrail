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

#include "decimalcoordinate.h"

#include <qlineedit.h>
#include <qformlayout.h>
#include <qdebug.h>

#include <QDoubleValidator>

#include <klocalizedstring.h>

#include "trackdata.h"

#define PRECISION		6			// how many decimal places


DecimalCoordinateHandler::DecimalCoordinateHandler(QObject *pnt)
    : AbstractCoordinateHandler(pnt)
{
    qDebug();
    setObjectName("DecimalCoordinateHandler");
}


QWidget *DecimalCoordinateHandler::createWidget(QWidget *pnt)
{
    QWidget *w = new QWidget(pnt);
    QFormLayout *fl = new QFormLayout(w);

    mLatitudeEdit = new QLineEdit(w);
    connect(mLatitudeEdit, &QLineEdit::textEdited, this, &DecimalCoordinateHandler::slotTextChanged);
    QDoubleValidator *dv = new QDoubleValidator(mLatitudeEdit);
    dv->setRange(-90, 90, PRECISION);
    mLatitudeEdit->setValidator(dv);
    fl->addRow(i18n("Latitude:"), mLatitudeEdit);

    mLongitudeEdit = new QLineEdit(w);
    connect(mLongitudeEdit, &QLineEdit::textEdited, this, &DecimalCoordinateHandler::slotTextChanged);
    dv = new QDoubleValidator(mLongitudeEdit);
    dv->setRange(-180, 180, PRECISION);
    mLongitudeEdit->setValidator(dv);
    fl->addRow(i18n("Longitude:"), mLongitudeEdit);

    return (w);
}


void DecimalCoordinateHandler::updateGUI(double lat, double lon)
{
    qDebug() << lat << lon;

    if (!ISNAN(lat)) mLatitudeEdit->setText(QString::number(lat, 'f', PRECISION));
    else mLatitudeEdit->clear();

    if (!ISNAN(lon)) mLongitudeEdit->setText(QString::number(lon, 'f', PRECISION));
    else mLongitudeEdit->clear();
}


bool DecimalCoordinateHandler::hasAcceptableInput() const
{
    return (mLatitudeEdit->hasAcceptableInput() && mLongitudeEdit->hasAcceptableInput());
}


void DecimalCoordinateHandler::slotTextChanged(const QString &text)
{
    qDebug();
    updateValues(mLatitudeEdit->text().toDouble(),
                 mLongitudeEdit->text().toDouble());
}


QString DecimalCoordinateHandler::tabName() const
{
    return (i18nc("@title:tab", "Decimal"));
}
