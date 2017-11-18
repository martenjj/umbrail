
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
    QWidget *w = new QWidget;
    QFormLayout *fl = new QFormLayout(w);

    mLatitudeEdit = new QLineEdit(w);
    connect(mLatitudeEdit, SIGNAL(textEdited(const QString &)), SLOT(slotTextChanged(const QString &)));
    QDoubleValidator *dv = new QDoubleValidator(mLatitudeEdit);
    dv->setRange(-90, 90, PRECISION);
    mLatitudeEdit->setValidator(dv);
    fl->addRow(i18n("Latitude:"), mLatitudeEdit);

    mLongitudeEdit = new QLineEdit(w);
    connect(mLongitudeEdit, SIGNAL(textEdited(const QString &)), SLOT(slotTextChanged(const QString &)));
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
