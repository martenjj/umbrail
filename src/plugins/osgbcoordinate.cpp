
#include "osgbcoordinate.h"

#include <math.h>

#include <qlineedit.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qgridlayout.h>
#include <qregexp.h>
#include <qdebug.h>

#include <QIntValidator>

#include <klocalizedstring.h>

#include <dialogbase.h>

#include "trackdata.h"


//  The reference document for the OSGB coordinate system is
//  "A guide to coordinate systems in Great Britain"
//
//  https://www.ordnancesurvey.co.uk/docs/support/
//    guide-coordinate-systems-great-britain.pdf
//
//  References to "OSGB" are to this document.
//
//  For background information and an online converter see also
//
//    https://www.movable-type.co.uk/scripts/latlong-os-gridref.html
//
//  References to "MT" are to this page.


//  Constants and definitions

#define MIN_OSGB_EASTING	0
#define MAX_OSGB_EASTING	(7*100*1000)		// squares of 100km in metres from SV
#define MIN_OSGB_NORTHING	0
#define MAX_OSGB_NORTHING	(13*100*1000)		// squares of 100km in metres from SV

#define OSGB_DIGITS		(6/2)			// digits for display of reference

#define DEBUG_OSGB

//  Valid grid squares for GB coverage

static const char validSquares[] = "HP HT HU HW HX HY HZ "
                                   "NA NB NC ND NF NG NH NJ NK NL NM NN NO NR NS NT NU NW NX NY NZ "
                                   "OV "
                                   "SC SD SE SH SJ SK SM SN SO SP SR SS ST SU SV SW SX SY SZ "
                                   "TA TF TG TL TM TQ TR TV ";

// Structures for coordinate passing

struct Vector3D						// 3D Cartesian coordinates
{
    double x;
    double y;
    double z;
};

struct LatLon						// Latitude/longitude
{
    double lat;
    double lon;
};

struct OSGBRef						// OSGB eastings/northings
{
    int E;
    int N;
};

struct Transform					// Helmert transform matrix
{
    double t[7];
};


//  Converts point from (geodetic) latitude/longitude coordinates
//  to (geocentric) cartesian (x/y/z) coordinates.
//
//  Returns a Vector[x y z] pointing to lat/lon point,
//  with x, y, z in metres from earth centre.
//
// from MM LatLon.prototype.toCartesian()

static Vector3D toCartesian(double lat, double lon)
{
    const double phi = DEGREES_TO_RADIANS(lat);
    const double lambda = DEGREES_TO_RADIANS(lon);

    // from MM LatLon.ellipsoid for WGS84
    const double a = 6378137;
    const double f = 1.0/298.257223563;

    const double sinphi = sin(phi);
    const double cosphi = cos(phi);
    const double sinlambda = sin(lambda);
    const double coslambda = cos(lambda);

    const double eSq = 2*f - f*f;			// 1st eccentricity squared = (a²-b²)/a²
    const double v = a/sqrt(1 - eSq*sinphi*sinphi);	// radius of curvature in prime vertical

    Vector3D point;
    point.x = v*cosphi*coslambda;
    point.y = v*cosphi*sinlambda;
    point.z = v*(1-eSq)*sinphi;

    return (point);
}


// from MM LatLon.datum for OSGB36

static Transform getOSGB36Transform()
{
    Transform transform;
    transform.t[0] = -446.448;
    transform.t[1] = 125.157;
    transform.t[2] = -542.060;
    transform.t[3] = 20.4894;
    transform.t[4] = -0.1502;
    transform.t[5] = -0.2470;
    transform.t[6] = -0.8421;
    return (transform);
}


//  Applies Helmert transform to 'point' using transform parameters 't'.
//
//  from MM Vector3d.prototype.applyTransform()

