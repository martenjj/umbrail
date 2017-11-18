
#include "dmscoordinate.h"

#include <qlineedit.h>
#include <qlabel.h>
#include <qgridlayout.h>
#include <qcombobox.h>
#include <qdebug.h>

#include <QIntValidator>
#include <QDoubleValidator>

#include <klocalizedstring.h>

#include <dialogbase.h>

#include "trackdata.h"


void DMSCoordinateHandler::setDMS(double d,
                                  QLineEdit *deg, QLineEdit *min,
                                  QLineEdit *sec, QComboBox *sign)
{
    sign->setCurrentIndex(d<0.0 ? 1 : 0);
    d = fabs(d);

    int dig = static_cast<int>(d);
    deg->setText(QString::number(dig));
    d = (d-dig)*60.0;
    dig = static_cast<int>(d);
    min->setText(QString::number(dig));
    d = (d-dig)*60.0;
    sec->setText(QString::number(d, 'f', 1));
}


double DMSCoordinateHandler::getDMS(QLineEdit *deg, QLineEdit *min,
                                    QLineEdit *sec, QComboBox *sign) const
{
    double d = sec->text().toDouble();
    d /= 60.0;
    d += min->text().toDouble();
    d /= 60.0;
    d += deg->text().toDouble();
    if (sign->currentIndex()==1) d = -d;

    return (d);
}


DMSCoordinateHandler::DMSCoordinateHandler(QObject *pnt)
    : AbstractCoordinateHandler(pnt)
{
    qDebug();
    setObjectName("DMSCoordinateHandler");
}


QWidget *DMSCoordinateHandler::createWidget(QWidget *pnt)
{
    QWidget *w = new QWidget;
    QGridLayout *gl = new QGridLayout(w);

    Qt::AlignmentFlag labelAlign = static_cast<Qt::AlignmentFlag>(w->style()->styleHint(QStyle::SH_FormLayoutLabelAlignment));

    // columns: 0     1     2   3 4     5   6 7     8   9 10    11    12
    //          label space deg 0 space min ' space sec " space combo stretch

    // row 0: latitude
    QLabel *l = new QLabel(i18n("Latitude:"), w);
    gl->addWidget(l, 0, 0, labelAlign);

    mLatitudeDeg = new QLineEdit(w);
    connect(mLatitudeDeg, SIGNAL(textEdited(const QString &)), SLOT(slotTextChanged()));
    QIntValidator *iv = new QIntValidator(mLatitudeDeg);
    iv->setRange(0, 90);
    mLatitudeDeg->setValidator(iv);
    gl->addWidget(mLatitudeDeg, 0, 2, Qt::AlignRight);
    l->setBuddy(mLatitudeDeg);

    l = new QLabel(QString(0xB0), w);
    gl->addWidget(l, 0, 3, Qt::AlignLeft);

    mLatitudeMin = new QLineEdit(w);
    connect(mLatitudeMin, SIGNAL(textEdited(const QString &)), SLOT(slotTextChanged()));
    iv = new QIntValidator(mLatitudeMin);
    iv->setRange(0, 59);
    mLatitudeMin->setValidator(iv);
    gl->addWidget(mLatitudeMin, 0, 5, Qt::AlignRight);

    l = new QLabel(QString('\''), w);
    gl->addWidget(l, 0, 6, Qt::AlignLeft);

    mLatitudeSec = new QLineEdit(w);
    connect(mLatitudeSec, SIGNAL(textEdited(const QString &)), SLOT(slotTextChanged()));
    QDoubleValidator *dv = new QDoubleValidator(mLatitudeSec);
    dv->setRange(0, 59.99, 2);
    mLatitudeSec->setValidator(dv);
    gl->addWidget(mLatitudeSec, 0, 8, Qt::AlignRight);

    l = new QLabel(QString('\"'), w);
    gl->addWidget(l, 0, 9, Qt::AlignLeft);

    mLatitudeCombo = new QComboBox(w);
    connect(mLatitudeCombo, SIGNAL(activated(int)), SLOT(slotTextChanged()));
    mLatitudeCombo->addItem(i18n("North"));
    mLatitudeCombo->addItem(i18n("South"));
    gl->addWidget(mLatitudeCombo, 0, 11);

    // row 1: longitude
    l = new QLabel(i18n("Longitude:"), w);
    gl->addWidget(l, 1, 0, labelAlign);

    mLongitudeDeg = new QLineEdit(w);
    connect(mLongitudeDeg, SIGNAL(textEdited(const QString &)), SLOT(slotTextChanged()));
    iv = new QIntValidator(mLongitudeDeg);
    iv->setRange(0, 180);
    mLongitudeDeg->setValidator(iv);
    gl->addWidget(mLongitudeDeg, 1, 2, Qt::AlignRight);
    l->setBuddy(mLongitudeDeg);

    l = new QLabel(QString(0xB0), w);
    gl->addWidget(l, 1, 3, Qt::AlignLeft);

    mLongitudeMin = new QLineEdit(w);
    connect(mLongitudeMin, SIGNAL(textEdited(const QString &)), SLOT(slotTextChanged()));
    iv = new QIntValidator(mLongitudeMin);
    iv->setRange(0, 59);
    mLongitudeMin->setValidator(iv);
    gl->addWidget(mLongitudeMin, 1, 5, Qt::AlignRight);

    l = new QLabel(QString('\''), w);
    gl->addWidget(l, 1, 6, Qt::AlignLeft);

    mLongitudeSec = new QLineEdit(w);
    connect(mLongitudeSec, SIGNAL(textEdited(const QString &)), SLOT(slotTextChanged()));
    dv = new QDoubleValidator(mLongitudeSec);
    dv->setRange(0, 59.99, 2);
    mLongitudeSec->setValidator(dv);
    gl->addWidget(mLongitudeSec, 1, 8, Qt::AlignRight);

    l = new QLabel(QString('\"'), w);
    gl->addWidget(l, 1, 9, Qt::AlignLeft);

    mLongitudeCombo = new QComboBox(w);
    connect(mLongitudeCombo, SIGNAL(activated(int)), SLOT(slotTextChanged()));
    mLongitudeCombo->addItem(i18n("East"));
    mLongitudeCombo->addItem(i18n("West"));
    gl->addWidget(mLongitudeCombo, 1, 11);

    // layout adjustment
    gl->setColumnMinimumWidth(1, DialogBase::horizontalSpacing());
    gl->setColumnMinimumWidth(4, DialogBase::horizontalSpacing());
    gl->setColumnMinimumWidth(7, DialogBase::horizontalSpacing());
    gl->setColumnMinimumWidth(10, DialogBase::horizontalSpacing());
    gl->setColumnStretch(12, 1);

    return (w);
}


