

#include "trackdata.h"

#include <qhash.h>

#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <kmimetype.h>
#include <ktimezone.h>
#include <ksystemtimezone.h>

#include "style.h"
#include "dataindexer.h"

#undef MEMORY_TRACKING

//////////////////////////////////////////////////////////////////////////
//									//
//  Insternal constants							//
//									//
//////////////////////////////////////////////////////////////////////////

static const double DEGREES_TO_RADIANS = (2*M_PI)/360;	// multiplier

const TimeRange TimeRange::null = TimeRange();
const BoundingArea BoundingArea::null = BoundingArea();

//////////////////////////////////////////////////////////////////////////
//									//
//  Internal static							//
//									//
//////////////////////////////////////////////////////////////////////////

static int counterFile = 0;
static int counterTrack = 0;
static int counterSegment = 0;
static int counterPoint = 0;

#ifdef MEMORY_TRACKING
static int allocFile = 0;
static int allocTrack = 0;
static int allocSegment = 0;
static int allocPoint = 0;
static int allocStyle = 0;
#endif

//////////////////////////////////////////////////////////////////////////
//									//
//  TimeRange								//
//									//
//////////////////////////////////////////////////////////////////////////

TimeRange TimeRange::united(const TimeRange &other) const
{
    if (!isValid()) return (other);

    TimeRange result = *this;
    if (!other.isValid()) return (result);

    if (other.mStart<mStart) result.mStart = other.mStart;
    if (other.mFinish>mFinish) result.mFinish = other.mFinish;
    return (result);
}

//////////////////////////////////////////////////////////////////////////
//									//
//  BoundingArea							//
//									//
//////////////////////////////////////////////////////////////////////////

BoundingArea BoundingArea::united(const BoundingArea &other) const
{
    if (!isValid()) return (other);

    BoundingArea result = *this;
    if (!other.isValid()) return (result);

    if (other.mLatNorth>mLatNorth) result.mLatNorth = other.mLatNorth;
    if (other.mLatSouth<mLatSouth) result.mLatSouth = other.mLatSouth;
    if (other.mLonEast>mLonEast) result.mLonEast = other.mLonEast;
    if (other.mLonWest<mLonWest) result.mLonWest = other.mLonWest;
    return (result);
}

//////////////////////////////////////////////////////////////////////////
//									//
//  TrackData								//
//									//
//////////////////////////////////////////////////////////////////////////

TimeRange TrackData::unifyTimeSpans(const QList<TrackDataItem *> &items)
{
    int num = items.count();
    if (num==0) return (TimeRange::null);

    const TrackDataItem *first = items.first();
    TimeRange result = first->timeSpan();
    for (int i = 1; i<num; ++i)
    {
        TimeRange itsSpan = items[i]->timeSpan();
        result = result.united(itsSpan);
    }

    return (result);
}


BoundingArea TrackData::unifyBoundingAreas(const QList<TrackDataItem *> &items)
{
    int num = items.count();
    if (num==0) return (BoundingArea::null);

    const TrackDataItem *first = items.first();
    BoundingArea result = first->boundingArea();
    for (int i = 1; i<num; ++i)
    {
        BoundingArea itsArea = items[i]->boundingArea();
        result = result.united(itsArea);
    }

    return (result);
}


double TrackData::sumTotalTravelDistance(const QList<TrackDataItem *> &items)
{
    int num = items.count();
    if (num==0) return (0.0);

    double dist = 0.0;					// running total
    const TrackDataPoint *prev = dynamic_cast<const TrackDataPoint *>(items.first());
    if (prev!=NULL)					// a list of points
    {
        for (int i = 1; i<num; ++i)
        {
            const TrackDataPoint *next = dynamic_cast<const TrackDataPoint *>(items[i]);
            Q_ASSERT(next!=NULL);			// next point in sequence

            dist += prev->distanceTo(next);		// from previous point to this
            prev = next;				// then move on current point
        }
    }
    else						// not points, must be containers
    {
        for (int i = 0; i<num; ++i)			// sum over all of them
        {
            dist += items[i]->totalTravelDistance();
        }
    }

    return (dist);
}


unsigned TrackData::sumTotalTravelTime(const QList<TrackDataItem *> &items)
{
    int num = items.count();
    if (num==0) return (0);

    unsigned int tot = 0;				// running total
    const TrackDataPoint *first = dynamic_cast<const TrackDataPoint *>(items.first());
    if (first!=NULL)					// a list of points
    {
        const TrackDataPoint *last = dynamic_cast<const TrackDataPoint *>(items.last());
        Q_ASSERT(last!=NULL);				// last point in sequence
        tot = first->timeTo(last);
    }
    else						// not points, must be containers
    {
        for (int i = 0; i<num; ++i)			// sum over all of them
        {
            tot += items[i]->totalTravelTime();
        }
    }

    return (tot);
}