static Vector3D applyTransform(const Vector3D &point, const Transform &t)
{
    // this point
    const double x1 = point.x;
    const double y1 = point.y;
    const double z1 = point.z;

    // transform parameters
    const double tx = t.t[0];				// x-shift
    const double ty = t.t[1];				// y-shift
    const double tz = t.t[2];				// z-shift
    const double s1 = t.t[3]/1e6 + 1;			// scale: normalise parts-per-million to (s+1)
    const double rx = DEGREES_TO_RADIANS(t.t[4]/3600);	// x-rotation: normalise arcseconds to radians
    const double ry = DEGREES_TO_RADIANS(t.t[5]/3600);	// y-rotation: normalise arcseconds to radians
    const double rz = DEGREES_TO_RADIANS(t.t[6]/3600);	// z-rotation: normalise arcseconds to radians

    // apply transform
    Vector3D result;
    result.x = tx + x1*s1 - y1*rz + z1*ry;
    result.y = ty + x1*rz + y1*s1 - z1*rx;
    result.z = tz - x1*ry + y1*rx + z1*s1;
    return (result);
}


//  Converts (geocentric) cartesian (x/y/z) point to
//  (ellipsoidal geodetic) latitude/longitude coordinates.
//
//  Uses Bowring’s (1985) formulation for μm precision in concise form.
//
//  from MM Vector3d.prototype.toLatLonE()

static LatLon toLatLonE(const Vector3D &point)
{
    const double x = point.x;
    const double y = point.y;
    const double z = point.z;

    // from MM LatLon.ellipsoid for OSGB36 (Airy1830)
    const double a = 6377563.396;
    const double b = 6356256.909;
    const double f = 1/299.3249646;

    const double e2 = 2*f - f*f;			// 1st eccentricity squared = (a²-b²)/a²
    const double eps2 = e2/(1-e2);			// 2nd eccentricity squared = (a²-b²)/b²
    const double p = sqrt(x*x + y*y);			// distance from minor axis
    const double R = sqrt(p*p + z*z);			// polar radius

    // parametric latitude (Bowring eqn 17, replacing tanβ = z·a / p·b)
    const double tanbeta = (b*z)/(a*p)*(1+eps2*b/R);
    const double sinbeta = tanbeta/sqrt(1+tanbeta*tanbeta);
    const double cosbeta = sinbeta/tanbeta;

    // geodetic latitude (Bowring eqn 18: tanφ = z+ε²bsin³β / p−e²cos³β)
    const double phi = isnan(cosbeta) ? 0 : atan2(z+eps2*b*sinbeta*sinbeta*sinbeta, p-e2*a*cosbeta*cosbeta*cosbeta);

    // longitude
    const double lambda = atan2(y, x);

    LatLon result;
    result.lat = RADIANS_TO_DEGREES(phi);
    result.lon = RADIANS_TO_DEGREES(lambda);
    return (result);
}


//  Convert latitude/longitude to Ordnance Survey grid reference and
//  easting/northing coordinate.
//
//  Note: formulation implemented here due to Thomas, Redfearn, etc is
//  as published by OS, but is inferior to Krüger as used by e.g. Karney 2011.
//
// from MM OsGridRef.latLonToOsGrid()

