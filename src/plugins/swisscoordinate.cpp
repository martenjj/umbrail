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

#include "swisscoordinate.h"

#include <qlineedit.h>
#include <qlabel.h>
#include <qformlayout.h>
#include <qdebug.h>

#include <QIntValidator>

#include <klocalizedstring.h>

#include "trackdata.h"


//  The reference document for the Swiss coordinate system is "Formulas
//  and constants for the calculation of the Swiss conformal cylindrical
//  projection and for the transformation between coordinate systems"
//
//  https://www.swisstopo.admin.ch/content/swisstopo-internet/en/online/
//    calculation-services/_jcr_content/contentPar/tabs/items/
//    documents_publicatio/tabPar/downloadlist/downloadItems/
//    20_1467104436749.download/refsyse.pdf
//
//  Section references to "SwissTopo" are to this document.
//
//  Currently the implementation here is for coordinate system CH1903/LV03.
//  SwissTopo section 1.1 says that the new CH1903+/LV95 system is mandated
//  for official use from 2016 onwards, but if using the approximate formulas
//  (SwissTopo sections 4.1 and 4.2) the two systems are identical apart from
//  the fixed E/N offsets.
//
//  For background information and an online converter see also
//
//  http://blog.encomiabile.it/2014/10/13/convert-swiss-national-grid-coordinates-ch1903-to-wgs1984/
//  http://www.giangrandi.ch/soft/swissgrid/swissgrid.shtml

// from SwissTopo section 5
#define MIN_SWISS_EASTING	400000
#define MAX_SWISS_EASTING	900000
#define MIN_SWISS_NORTHING	0
#define MAX_SWISS_NORTHING	300000

#undef DEBUG_SWISS


void SwissCoordinateHandler::setSwiss(double lat, double lon,
                                      QLineEdit *east, QLineEdit *north)
{
    // SwissTopo section 4.1
    double phi = lat*3600;			// degrees -> arcseconds
    double lambda = lon*3600;
#ifdef DEBUG_SWISS
    qDebug() << "Swiss: input phi" << phi << "lambda" << lambda;
#endif
    double phip = (phi-169028.66)/10000;
    double lambdap = (lambda-26782.5)/10000;
#ifdef DEBUG_SWISS
    qDebug() << "  phip" << phip << "lambdap" << lambdap;
#endif
    int e = qRound(2600072.37
                   + 211455.93*lambdap
                   - 10938.51*lambdap*phip
                   - 0.36*lambdap*pow(phip, 2)
                   - 44.54*pow(lambdap, 3));
    int n = qRound(1200147.07
                   + 308807.95*phip
                   + 3745.25*pow(lambdap, 2)
                   + 76.63*pow(phip, 2)
                   - 194.56*pow(lambdap, 2)*phip
                   + 119.79*pow(phip, 3));
#ifdef DEBUG_SWISS
    qDebug() << "  e" << e << "n" << n;
#endif
    int yy = e-2000000;
    int xx = n-1000000;
#ifdef DEBUG_SWISS
    qDebug() << "Swiss: result yy" << yy << "xx" << xx;
#endif
    east->setText(QString::number(yy));
    north->setText(QString::number(xx));
}


void SwissCoordinateHandler::getSwiss(QLineEdit *east, QLineEdit *north,
                                      double *latp, double *lonp) const
{
    int yy = east->text().toInt();
    int xx = north->text().toInt();
#ifdef DEBUG_SWISS
    qDebug() << "Swiss: input yy" << yy << "xx" << xx;
#endif
    // SwissTopo section 4.2
    double yp = (yy-600000)/1000000.0;
    double xp = (xx-200000)/1000000.0;
#ifdef DEBUG_SWISS
    qDebug() << "  yp" << yp << "xp" << xp;
#endif

    double lon = 2.6779094
                 + 4.728982*yp
                 + 0.791484*yp*xp
                 + 0.1306*yp*pow(xp, 2)
                 - 0.0436*pow(yp, 3);
    double lat = 16.9023892
                 + 3.238272*xp
                 - 0.270978*pow(yp, 2)
                 - 0.002528*pow(xp, 2)
                 - 0.0447*pow(yp, 2)*xp
                 - 0.0140*pow(xp, 3);
#ifdef DEBUG_SWISS
    qDebug() << "  lon" << lon << "lat" << lat;
#endif
    *lonp = lon*100/36;
    *latp = lat*100/36;
#ifdef DEBUG_SWISS
    qDebug() << "Swiss: result lon" << *lonp << "lat" << *latp;
#endif
}


SwissCoordinateHandler::SwissCoordinateHandler(QObject *pnt)
    : AbstractCoordinateHandler(pnt)
{
    qDebug();
    setObjectName("SwissCoordinateHandler");
}


QWidget *SwissCoordinateHandler::createWidget(QWidget *pnt)
{
    QWidget *w = new QWidget(pnt);
    QFormLayout *fl = new QFormLayout(w);

    mSwissNorthEdit = new QLineEdit(w);
    connect(mSwissNorthEdit, &QLineEdit::textEdited, this, &SwissCoordinateHandler::slotTextChanged);
    QIntValidator *iv = new QIntValidator(mSwissNorthEdit);
    iv->setRange(MIN_SWISS_NORTHING, MAX_SWISS_NORTHING);
    mSwissNorthEdit->setValidator(iv);
    fl->addRow(i18n("Northing:"), mSwissNorthEdit);

    mSwissEastEdit = new QLineEdit(w);
    connect(mSwissEastEdit, &QLineEdit::textEdited, this, &SwissCoordinateHandler::slotTextChanged);
    iv = new QIntValidator(mSwissEastEdit);
    iv->setRange(MIN_SWISS_EASTING, MAX_SWISS_EASTING);
    mSwissEastEdit->setValidator(iv);
    fl->addRow(i18n("Easting:"), mSwissEastEdit);

    return (w);
}


void SwissCoordinateHandler::updateGUI(double lat, double lon)
{
    qDebug() << lat << lon;

    if (!ISNAN(lat) && !ISNAN(lon))
    {
        setSwiss(lat, lon, mSwissEastEdit, mSwissNorthEdit);
    }
    else
    {
        mSwissNorthEdit->clear();
        mSwissEastEdit->clear();
    }
}


void SwissCoordinateHandler::checkError()
{
    AbstractCoordinateHandler::checkError();

    int yy = mSwissEastEdit->text().toInt();
    int xx = mSwissNorthEdit->text().toInt();

    if (yy<MIN_SWISS_EASTING || yy>MAX_SWISS_EASTING ||
        xx<MIN_SWISS_NORTHING || xx>MAX_SWISS_NORTHING)
    {
        setError(i18n("Coordinates out of range for Switzerland"));
    }
}


bool SwissCoordinateHandler::hasAcceptableInput() const
{
    // We can't verify the Swiss entries here, as they may be
    // out of range outside that country.  Relying on the other
    // coordinate formats.
    return (true);
}


void SwissCoordinateHandler::slotTextChanged()
{
    double lat;
    double lon;
    getSwiss(mSwissEastEdit, mSwissNorthEdit, &lat, &lon);
    updateValues(lat, lon);
    checkError();
}


QString SwissCoordinateHandler::tabName() const
{
    return (i18nc("@title:tab", "Swiss"));
}