unsigned TrackData::sumTotalChildCount(const QList<TrackDataItem *> &items)
{
    int num = 0;
    for (int i = 0; i<items.count(); ++i) num += items[i]->childCount();
    return (num);
}


static QString toDMS(double d, int degWidth, char posMark, char negMark)
{
    char mark = (d==0.0 ? ' ' : (d<0.0 ? negMark : posMark));
    d = fabs(d);

    int deg = static_cast<int>(d);
    d = (d-deg)*60.0;
    int min = static_cast<int>(d);
    d = (d-min)*60.0;

    return (QString("%1%2%3'%4\"%5")
            .arg(deg, degWidth)
            .arg(QLatin1Char(0xB0))
            .arg(min, 2, 10, QLatin1Char('0'))
            .arg(d, 4, 'f', 1, QLatin1Char('0'))
            .arg(mark));
}


QString TrackData::formattedLatLong(double lat, double lon, bool blankIfUnknown)
{
    if (isnan(lat) || isnan(lon))
    {
        if (blankIfUnknown) return (QString::null);
        return (i18nc("an unknown quantity", "unknown"));
    }

    QString latStr = toDMS(lat, 2, 'N', 'S');
    QString lonStr = toDMS(lon, 3, 'E', 'W');
    return (latStr+" "+lonStr);
}


QString TrackData::formattedDuration(unsigned t, bool blankIfZero)
{
    if (t==0 && blankIfZero) return (QString::null);

    int sec = t % 60;					// seconds
    t -= sec;
    t /= 60;

    int min = t % 60;					// minutes
    t -= min;
    t /= 60;
 							// the rest is hours
    return (QString("%1:%2:%3")
            .arg(t, 2, 10, QLatin1Char('0'))
            .arg(min, 2, 10, QLatin1Char('0'))
            .arg(sec, 2, 10, QLatin1Char('0')));
}


QString TrackData::formattedTime(const QDateTime &dt, const KTimeZone *tz)
{
    if (!dt.isValid()) return (i18nc("an unknown quantity", "unknown"));
    if (tz==NULL) return (KGlobal::locale()->formatDateTime(dt, KLocale::ShortDate, true));

    QDateTime tzdt = tz->toZoneTime(dt);
    //kDebug() << dt << "->" << tzdt << tz->abbreviation(dt);
    return (KGlobal::locale()->formatDateTime(tzdt, KLocale::ShortDate, true)+" "+tz->abbreviation(dt).constData());
}

//////////////////////////////////////////////////////////////////////////
//									//
//  TrackDataItem							//
//									//
//////////////////////////////////////////////////////////////////////////

TrackDataItem::TrackDataItem(const QString &nm, const char *format, int *counter)
{
    mName = nm;
    if (nm.isEmpty() && format!=NULL) mName.sprintf(format, ++(*counter));
    init();
}


void TrackDataItem::init()
{
    mParent = NULL;					// not attached to parent
    mRefCount = 0;					// no references to this
    mStyle = NULL;					// no style set yet
    mSelectionId = 1;					// nothing selected yet
}


TrackDataItem::~TrackDataItem()
{
    qDeleteAll(mChildItems);
    delete mStyle;
}


void TrackDataItem::addChildItem(TrackDataItem *data, int idx)
{
    if (data->parent()!=NULL) kWarning() << "item" << data->name() << "already has parent" << data->parent()->name();

    if (idx>=0) mChildItems.insert(idx, data);		// insert at specified place
    else mChildItems.append(data);			// default is to append
							// have taken ownership of child
    data->mParent = this;				// set child item parent
}


TrackDataItem *TrackDataItem::takeLastChildItem()
{
    Q_ASSERT(!mChildItems.isEmpty());
    TrackDataItem *data = mChildItems.takeLast();
    data->mParent = NULL;				// now no longer has parent
    return (data);
}


TrackDataItem *TrackDataItem::takeFirstChildItem()
{
    Q_ASSERT(!mChildItems.isEmpty());
    TrackDataItem *data = mChildItems.takeFirst();
    data->mParent = NULL;				// now no longer has parent
    return (data);
}


TrackDataItem *TrackDataItem::takeChildItem(int idx)
{
    Q_ASSERT(idx>=0 && idx<mChildItems.count());
    TrackDataItem *data = mChildItems.takeAt(idx);
    data->mParent = NULL;				// now no longer has parent
    return (data);

}


void TrackDataItem::takeChildItem(TrackDataItem *item)
{
    takeChildItem(mChildItems.indexOf(item));
}