static OSGBRef latLonToOSGrid(double lat, double lon)
{
#ifdef DEBUG_OSGB
    qDebug() << "lat" << lat << "lon" << lon;
#endif
    // Input is in WGS84.  Needs to be converted to OSGB36.
    //
    // from MM LatLon.prototype.convertDatum()

    // First convert polar to cartesian,
    Vector3D oldCartesian = toCartesian(lat, lon);

    // then apply transform to OSGB36,
    Transform transform = getOSGB36Transform();
    Vector3D newCartesian = applyTransform(oldCartesian, transform);

    // finally convert cartesian to polar
    LatLon newLatLon = toLatLonE(newCartesian);

    // continuing MM OsGridRef.latLonToOsGrid()

    const double phi = DEGREES_TO_RADIANS(newLatLon.lat);
    const double lambda = DEGREES_TO_RADIANS(newLatLon.lon);

    // from MM LatLon.ellipsoid for OSGB36 (Airy1830)
    const double a = 6377563.396;			// major semi-axis
    const double b = 6356256.909;			// minor semi-axis
    const double F0 = 0.9996012717;			// NatGrid scale factor on central meridian

    const double phi0 = DEGREES_TO_RADIANS(49);		// NatGrid true origin is 49°N 2°W
    const double lambda0 = DEGREES_TO_RADIANS(-2);

    const double N0 = -100000;				// northing & easting of true origin, metres
    const double E0 = 400000;
    const double e2 = 1 - (b*b)/(a*a);			// eccentricity squared
    const double n = (a-b)/(a+b);			// n, n², n³
    const double n2 = n*n;
    const double n3 = n*n*n;

    const double cosphi = cos(phi);
    const double sinphi = sin(phi);
    const double v = a*F0/sqrt(1-e2*sinphi*sinphi);		// transverse radius of curvature
    const double p = a*F0*(1-e2)/pow(1-e2*sinphi*sinphi, 1.5);	// meridional radius of curvature
    const double eta2 = v/p-1;

    const double Ma = (1 + n + (5/4)*n2 + (5/4)*n3) * (phi-phi0);
    const double Mb = (3*n + 3*n*n + (21/8)*n3) * sin(phi-phi0) * cos(phi+phi0);
    const double Mc = ((15/8)*n2 + (15/8)*n3) * sin(2*(phi-phi0)) * cos(2*(phi+phi0));
    const double Md = (35/24)*n3 * sin(3*(phi-phi0)) * cos(3*(phi+phi0));
    const double M = b * F0 * (Ma - Mb + Mc - Md);	// meridional arc

    const double cos3phi = cosphi*cosphi*cosphi;
    const double cos5phi = cos3phi*cosphi*cosphi;
    const double tan2phi = tan(phi)*tan(phi);
    const double tan4phi = tan2phi*tan2phi;

    const double I = M + N0;
    const double II = (v/2)*sinphi*cosphi;
    const double III = (v/24)*sinphi*cos3phi*(5-tan2phi+9*eta2);
    const double IIIA = (v/720)*sinphi*cos5phi*(61-58*tan2phi+tan4phi);
    const double IV = v*cosphi;
    const double V = (v/6)*cos3phi*(v/p-tan2phi);
    const double VI = (v/120) * cos5phi * (5 - 18*tan2phi + tan4phi + 14*eta2 - 58*tan2phi*eta2);

    const double deltalambda = lambda-lambda0;
    const double deltalambda2 = deltalambda*deltalambda;
    const double deltalambda3 = deltalambda2*deltalambda;
    const double deltalambda4 = deltalambda3*deltalambda;
    const double deltalambda5 = deltalambda4*deltalambda;
    const double deltalambda6 = deltalambda5*deltalambda;

    OSGBRef ref;
    ref.E = qRound(E0 + IV*deltalambda + V*deltalambda3 + VI*deltalambda5);
    ref.N = qRound(I + II*deltalambda2 + III*deltalambda4 + IIIA*deltalambda6);
#ifdef DEBUG_OSGB
    qDebug() << "-> E" << ref.E << "N" << ref.N;
#endif
    return (ref);
}


//  Converts Ordnance Survey grid reference easting/northing coordinate to
//  latitude/longitude (SW corner of grid square).
//
//  Note formulation implemented here due to Thomas, Redfearn, etc is
//  as published by OS, but is inferior to Krüger as used by e.g. Karney 2011.
//
// from MM OsGridRef.osGridToLatLon()

