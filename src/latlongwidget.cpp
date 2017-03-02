
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

#include <kdialog.h>
#include <klocale.h>
#include <kconfiggroup.h>
#include <kglobal.h>


#define PRECISION		6			// how many decimal places

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
//  Currently the implementation here is for coordinate system CH1903/LV03.
//  SwissTopo section 1.1 says that the new CH1903+/LV95 system is mandated
//  for official use from 2016 onwards.
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
    gl->setColumnMinimumWidth(1, KDialog::spacingHint());
    gl->setColumnMinimumWidth(4, KDialog::spacingHint());
    gl->setColumnMinimumWidth(7, KDialog::spacingHint());
    gl->setColumnMinimumWidth(10, KDialog::spacingHint());
    gl->setColumnStretch(12, 1);

    mTabs->addTab(w, i18n("DMS"));

    // Tab for "Swiss" format
    w = new QWidget(this);
    fl = new QFormLayout(w);

    mSwissNorthEdit = new QLineEdit(this);
    connect(mSwissNorthEdit, SIGNAL(textEdited(const QString &)), SLOT(slotSwissTextChanged()));
    iv = new QIntValidator(mSwissNorthEdit);
    iv->setRange(MIN_SWISS_NORTHING, MAX_SWISS_NORTHING);
    mSwissNorthEdit->setValidator(dv);
    fl->addRow(i18n("Northing:"), mSwissNorthEdit);

    mSwissEastEdit = new QLineEdit(this);
    connect(mSwissEastEdit, SIGNAL(textEdited(const QString &)), SLOT(slotSwissTextChanged()));
    iv = new QIntValidator(mSwissEastEdit);
    iv->setRange(MIN_SWISS_EASTING, MAX_SWISS_EASTING);
    mSwissEastEdit->setValidator(iv);
    fl->addRow(i18n("Easting:"), mSwissEastEdit);

    mTabs->addTab(w, i18n("Swiss"));

    // "Paste" button
    QAction *act = KStandardAction::paste(this);
    QPushButton *pasteButton = new QPushButton(act->icon(), act->text(), this);
    connect(pasteButton, SIGNAL(clicked()), SLOT(slotPasteCoordinates()));
    hb->addWidget(pasteButton);

    KConfigGroup grp = KGlobal::config()->group(objectName());
    int idx = grp.readEntry("Index", -1);
    if (idx!=-1) mTabs->setCurrentIndex(idx);
}


LatLongWidget::~LatLongWidget()
{
    KConfigGroup grp = KGlobal::config()->group(objectName());
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

    if (!ISNAN(mLatitude) && !ISNAN(mLongitude))
    {
        setSwiss(mLatitude, mLongitude, mSwissEastEdit, mSwissNorthEdit);
    }
    else
    {
        mSwissNorthEdit->clear();
        mSwissEastEdit->clear();
    }
}


void LatLongWidget::setSwiss(double lat, double lon,
                             QLineEdit *east, QLineEdit *north)
{
    // SwissTopo section 4.1
    double phi = mLatitude*3600;			// degrees -> arcseconds
    double lambda = mLongitude*3600;
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


void LatLongWidget::getSwiss(QLineEdit *east, QLineEdit *north,
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

    setSwiss(mLatitude, mLongitude, mSwissEastEdit, mSwissNorthEdit);

    textChanged();
}


void LatLongWidget::slotDmsTextChanged()
{
    mLatitude = getDMS(mLatitudeDeg, mLatitudeMin, mLatitudeSec, mLatitudeCombo);
    mLongitude = getDMS(mLongitudeDeg, mLongitudeMin, mLongitudeSec, mLongitudeCombo);

    mLatitudeEdit->setText(QString::number(mLatitude, 'f', PRECISION));
    mLongitudeEdit->setText(QString::number(mLongitude, 'f', PRECISION));

    setSwiss(mLatitude, mLongitude, mSwissEastEdit, mSwissNorthEdit);

    textChanged();
}


void LatLongWidget::slotSwissTextChanged()
{
    getSwiss(mSwissEastEdit, mSwissNorthEdit, &mLatitude, &mLongitude);

    mLatitudeEdit->setText(QString::number(mLatitude, 'f', PRECISION));
    mLongitudeEdit->setText(QString::number(mLongitude, 'f', PRECISION));
    setDMS(mLatitude, mLatitudeDeg, mLatitudeMin, mLatitudeSec, mLatitudeCombo);
    setDMS(mLongitude, mLongitudeDeg, mLongitudeMin, mLongitudeSec, mLongitudeCombo);

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

    // We don't verify the Swiss entries here, as they may be
    // out of range outside that country.  Relying on the above.

    return (ok);
}
