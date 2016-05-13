
#include "latlongwidget.h"

#include <math.h>

#include <qlineedit.h>
#include <qlabel.h>
#include <qgridlayout.h>
#include <qformlayout.h>
#include <qtabwidget.h>
#include <qcombobox.h>
#include <QIntValidator>
#include <QDoubleValidator>

#include <klocalizedstring.h>
#include <kconfiggroup.h>
#include <ksharedconfig.h>

#include <dialogbase.h>


#define PRECISION	6				// how many decimal places


LatLongWidget::LatLongWidget(QWidget *pnt)
    : QFrame(pnt)
{
    setObjectName("LatLongWidget");

    // Tab container
    mTabs = new QTabWidget(this);
    QVBoxLayout *vb = new QVBoxLayout(this);
    vb->setMargin(0);
    vb->addWidget(mTabs);

    // Tab for "Decimal" format
    QWidget *w = new QWidget(this);
    QFormLayout *fl = new QFormLayout(w);

    mLatitudeEdit = new QLineEdit(this);
    connect(mLatitudeEdit, SIGNAL(textEdited(const QString &)), SLOT(slotDecimalTextChanged()));
    QDoubleValidator *dv = new QDoubleValidator(mLatitudeEdit);
    dv->setRange(-90, 90, PRECISION);
    mLatitudeEdit->setValidator(dv);
    fl->addRow(i18n("Latitude:"), mLatitudeEdit);

    mLongitudeEdit = new QLineEdit(this);
    connect(mLongitudeEdit, SIGNAL(textEdited(const QString &)), SLOT(slotDecimalTextChanged()));
    dv = new QDoubleValidator(mLongitudeEdit);
    dv->setRange(-180, 180, PRECISION);
    mLongitudeEdit->setValidator(dv);
    fl->addRow(i18n("Longitude:"), mLongitudeEdit);

    mTabs->addTab(w, i18n("Decimal"));

    // Tab for "DMS" format
    w = new QWidget(this);
    QGridLayout *gl = new QGridLayout(w);

    // columns: 0     1     2   3 4     5   6 7     8   9 10    11    12
    //          label space deg 0 space min ' space sec " space combo stretch

    // row 0: latitude
    QLabel *l = new QLabel(i18n("Latitude:"), w);
    gl->addWidget(l, 0, 0, fl->labelAlignment());

    mLatitudeDeg = new QLineEdit(w);
    connect(mLatitudeDeg, SIGNAL(textEdited(const QString &)), SLOT(slotDmsTextChanged()));
    QIntValidator *iv = new QIntValidator(mLatitudeDeg);
    iv->setRange(0, 90);
    mLatitudeDeg->setValidator(iv);
    gl->addWidget(mLatitudeDeg, 0, 2, Qt::AlignRight);
    l->setBuddy(mLatitudeDeg);

    l = new QLabel(QString(0xB0), w);
    gl->addWidget(l, 0, 3, Qt::AlignLeft);

    mLatitudeMin = new QLineEdit(w);
    connect(mLatitudeMin, SIGNAL(textEdited(const QString &)), SLOT(slotDmsTextChanged()));
    iv = new QIntValidator(mLatitudeMin);
    iv->setRange(0, 59);
    mLatitudeMin->setValidator(iv);
    gl->addWidget(mLatitudeMin, 0, 5, Qt::AlignRight);

    l = new QLabel(QString('\''), w);
    gl->addWidget(l, 0, 6, Qt::AlignLeft);

    mLatitudeSec = new QLineEdit(w);
    connect(mLatitudeSec, SIGNAL(textEdited(const QString &)), SLOT(slotDmsTextChanged()));
    dv = new QDoubleValidator(mLatitudeSec);
    dv->setRange(0, 59.99, 2);
    mLatitudeSec->setValidator(dv);
    gl->addWidget(mLatitudeSec, 0, 8, Qt::AlignRight);

    l = new QLabel(QString('\"'), w);
    gl->addWidget(l, 0, 9, Qt::AlignLeft);

    mLatitudeCombo = new QComboBox(w);
    connect(mLatitudeCombo, SIGNAL(activated(int)), SLOT(slotDmsTextChanged()));
    mLatitudeCombo->addItem(i18n("North"));
    mLatitudeCombo->addItem(i18n("South"));
    gl->addWidget(mLatitudeCombo, 0, 11);

    // row 1: longitude
    l = new QLabel(i18n("Longitude:"), w);
    gl->addWidget(l, 1, 0, fl->labelAlignment());

    mLongitudeDeg = new QLineEdit(w);
    connect(mLongitudeDeg, SIGNAL(textEdited(const QString &)), SLOT(slotDmsTextChanged()));
    iv = new QIntValidator(mLongitudeDeg);
    iv->setRange(0, 180);
    mLongitudeDeg->setValidator(iv);
    gl->addWidget(mLongitudeDeg, 1, 2, Qt::AlignRight);
    l->setBuddy(mLongitudeDeg);

    l = new QLabel(QString(0xB0), w);
    gl->addWidget(l, 1, 3, Qt::AlignLeft);

    mLongitudeMin = new QLineEdit(w);
    connect(mLongitudeMin, SIGNAL(textEdited(const QString &)), SLOT(slotDmsTextChanged()));
    iv = new QIntValidator(mLongitudeMin);
    iv->setRange(0, 59);
    mLongitudeMin->setValidator(iv);
    gl->addWidget(mLongitudeMin, 1, 5, Qt::AlignRight);

    l = new QLabel(QString('\''), w);
    gl->addWidget(l, 1, 6, Qt::AlignLeft);

    mLongitudeSec = new QLineEdit(w);
    connect(mLongitudeSec, SIGNAL(textEdited(const QString &)), SLOT(slotDmsTextChanged()));
    dv = new QDoubleValidator(mLongitudeSec);
    dv->setRange(0, 59.99, 2);
    mLongitudeSec->setValidator(dv);
    gl->addWidget(mLongitudeSec, 1, 8, Qt::AlignRight);

    l = new QLabel(QString('\"'), w);
    gl->addWidget(l, 1, 9, Qt::AlignLeft);

    mLongitudeCombo = new QComboBox(w);
    connect(mLongitudeCombo, SIGNAL(activated(int)), SLOT(slotDmsTextChanged()));
    mLongitudeCombo->addItem(i18n("East"));
    mLongitudeCombo->addItem(i18n("West"));
    gl->addWidget(mLongitudeCombo, 1, 11);

    // layout adjustment
    gl->setColumnMinimumWidth(1, DialogBase::horizontalSpacing());
    gl->setColumnMinimumWidth(4, DialogBase::horizontalSpacing());
    gl->setColumnMinimumWidth(7, DialogBase::horizontalSpacing());
    gl->setColumnMinimumWidth(10, DialogBase::horizontalSpacing());
    gl->setColumnStretch(12, 1);

    mTabs->addTab(w, i18n("DMS"));

    KConfigGroup grp = KSharedConfig::openConfig()->group(objectName());
    int idx = grp.readEntry("Index", -1);
    if (idx!=-1) mTabs->setCurrentIndex(idx);
}