static LatLon getOSGBFromEN(const OSGBRef &ref)
{
    const int E = ref.E;
    const int N = ref.N;
#ifdef DEBUG_OSGB
    qDebug() << "E" << ref.E << "N" << ref.N;
#endif

    const double a = 6377563.396;			// Airy 1830 major & minor semi-axes
    const double b = 6356256.909;
    const double F0 = 0.9996012717;			// NatGrid scale factor on central meridian
    const double phi0 = DEGREES_TO_RADIANS(49);		// NatGrid true origin is 49°N 2°W
    const double lambda0 = DEGREES_TO_RADIANS(-2);
    const double N0 = -100000;				// northing & easting of true origin, metres
    const double E0 = 400000;
    const double e2 = 1 - (b*b)/(a*a);			 // eccentricity squared
    const double n = (a-b)/(a+b);			 // n, n², n³
    const double n2 = n*n;
    const double n3 = n*n*n;

    double phi = phi0;
    double M = 0;
#ifdef DEBUG_OSGB
    int iterCount = 0;
#endif
    do {
        phi = (N-N0-M)/(a*F0) + phi;

        const double Ma = (1 + n + (5/4)*n2 + (5/4)*n3) * (phi-phi0);
        const double Mb = (3*n + 3*n*n + (21/8)*n3) * sin(phi-phi0) * cos(phi+phi0);
        const double Mc = ((15/8)*n2 + (15/8)*n3) * sin(2*(phi-phi0)) * cos(2*(phi+phi0));
        const double Md = (35/24)*n3 * sin(3*(phi-phi0)) * cos(3*(phi+phi0));
        M = b * F0 * (Ma - Mb + Mc - Md);		// meridional arc
#ifdef DEBUG_OSGB
        ++iterCount;
#endif
    } while ((N-N0-M)>=0.00001);			// until < 0.01mm
#ifdef DEBUG_OSGB
    qDebug() << "took" << iterCount << "iterations";
#endif

    const double cosphi = cos(phi);
    const double sinphi = sin(phi);
    const double v = a*F0/sqrt(1-e2*sinphi*sinphi);		// transverse radius of curvature
    const double p = a*F0*(1-e2)/pow(1-e2*sinphi*sinphi, 1.5);	// meridional radius of curvature
    const double eta2 = v/p-1;

    const double tanphi = tan(phi);
    const double tan2phi = tanphi*tanphi;
    const double tan4phi = tan2phi*tan2phi;
    const double tan6phi = tan4phi*tan2phi;
    const double secphi = 1/cosphi;
    const double v3 = v*v*v;
    const double v5 = v3*v*v;
    const double v7 = v5*v*v;

    const double VII = tanphi/(2*p*v);
    const double VIII = tanphi/(24*p*v3)*(5+3*tan2phi+eta2-9*tan2phi*eta2);
    const double IX = tanphi/(720*p*v5)*(61+90*tan2phi+45*tan4phi);
    const double X = secphi/v;
    const double XI = secphi/(6*v3)*(v/p+2*tan2phi);
    const double XII = secphi/(120*v5)*(5+28*tan2phi+24*tan4phi);
    const double XIIA = secphi/(5040*v7)*(61+662*tan2phi+1320*tan4phi+720*tan6phi);

    const double dE = (E-E0);
    const double dE2 = dE*dE;
    const double dE3 = dE2*dE;
    const double dE4 = dE2*dE2;
    const double dE5 = dE3*dE2;
    const double dE6 = dE4*dE2;
    const double dE7 = dE5*dE2;
    phi = phi - VII*dE2 + VIII*dE4 - IX*dE6;
    const double lambda = lambda0 + X*dE - XI*dE3 + XII*dE5 - XIIA*dE7;

    // The output at this stage is in OSGB36.  Needs to be converted to WGS84.
    //
    // from MM LatLon.prototype.convertDatum()

    // First convert polar to cartesian,
    Vector3D oldCartesian = toCartesian(RADIANS_TO_DEGREES(phi), RADIANS_TO_DEGREES(lambda));
    // then get transform to OSGB36,
    Transform transform = getOSGB36Transform();
    // invert the transformation,
    for (int i = 0; i<=6; ++i) transform.t[i] = -transform.t[i];
    // then apply transformation,
    Vector3D newCartesian = applyTransform(oldCartesian, transform);
    // finally convert cartesian to polar
    LatLon newLatLon = toLatLonE(newCartesian);
#ifdef DEBUG_OSGB
    qDebug() << "-> lat" << newLatLon.lat << "lon" << newLatLon.lon;
#endif
    return (newLatLon);
}


