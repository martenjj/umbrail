
#include "latlongwidget.h"

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
#include <qdebug.h>
#include <qstyle.h>

#include <QIntValidator>
#include <QDoubleValidator>

#include <klocalizedstring.h>
#include <kconfiggroup.h>
#include <ksharedconfig.h>
#include <kstandardaction.h>
#include <kmessagebox.h>

#include <dialogbase.h>

#include "trackdata.h"

//  ---------------------------------------------------------------------------

AbstractCoordinateHandler::AbstractCoordinateHandler(QObject *pnt)
    : QObject(pnt)
{
    qDebug();
}


void AbstractCoordinateHandler::setLatLong(double lat, double lon)
{
    QSignalBlocker blocker(this);

    qDebug() << lat << lon;

    mLatitude = lat;
    mLongitude = lon;
    updateGUI(lat, lon);
}


void AbstractCoordinateHandler::updateValues(double lat, double lon)
{
    qDebug() << lat << lon;

    mLatitude = lat;
    mLongitude = lon;
    emit valueChanged();
}

//  ---------------------------------------------------------------------------

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

//  ---------------------------------------------------------------------------

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

//  ---------------------------------------------------------------------------

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
    QWidget *w = new QWidget;
    QFormLayout *fl = new QFormLayout(w);

    mSwissNorthEdit = new QLineEdit(w);
    connect(mSwissNorthEdit, SIGNAL(textEdited(const QString &)), SLOT(slotTextChanged()));
    QIntValidator *iv = new QIntValidator(mSwissNorthEdit);
    iv->setRange(MIN_SWISS_NORTHING, MAX_SWISS_NORTHING);
    mSwissNorthEdit->setValidator(iv);
    fl->addRow(i18n("Northing:"), mSwissNorthEdit);

    mSwissEastEdit = new QLineEdit(w);
    connect(mSwissEastEdit, SIGNAL(textEdited(const QString &)), SLOT(slotTextChanged()));
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


bool SwissCoordinateHandler::hasAcceptableInput() const
{
    // We can't verify the Swiss entries here, as they may be
    // out of range outside that country.  Relying on the other
    // coordinate formats.
    return (true);
}


void SwissCoordinateHandler::slotTextChanged()
{
    qDebug();

    double lat;
    double lon;
    getSwiss(mSwissEastEdit, mSwissNorthEdit, &lat, &lon);
    updateValues(lat, lon);
}


QString SwissCoordinateHandler::tabName() const
{
    return (i18nc("@title:tab", "Swiss"));
}

//  ---------------------------------------------------------------------------

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
    DecimalCoordinateHandler *handler1 = new DecimalCoordinateHandler(this);
    QWidget *w = handler1->createWidget(this);
    connect(handler1, SIGNAL(valueChanged()), SLOT(slotValueChanged()));
    mHandlers.append(handler1);
    mTabs->addTab(w, handler1->tabName());

    // Tab for "DMS" format
    DMSCoordinateHandler *handler2 = new DMSCoordinateHandler(this);
    w = handler2->createWidget(this);
    connect(handler2, SIGNAL(valueChanged()), SLOT(slotValueChanged()));
    mHandlers.append(handler2);
    mTabs->addTab(w, handler2->tabName());

    // Tab for "Swiss" format
    SwissCoordinateHandler *handler3 = new SwissCoordinateHandler(this);
    w = handler3->createWidget(this);
    connect(handler3, SIGNAL(valueChanged()), SLOT(slotValueChanged()));
    mHandlers.append(handler3);
    mTabs->addTab(w, handler3->tabName());

    // "Paste" button
    QAction *act = KStandardAction::paste(this);
    QPushButton *pasteButton = new QPushButton(act->icon(), act->text(), this);
    connect(pasteButton, SIGNAL(clicked()), SLOT(slotPasteCoordinates()));
    hb->addWidget(pasteButton);

    KConfigGroup grp = KSharedConfig::openConfig()->group(objectName());
    int idx = grp.readEntry("Index", -1);
    if (idx!=-1) mTabs->setCurrentIndex(idx);
}


LatLongWidget::~LatLongWidget()
{
    KConfigGroup grp = KSharedConfig::openConfig()->group(objectName());
    grp.writeEntry("Index", mTabs->currentIndex());
}


void LatLongWidget::setLatLong(double lat, double lon)
{
    mLatitude = lat;
    mLongitude = lon;

    foreach (AbstractCoordinateHandler *handler, mHandlers)
    {
        handler->setLatLong(lat, lon);
    }
}


void LatLongWidget::slotValueChanged()
{
    AbstractCoordinateHandler *changedHandler = qobject_cast<AbstractCoordinateHandler *>(sender());
    if (changedHandler==NULL || !mHandlers.contains(changedHandler))
    {
        qWarning() << "called by unknown handler" << changedHandler;
        return;
    }

    mLatitude = changedHandler->getLatitude();
    mLongitude = changedHandler->getLongitude();

    foreach (AbstractCoordinateHandler *handler, mHandlers)
    {
        // apart from the one just changed
        if (handler!=changedHandler) handler->setLatLong(mLatitude, mLongitude);
    }

    textChanged();
}


void LatLongWidget::textChanged()
{
    const bool valid = hasAcceptableInput();
    emit positionValid(valid);
    if (valid) emit positionChanged(mLatitude, mLongitude);
}


bool LatLongWidget::hasAcceptableInput() const
{
    bool ok = true;					// assume so to start

    foreach (AbstractCoordinateHandler *handler, mHandlers)
    {
        if (!handler->hasAcceptableInput()) ok = false;
    }
    return (ok);
}


void LatLongWidget::slotPasteCoordinates()
{
    QString text = QApplication::clipboard()->text().simplified();
    qDebug() << text;
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
        textChanged();
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
        textChanged();
        return;
    }

    KMessageBox::sorry(this, i18n("Coordinate format not recognised"));
}