LatLongWidget::~LatLongWidget()
{
    KConfigGroup grp = KSharedConfig::openConfig()->group(objectName());
    grp.writeEntry("Index", mTabs->currentIndex());
}


void LatLongWidget::setDMS(double d,
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


double LatLongWidget::getDMS(QLineEdit *deg, QLineEdit *min,
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


void LatLongWidget::setLatLong(double lat, double lon)
{
    mLatitude = lat;
    mLongitude = lon;

    if (!isnan(mLatitude))
    {
        mLatitudeEdit->setText(QString::number(mLatitude, 'f', PRECISION));
        setDMS(mLatitude, mLatitudeDeg, mLatitudeMin, mLatitudeSec, mLatitudeCombo);
    }
    else
    {
        mLatitudeEdit->clear();
        mLatitudeDeg->clear();
        mLatitudeMin->clear();
        mLatitudeSec->clear();
    }

    if (!isnan(mLongitude))
    {
        mLongitudeEdit->setText(QString::number(mLongitude, 'f', PRECISION));
        setDMS(mLongitude, mLongitudeDeg, mLongitudeMin, mLongitudeSec, mLongitudeCombo);
    }
    else
    {
        mLongitudeEdit->clear();
        mLongitudeDeg->clear();
        mLongitudeMin->clear();
        mLongitudeSec->clear();
    }
}


void LatLongWidget::textChanged()
{
    const bool valid = hasAcceptableInput();
    emit positionValid(valid);
    if (valid) emit positionChanged(mLatitude, mLongitude);
}


void LatLongWidget::slotDecimalTextChanged()
{
    mLatitude = mLatitudeEdit->text().toDouble();
    mLongitude = mLongitudeEdit->text().toDouble();

    setDMS(mLatitude, mLatitudeDeg, mLatitudeMin, mLatitudeSec, mLatitudeCombo);
    setDMS(mLongitude, mLongitudeDeg, mLongitudeMin, mLongitudeSec, mLongitudeCombo);
    textChanged();
}


void LatLongWidget::slotDmsTextChanged()
{
    mLatitude = getDMS(mLatitudeDeg, mLatitudeMin, mLatitudeSec, mLatitudeCombo);
    mLongitude = getDMS(mLongitudeDeg, mLongitudeMin, mLongitudeSec, mLongitudeCombo);

    mLatitudeEdit->setText(QString::number(mLatitude, 'f', PRECISION));
    mLongitudeEdit->setText(QString::number(mLongitude, 'f', PRECISION));
    textChanged();
}


bool LatLongWidget::hasAcceptableInput() const
{
    bool ok = true;					// assume so to start

    if (!mLatitudeEdit->hasAcceptableInput()) ok = false;
    if (!mLongitudeEdit->hasAcceptableInput()) ok = false;

    if (!mLatitudeDeg->hasAcceptableInput()) ok = false;
    if (!mLatitudeMin->hasAcceptableInput()) ok = false;
    if (!mLatitudeSec->hasAcceptableInput()) ok = false;

    if (!mLongitudeDeg->hasAcceptableInput()) ok = false;
    if (!mLongitudeMin->hasAcceptableInput()) ok = false;
    if (!mLongitudeSec->hasAcceptableInput()) ok = false;

    return (ok);
}
