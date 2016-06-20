
#include "latlongwidget.h"

#include <math.h>

#include <qlineedit.h>
#include <qlabel.h>
#include <qgridlayout.h>
#include <qformlayout.h>
#include <qtabwidget.h>
#include <qcombobox.h>
#include <qpushbutton.h>
#include <qclipboard.h>
#include <qapplication.h>
#include <qregexp.h>

#include <QIntValidator>
#include <QDoubleValidator>

#include <kdebug.h>
#include <kdialog.h>
#include <klocale.h>
#include <kconfiggroup.h>
#include <kglobal.h>
#include <kaction.h>
#include <kstandardaction.h>
#include <kmessagebox.h>


#define PRECISION	6				// how many decimal places


LatLongWidget::LatLongWidget(QWidget *pnt)
    : QFrame(pnt)
{
    setObjectName("LatLongWidget");

    // Tab container
    mTabs = new QTabWidget(this);
    QHBoxLayout *hb = new QHBoxLayout(this);
    hb->setMargin(0);
    hb->addWidget(mTabs);

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

    // "Paste" button
    KAction *act = KStandardAction::paste(this);
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


void LatLongWidget::slotPasteCoordinates()
{
    QString text = QApplication::clipboard()->text().simplified();
    kDebug() << text;
    if (text.isEmpty())					// nothing to paste
    {
        KMessageBox::sorry(this, i18n("Nothing (or not text) to paste"));
        return;
    }

    QRegExp rx1("^(\\d+\\.\\d+)\\D+(\\d+\\.\\d+)");
    if (text.contains(rx1))				// try match in decimal format
    {
        double lat = rx1.cap(1).toDouble();		// assume success, because
        double lon = rx1.cap(2).toDouble();		// of regexp match above
        setLatLong(lat, lon);
        return;
    }

    QRegExp rx2("^(\\d+)\\D+(\\d+)\\D(\\d+(\\.\\d+))\\D*([NnSs])\\D+(\\d+)\\D+(\\d+)\\D(\\d+(\\.\\d+))\\D*([EeWw])");
    if (text.contains(rx2))				// try match in DMS format
    {
        int latD = rx2.cap(1).toInt();
        int latM = rx2.cap(2).toInt();
        double latS = rx2.cap(3).toDouble();
        QChar latSign = (rx2.cap(5).left(1).toUpper())[0];

        int lonD = rx2.cap(6).toInt();
        int lonM = rx2.cap(7).toInt();
        double lonS = rx2.cap(8).toDouble();
        QChar lonSign = (rx2.cap(10).left(1).toUpper())[0];

        double lat = latD+(latM/60.0)+(latS/3600.0);
        if (latSign=='S') lat = -lat;

        double lon = lonD+(lonM/60.0)+(lonS/3600.0);
        if (lonSign=='W') lon = -lon;

        setLatLong(lat, lon);
        return;
    }

    KMessageBox::sorry(this, i18n("Coordinate format not recognised"));
}