//  Parses grid reference to an OSGB grid reference (easting, northing)
//
//  Accepts standard grid references (eg 'SU 387 148'), with or without
//  whitespace separators, from two-digit references up to 10-digit
//  references (1m×1m square), or fully numeric comma-separated
//  references in metres (eg '438700,114800').
//
// from MM OsGridRef.parse()

static bool getOSGBFromRef(const QString &refStr, OSGBRef *gridRef)
{
#ifdef DEBUG_OSGB
    qDebug() << refStr;
#endif
    // check for fully numeric comma-separated gridref format
    QRegExp rx1("^(\\d+),\\s*(\\d+)$");
    if (refStr.contains(rx1))
    {
        gridRef->E = rx1.cap(1).toInt();
        gridRef->N = rx1.cap(2).toInt();
        return (true);
    }

    // validate format
    QRegExp rx2("^[A-Z]{2}\\s*[0-9]+\\s*[0-9]+$");
    rx2.setCaseSensitivity(Qt::CaseInsensitive);
    if (!refStr.contains(rx2))
    {
        qDebug() << "invalid grid reference" << refStr;
        return (false);
    }

    // get numeric values of letter references, mapping A->0, B->1, C->2, etc
    int l1 = refStr.at(0).toUpper().toLatin1()-'A';
    int l2 = refStr.at(1).toUpper().toLatin1()-'A';
    // shuffle down letters after 'I' since 'I' is not used in grid
    if (l1>7) --l1;
    if (l2>7) --l2;

    // convert grid letters into 100km-square indexes from false origin (grid square SV)
    int e100km = ((l1-2) % 5)*5 + (l2 % 5);
    int n100km = (19-(l1/5)*5) - (l2/5);
#ifdef DEBUG_OSGB
    qDebug() << "e100km" << e100km << "n100km" << n100km;
#endif
    // skip grid letters to get numeric (easting/northing) part of ref
    QStringList en = refStr.mid(2).trimmed().split(QRegExp("\\s+"));
    // if E/N not whitespace separated, split halfway
    if (en.count()==1)
    {
        const QString enf = en.first();
        en.clear();
        const int enl = enf.length();
        if ((enl % 2)!=0)
        {
            qDebug() << "not even length coordinates" << enf;
            return (false);
        }

        en.append(enf.left(enl/2));
        en.append(enf.right(enl/2));
    }
    if (en.length()!=2)
    {
        qDebug() << "too many fields in grid ref" << refStr;
        return (false);
    }

    // validation
    if (e100km<0 || e100km>6 || n100km<0 || n100km>12)
    {
        qDebug() << "grid ref" << refStr << "out of range";
        return (false);
    }

    // standardise to 10-digit refs (metres)
    gridRef->E = (e100km*100000)+QString(en[0]+"00000").left(5).toInt();
    gridRef->N = (n100km*100000)+QString(en[1]+"00000").left(5).toInt();
#ifdef DEBUG_OSGB
    qDebug() << "-> E" << gridRef->E << "N" << gridRef->N;
#endif
    return (true);
}


void OSGBCoordinateHandler::setOSGB(double lat, double lon) const
{
    OSGBRef ref = latLonToOSGrid(lat, lon);
    setOSGBToEN(ref);
    setOSGBToRef(ref);
}