BoundingArea TrackDataItem::boundingArea() const
{
    return (TrackData::unifyBoundingAreas(mChildItems));
}


TimeRange TrackDataItem::timeSpan() const
{
    return (TrackData::unifyTimeSpans(mChildItems));
}


double TrackDataItem::totalTravelDistance() const
{
    return (TrackData::sumTotalTravelDistance(mChildItems));
}


unsigned int TrackDataItem::totalTravelTime() const
{
    return (TrackData::sumTotalTravelTime(mChildItems));
}


void TrackDataItem::setMetadata(int idx, const QString &value)
{
    int cnt = mMetadata.count();
    if (idx>=cnt) mMetadata.resize(idx+1);
    mMetadata[idx] = value;
}


QString TrackDataItem::metadata(int idx) const
{
    int cnt = mMetadata.count();
    if (idx<0 || idx>=cnt) return (QString::null);
    return (mMetadata.at(idx));
}


QString TrackDataItem::metadata(const QString &key) const
{
    return (metadata(DataIndexer::self()->index(key)));
}


void TrackDataItem::copyMetadata(const TrackDataItem *other, bool overwrite)
{
    for (int idx = 0; idx<other->mMetadata.size(); ++idx)
    {
        QString om = other->mMetadata.at(idx);
        if (om.isEmpty()) continue;
        QString tm = this->metadata(idx);
        if (!tm.isEmpty() && !overwrite) continue;
        this->setMetadata(idx, om);
    }
}


const Style *TrackDataItem::style() const
{
    return ((mStyle!=NULL) ? mStyle : &Style::null);
}


void TrackDataItem::setStyle(const Style &s)
{
    if (mStyle==NULL)
    {
        kDebug() << "set style for" << name();
        mStyle = new Style(s);
#ifdef MEMORY_TRACKING
        ++allocStyle;
#endif
    }
    else *mStyle = s;
}


QString TrackDataItem::timeZone() const
{
    const TrackDataItem *parentItem = this;
    while (parentItem!=NULL)
    {
        const TrackDataFile *parentFile = dynamic_cast<const TrackDataFile *>(parentItem);
        if (parentFile!=NULL) return (parentFile->metadata("timezone"));
        parentItem = parentItem->parent();
    }
    return (QString::null);
}

//////////////////////////////////////////////////////////////////////////
//									//
//  TrackDataFile							//
//									//
//////////////////////////////////////////////////////////////////////////

TrackDataFile::TrackDataFile(const QString &nm)
    : TrackDataItem(nm, "file_%02d", &counterFile)
{
#ifdef MEMORY_TRACKING
    ++allocFile;
#endif
}


QString TrackDataFile::iconName() const
{
    return (KMimeType::iconNameForUrl(mFileName));
}

//////////////////////////////////////////////////////////////////////////
//									//
//  TrackDataTrack							//
//									//
//////////////////////////////////////////////////////////////////////////

TrackDataTrack::TrackDataTrack(const QString &nm)
    : TrackDataItem(nm, "track_%02d", &counterTrack)
{
#ifdef MEMORY_TRACKING
    ++allocTrack;
#endif
}

//////////////////////////////////////////////////////////////////////////
//									//
//  TrackDataSegment							//
//									//
//////////////////////////////////////////////////////////////////////////

TrackDataSegment::TrackDataSegment(const QString &nm)
    : TrackDataItem(nm, "segment_%02d", &counterSegment)
{
#ifdef MEMORY_TRACKING
    ++allocSegment;
#endif
}


// Optimisation, assumes that points are in chronological order
TimeRange TrackDataSegment::timeSpan() const
{
    int num = childCount();
    if (num==0) return (TimeRange::null);

    const TrackDataPoint *firstPoint = dynamic_cast<const TrackDataPoint *>(childAt(0));
    Q_ASSERT(firstPoint!=NULL);
    if (num==1) return (TimeRange(firstPoint->time(), firstPoint->time()));

    const TrackDataPoint *lastPoint = dynamic_cast<const TrackDataPoint *>(childAt(num-1));
    Q_ASSERT(lastPoint!=NULL);
    return (TimeRange(firstPoint->time(), lastPoint->time()));
}

//////////////////////////////////////////////////////////////////////////
//									//
//  TrackDataPoint							//
//									//
//////////////////////////////////////////////////////////////////////////

TrackDataPoint::TrackDataPoint(const QString &nm)
    : TrackDataItem(nm, "point_%04d", &counterPoint)
{
    mLatitude = mLongitude = NAN;
    mElevation = NAN;
#ifdef MEMORY_TRACKING
    ++allocPoint;
#endif
}