void DMSCoordinateHandler::updateGUI(double lat, double lon)
{
    qDebug() << lat << lon;

    if (!ISNAN(lat))
    {
        setDMS(lat, mLatitudeDeg, mLatitudeMin, mLatitudeSec, mLatitudeCombo);
    }
    else
    {
        mLatitudeDeg->clear();
        mLatitudeMin->clear();
        mLatitudeSec->clear();
    }

    if (!ISNAN(lon))
    {
        setDMS(lon, mLongitudeDeg, mLongitudeMin, mLongitudeSec, mLongitudeCombo);
    }
    else
    {
        mLongitudeDeg->clear();
        mLongitudeMin->clear();
        mLongitudeSec->clear();
    }
}


bool DMSCoordinateHandler::hasAcceptableInput() const
{
    return (mLatitudeDeg->hasAcceptableInput() &&
            mLatitudeMin->hasAcceptableInput() &&
            mLatitudeSec->hasAcceptableInput() &&
            mLongitudeDeg->hasAcceptableInput() &&
            mLongitudeMin->hasAcceptableInput() &&
            mLongitudeSec->hasAcceptableInput());
}


void DMSCoordinateHandler::slotTextChanged()
{
    qDebug();
    updateValues(getDMS(mLatitudeDeg, mLatitudeMin, mLatitudeSec, mLatitudeCombo),
                 getDMS(mLongitudeDeg, mLongitudeMin, mLongitudeSec, mLongitudeCombo));
}


QString DMSCoordinateHandler::tabName() const
{
    return (i18nc("@title:tab", "DMS"));
}