void OSGBCoordinateHandler::setOSGBToEN(const OSGBRef &ref) const
{
    mEastEdit->setText(QString::number(ref.E));
    mNorthEdit->setText(QString::number(ref.N));
}


// from MM OsGridRef.prototype.toString()

void OSGBCoordinateHandler::setOSGBToRef(const OSGBRef &ref) const
{
    const int E = ref.E;
    const int N = ref.N;
#ifdef DEBUG_OSGB
    qDebug() << "E" << E << "N" << N;
#endif

    // get the 100km-grid indices
    const int e100k = qRound(floor(E/100000.0));
    const int n100k = qRound(floor(N/100000.0));

    if (e100k<0 || e100k>6 || n100k<0 || n100k>12)
    {
        mReferenceEdit->setText(i18nc("@item:intext", "(Invalid)"));
        return;
    }

    // translate those into numeric equivalents of the grid letters
    int l1 = (19-n100k) - (19-n100k)%5 + qRound(floor((e100k+10)/5.0));
    int l2 = ((19-n100k)*5)%25 + e100k%5;

    // compensate for skipped 'I' and build grid letter-pairs
    if (l1>7) ++l1;
    if (l2>7) ++l2;

    QString letterRef;
    letterRef.append(QLatin1Char(l1+'A'));
    letterRef.append(QLatin1Char(l2+'A'));
    mLetterCombo->setCurrentText(letterRef);

    // strip 100km-grid indices from easting & northing, and reduce precision
    double unwanted;
    int ee = qRound(modf(E/100000.0, &unwanted)*100000/pow(10, 5-OSGB_DIGITS));
    QString ees = QString("%1").arg(ee, OSGB_DIGITS, 10, QLatin1Char('0'));
    int nn = qRound(modf(N/100000.0, &unwanted)*100000/pow(10, 5-OSGB_DIGITS));
    QString nns = QString("%1").arg(nn, OSGB_DIGITS, 10, QLatin1Char('0'));
#ifdef DEBUG_OSGB
    qDebug() << "->" << letterRef << ees << nns;
#endif
    mReferenceEdit->setText(ees+nns);
}


OSGBCoordinateHandler::OSGBCoordinateHandler(QObject *pnt)
    : AbstractCoordinateHandler(pnt)
{
    qDebug();
    setObjectName("OSGBCoordinateHandler");
}


QWidget *OSGBCoordinateHandler::createWidget(QWidget *pnt)
{
    QWidget *w = new QWidget(pnt);
    QGridLayout *gl = new QGridLayout(w);

    Qt::AlignmentFlag labelAlign = static_cast<Qt::AlignmentFlag>(w->style()->styleHint(QStyle::SH_FormLayoutLabelAlignment));

    // columns: 0     1     2     3
    //          label space combo ref
    //          label space value

    // row 0: grid reference
    QLabel *l = new QLabel(i18n("Reference:"), w);
    gl->addWidget(l, 0, 0, labelAlign);

    mLetterCombo = new QComboBox(w);
    const char *p = validSquares;
    while (*p!='\0')
    {
        QString s = QString::fromLocal8Bit(p, 2);
        mLetterCombo->addItem(s, s);
        p += 3;
    }

    connect(mLetterCombo, QOverload<const QString &>::of(&QComboBox::currentIndexChanged),
            this, &OSGBCoordinateHandler::slotReferenceChanged);
    gl->addWidget(mLetterCombo, 0, 2);
    l->setBuddy(mLetterCombo);

    mReferenceEdit = new QLineEdit(w);
    // validator which accepts 2, 4, 6, 8 or 10 digits
    QRegExpValidator *rv = new QRegExpValidator(QRegExp("(\\d\\d){1,5}"), w);
    mReferenceEdit->setValidator(rv);
    connect(mReferenceEdit, &QLineEdit::textEdited, this, &OSGBCoordinateHandler::slotReferenceChanged);
    gl->addWidget(mReferenceEdit, 0, 3);

    // row 1: easting
    l = new QLabel(i18n("Easting:"), w);
    gl->addWidget(l, 1, 0, labelAlign);

    mEastEdit = new QLineEdit(w);
    QIntValidator *iv = new QIntValidator(mEastEdit);
    iv->setRange(MIN_OSGB_EASTING, MAX_OSGB_EASTING);
    mEastEdit->setValidator(iv);
    connect(mEastEdit, &QLineEdit::textEdited, this, &OSGBCoordinateHandler::slotCoordinateChanged);
    gl->addWidget(mEastEdit, 1, 2, 1, -1);
    l->setBuddy(mEastEdit);

    // row 2: northing
    l = new QLabel(i18n("Northing:"), w);
    gl->addWidget(l, 2, 0, labelAlign);

    mNorthEdit = new QLineEdit(w);
    iv = new QIntValidator(mNorthEdit);
    iv->setRange(MIN_OSGB_NORTHING, MAX_OSGB_NORTHING);
    mNorthEdit->setValidator(iv);
    connect(mNorthEdit, &QLineEdit::textEdited, this, &OSGBCoordinateHandler::slotCoordinateChanged);
    gl->addWidget(mNorthEdit, 2, 2, 1, -1);
    l->setBuddy(mNorthEdit);

    // layout adjustment
    gl->setColumnMinimumWidth(1, DialogBase::horizontalSpacing());
    gl->setColumnStretch(3, 1);

    return (w);
}