QString TrackDataPoint::formattedElevation() const
{
    if (isnan(mElevation)) return (i18nc("an unknown quantity", "unknown"));
    return (i18nc("@item:intable Number with unit of metres", "%1 m", QString::number(mElevation, 'f', 1)));
}


QString TrackDataPoint::formattedTime(bool withZone) const
{
    if (withZone)
    {
        QString zoneName = timeZone();
        if (!zoneName.isEmpty())
        {
            const KTimeZone tz = KSystemTimeZones::zone(zoneName);
            return (TrackData::formattedTime(mDateTime, &tz));
        }
    }

    return (TrackData::formattedTime(mDateTime));
}


QString TrackDataPoint::formattedPosition() const
{
    return (TrackData::formattedLatLong(mLatitude, mLongitude));
}


BoundingArea TrackDataPoint::boundingArea() const
{
    return (BoundingArea(mLatitude, mLongitude));
}


TimeRange TrackDataPoint::timeSpan() const
{
    return (TimeRange(time(), time()));
}


double TrackDataPoint::distanceTo(const TrackDataPoint *other, bool accurate) const
{
    // See http://www.movable-type.co.uk/scripts/latlong.html

    const double lat1 = latitude()*DEGREES_TO_RADIANS;
    const double lon1 = longitude()*DEGREES_TO_RADIANS;
    const double lat2 = other->latitude()*DEGREES_TO_RADIANS;
    const double lon2 = other->longitude()*DEGREES_TO_RADIANS;

    if (accurate)
    {
        // Spherical cosines for maximum accuracy
        const double d = (acos(sin(lat1)*sin(lat2) + cos(lat1)*cos(lat2)*cos(lon2-lon1)));
        if (!isnan(d)) return (d);
    }

    // Pythagoras is good enough for small distances
    const double x = (lon2-lon1)*cos((lat1+lat2)/2.0);
    const double y = (lat2-lat1);
    return (sqrt(x*x + y*y));
}


double TrackDataPoint::bearingTo(const TrackDataPoint *other) const
{
    // Rhumb line distance/bearing as per reference above

    double lat1 = latitude()*DEGREES_TO_RADIANS;
    double lon1 = longitude()*DEGREES_TO_RADIANS;
    double lat2 = other->latitude()*DEGREES_TO_RADIANS;
    double lon2 = other->longitude()*DEGREES_TO_RADIANS;

    double dphi = log(tan(M_PI/4+lat2/2)/tan(M_PI/4+lat1/2));
    double dlon = lon2-lon1;
    if (abs(dlon)>M_PI) dlon = dlon>0 ? -(2*M_PI-dlon) : (2*M_PI+dlon);
    return (atan2(dlon, dphi)/DEGREES_TO_RADIANS);
}


int TrackDataPoint::timeTo(const TrackDataPoint *other) const
{
    return (time().secsTo(other->time()));
}


void TrackDataPoint::copyData(const TrackDataPoint *other)
{
    mLatitude = other->mLatitude;
    mLongitude = other->mLongitude;
    mElevation = other->mElevation;
    mDateTime = other->mDateTime;
}

//////////////////////////////////////////////////////////////////////////
//									//
//  Memory tracking							//
//									//
//////////////////////////////////////////////////////////////////////////

#ifdef MEMORY_TRACKING

struct MemoryTracker
{
    MemoryTracker();
    ~MemoryTracker();
};

static MemoryTracker sTracker;


MemoryTracker::MemoryTracker()
{
    qDebug() << "*********** Structure sizes:";
    qDebug() << "item" << sizeof(TrackDataItem) << "bytes";
    qDebug() << "file" << sizeof(TrackDataFile) << "bytes";
    qDebug() << "track" << sizeof(TrackDataTrack) << "bytes";
    qDebug() << "segment" << sizeof(TrackDataSegment) << "bytes";
    qDebug() << "point" << sizeof(TrackDataPoint) << "bytes";
    qDebug() << "style" << sizeof(Style) << "bytes";
    qDebug() << "***********";
}


MemoryTracker::~MemoryTracker()
{
    qDebug() << "*********** Memory statistics:";
    qDebug() << "file allocated" << allocFile << "items, total" << allocFile*sizeof(TrackDataFile) << "bytes";
    qDebug() << "track allocated" << allocTrack << "items, total" << allocTrack*sizeof(TrackDataTrack) << "bytes";
    qDebug() << "segment allocated" << allocSegment << "items, total" << allocSegment*sizeof(TrackDataSegment) << "bytes";
    qDebug() << "point allocated" << allocPoint << "items, total" << allocPoint*sizeof(TrackDataPoint) << "bytes";
    qDebug() << "style allocated" << allocStyle << "items, total" << allocStyle*sizeof(Style) << "bytes";
    qDebug() << "***********";
}

#endif