void OSGBCoordinateHandler::updateGUI(double lat, double lon)
{
    qDebug() << lat << lon;

    if (!isnan(lat) && !isnan(lon))
    {
        setOSGB(lat, lon);
    }
    else
    {
        mReferenceEdit->clear();
        mNorthEdit->clear();
        mEastEdit->clear();
    }
}


void OSGBCoordinateHandler::checkError()
{
    AbstractCoordinateHandler::checkError();

    const int e = mEastEdit->text().toInt();
    const int n = mNorthEdit->text().toInt();

    if (e<MIN_OSGB_EASTING || e>MAX_OSGB_EASTING ||
        n<MIN_OSGB_NORTHING || n>MAX_OSGB_NORTHING)
    {
        setError(i18n("Coordinates out of National Grid range"));
    }
}


bool OSGBCoordinateHandler::hasAcceptableInput() const
{
    // We can't verify the OSGB entries here, as they may be
    // out of range outside that country.  Relying on the other
    // coordinate formats.
    return (true);
}


//  User has changed the grid square (combo) or reference (digits).
//  Update lat/lon values and the eastings/northings.

void OSGBCoordinateHandler::slotReferenceChanged()
{
    if (!mReferenceEdit->hasAcceptableInput())
    {
        // TODO: signal a syntax error displayed by parent widget
        return;
    }

    const QString letterRef = mLetterCombo->currentText();
    const QString squareRef = mReferenceEdit->text().trimmed();

    OSGBRef gridRef;
    bool valid = getOSGBFromRef((letterRef+" "+squareRef), &gridRef);
    if (!valid) return;

    setOSGBToEN(gridRef);
    LatLon newLatLon = getOSGBFromEN(gridRef);
    updateValues(newLatLon.lat, newLatLon.lon);
    checkError();
}


//  User has changed the full eastings/northings.
//  Update lat/lon values and the grid reference.

void OSGBCoordinateHandler::slotCoordinateChanged()
{
    OSGBRef gridRef;
    gridRef.E = mEastEdit->text().toInt();
    gridRef.N = mNorthEdit->text().toInt();

    setOSGBToRef(gridRef);
    LatLon newLatLon = getOSGBFromEN(gridRef);
    updateValues(newLatLon.lat, newLatLon.lon);
    checkError();
}


QString OSGBCoordinateHandler::tabName() const
{
    return (i18nc("@title:tab", "OSGB"));
}
