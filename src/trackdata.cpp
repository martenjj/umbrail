

#include "trackdata.h"

#include <qhash.h>
#include <qregexp.h>

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
//  Internal constants							//
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
static int counterTrackpoint = 0;
static int counterFolder = 0;
static int counterWaypoint = 0;

#ifdef MEMORY_TRACKING
static int allocFile = 0;
static int allocTrack = 0;
static int allocSegment = 0;
static int allocTrackpoint = 0;
static int allocFolder = 0;
static int allocWaypoint = 0;
static int allocStyle = 0;
static int allocChildren = 0;
static int allocMetadata = 0;
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

TimeRange TrackData::unifyTimeSpans(const QList<TrackDataItem *> *items)
{
    if (items==NULL) return (TimeRange::null);
    int num = items->count();
    if (num==0) return (TimeRange::null);

    const TrackDataItem *first = items->first();
    TimeRange result = first->timeSpan();
    for (int i = 1; i<num; ++i)
    {
        TimeRange itsSpan = items->at(i)->timeSpan();
        result = result.united(itsSpan);
    }

    return (result);
}



BoundingArea TrackData::unifyBoundingAreas(const QList<TrackDataItem *> *items)
{
    if (items==NULL) return (BoundingArea::null);
    int num = items->count();
    if (num==0) return (BoundingArea::null);

    const TrackDataItem *first = items->first();
    BoundingArea result = first->boundingArea();
    for (int i = 1; i<num; ++i)
    {
        BoundingArea itsArea = items->at(i)->boundingArea();
        result = result.united(itsArea);
    }

    return (result);
}



double TrackData::sumTotalTravelDistance(const QList<TrackDataItem *> *items, bool tracksOnly)
{
    if (items==NULL) return (0.0);
    int num = items->count();
    if (num==0) return (0.0);

    double dist = 0.0;					// running total
    const TrackDataAbstractPoint *prev = (tracksOnly ? dynamic_cast<const TrackDataTrackpoint *>(items->first()) :
                                                       dynamic_cast<const TrackDataAbstractPoint *>(items->first()));
    if (prev!=NULL)					// a list of (applicable) points
    {
        for (int i = 1; i<num; ++i)
        {
            const TrackDataAbstractPoint *next = dynamic_cast<const TrackDataAbstractPoint *>(items->at(i));
            Q_ASSERT(next!=NULL);			// next point in sequence

            dist += prev->distanceTo(next);		// from previous point to this
            prev = next;				// then move on current point
        }
    }
    else						// not points, must be containers
    {
        for (int i = 0; i<num; ++i)			// sum over all of them
        {
            dist += items->at(i)->totalTravelDistance();
        }
    }

    return (dist);
}




unsigned TrackData::sumTotalTravelTime(const QList<TrackDataItem *> *items)
{
    if (items==NULL) return (0);
    int num = items->count();
    if (num==0) return (0);

    unsigned int tot = 0;				// running total
    const TrackDataAbstractPoint *first = dynamic_cast<const TrackDataAbstractPoint *>(items->first());
    if (first!=NULL)					// a list of points
    {
        const TrackDataAbstractPoint *last = dynamic_cast<const TrackDataAbstractPoint *>(items->last());
        Q_ASSERT(last!=NULL);				// last point in sequence
        tot = first->timeTo(last);
    }
    else						// not points, must be containers
    {
        for (int i = 0; i<num; ++i)			// sum over all of them
        {
            tot += items->at(i)->totalTravelTime();
        }
    }

    return (tot);
}



unsigned TrackData::sumTotalChildCount(const QList<TrackDataItem *> *items)
{
    if (items==NULL) return (0);
    int num = 0;
    for (int i = 0; i<items->count(); ++i) num += items->at(i)->childCount();
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

    QDateTime tzdt = tz->toZoneTime(dt.toUTC());
    //kDebug() << dt << "->" << tzdt << tz->abbreviation(dt);
    return (KGlobal::locale()->formatDateTime(tzdt, KLocale::ShortDate, true)+" "+tz->abbreviation(dt).constData());
}


TrackDataFolder *TrackData::findFolderByPath(const QString &path, const TrackDataItem *root)
{
    const QStringList folders = path.split('/');

    TrackDataFolder *foundFolder = NULL;
    foreach (const QString &folder, folders)
    {
        foundFolder = (foundFolder!=NULL ? foundFolder : root)->findChildFolder(folder);
        if (foundFolder==NULL) break;
    }

    return (foundFolder);
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
    mChildren = NULL;					// no children yet
    mParent = NULL;					// not attached to parent
    mStyle = NULL;					// no style set yet
    mMetadata = NULL;					// no metadata yet
    mSelectionId = 1;					// nothing selected yet
}


TrackDataItem::~TrackDataItem()
{
    if (mChildren!=NULL) qDeleteAll(*mChildren);
    delete mChildren;
    delete mStyle;
    delete mMetadata;
}


void TrackDataItem::addChildItem(TrackDataItem *data, int idx)
{
    if (data->parent()!=NULL) kWarning() << "item" << data->name() << "already has parent" << data->parent()->name();
    Q_ASSERT(data->parent()==NULL);

    if (mChildren==NULL)
    {
#ifdef MEMORY_TRACKING
        ++allocChildren;
#endif
        mChildren = new QList<TrackDataItem *>;
    }

    if (idx>=0) mChildren->insert(idx, data);		// insert at specified place
    else mChildren->append(data);			// default is to append
							// have taken ownership of child
    data->mParent = this;				// set child item parent
}


TrackDataItem *TrackDataItem::takeLastChildItem()
{
    Q_ASSERT(mChildren!=NULL);
    Q_ASSERT(!mChildren->isEmpty());
    TrackDataItem *data = mChildren->takeLast();
    data->mParent = NULL;				// now no longer has parent
    return (data);
}


TrackDataItem *TrackDataItem::takeFirstChildItem()
{
    Q_ASSERT(mChildren!=NULL);
    Q_ASSERT(!mChildren->isEmpty());
    TrackDataItem *data = mChildren->takeFirst();
    data->mParent = NULL;				// now no longer has parent
    return (data);
}


TrackDataItem *TrackDataItem::takeChildItem(int idx)
{
    Q_ASSERT(mChildren!=NULL);
    Q_ASSERT(idx>=0 && idx<mChildren->count());
    TrackDataItem *data = mChildren->takeAt(idx);
    data->mParent = NULL;				// now no longer has parent
    return (data);
}


void TrackDataItem::removeChildItem(TrackDataItem *item)
{
    Q_ASSERT(mChildren!=NULL);
    takeChildItem(mChildren->indexOf(item));
}


BoundingArea TrackDataItem::boundingArea() const
{
    return (TrackData::unifyBoundingAreas(mChildren));
}


TimeRange TrackDataItem::timeSpan() const
{
    return (TrackData::unifyTimeSpans(mChildren));
}


double TrackDataItem::totalTravelDistance() const
{
    return (TrackData::sumTotalTravelDistance(mChildren));
}


unsigned int TrackDataItem::totalTravelTime() const
{
    return (TrackData::sumTotalTravelTime(mChildren));
}


void TrackDataItem::setMetadata(int idx, const QString &value)
{
    if (mMetadata==NULL)
    {
#ifdef MEMORY_TRACKING
        ++allocMetadata;
#endif
        mMetadata = new QVector<QString>;
    }

    int cnt = mMetadata->count();
    if (idx>=cnt) mMetadata->resize(idx+1);
    (*mMetadata)[idx] = value;
}


QString TrackDataItem::metadata(int idx) const
{
    if (mMetadata==NULL) return (QString::null);
    int cnt = mMetadata->count();
    if (idx<0 || idx>=cnt) return (QString::null);
    return (mMetadata->at(idx));
}


QString TrackDataItem::metadata(const QString &key) const
{
    if (mMetadata==NULL) return (QString::null);
    return (metadata(DataIndexer::self()->index(key)));
}


void TrackDataItem::copyMetadata(const TrackDataItem *other, bool overwrite)
{
    if (other->mMetadata==NULL) return;			// nothing to copy

    for (int idx = 0; idx<other->mMetadata->size(); ++idx)
    {
        QString om = other->mMetadata->at(idx);
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


TrackDataFolder *TrackDataItem::findChildFolder(const QString &wantName) const
{
    if (mChildren==NULL) return (NULL);			// no children exist

    //kDebug() << "name" << wantName << "under" << name();
    for (int i = 0; i<mChildren->count(); ++i)
    {
        TrackDataFolder *fold = dynamic_cast<TrackDataFolder *>(mChildren->at(i));
        if (fold!=NULL)					// this child is a folder,
        {						// check matching name
            if (fold->name()==wantName) return (fold);
        }
    }

    return (NULL);
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

    const TrackDataTrackpoint *firstPoint = dynamic_cast<const TrackDataTrackpoint *>(childAt(0));
    Q_ASSERT(firstPoint!=NULL);
    if (num==1) return (TimeRange(firstPoint->time(), firstPoint->time()));

    const TrackDataTrackpoint *lastPoint = dynamic_cast<const TrackDataTrackpoint *>(childAt(num-1));
    Q_ASSERT(lastPoint!=NULL);
    return (TimeRange(firstPoint->time(), lastPoint->time()));
}

//////////////////////////////////////////////////////////////////////////
//									//
//  TrackDataAbstractPoint						//
//									//
//////////////////////////////////////////////////////////////////////////

TrackDataAbstractPoint::TrackDataAbstractPoint(const QString &nm, const char *format, int *counter)
    : TrackDataItem(nm, format, counter)
{
    mLatitude = mLongitude = NAN;
    mElevation = NAN;
}


QString TrackDataAbstractPoint::formattedElevation() const
{
    if (isnan(mElevation)) return (i18nc("an unknown quantity", "unknown"));
    return (i18nc("@item:intable Number with unit of metres", "%1 m", QString::number(mElevation, 'f', 1)));
}


QString TrackDataAbstractPoint::formattedTime(bool withZone) const
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


QString TrackDataAbstractPoint::formattedPosition() const
{
    return (TrackData::formattedLatLong(mLatitude, mLongitude));
}


BoundingArea TrackDataAbstractPoint::boundingArea() const
{
    return (BoundingArea(mLatitude, mLongitude));
}


TimeRange TrackDataAbstractPoint::timeSpan() const
{
    return (TimeRange(time(), time()));
}


double TrackDataAbstractPoint::distanceTo(double lat, double lon, bool accurate) const
{
    // See http://www.movable-type.co.uk/scripts/latlong.html

    const double lat1 = latitude()*DEGREES_TO_RADIANS;
    const double lon1 = longitude()*DEGREES_TO_RADIANS;
    const double lat2 = lat*DEGREES_TO_RADIANS;
    const double lon2 = lon*DEGREES_TO_RADIANS;

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


double TrackDataAbstractPoint::distanceTo(const TrackDataAbstractPoint *other, bool accurate) const
{
    return (distanceTo(other->latitude(), other->longitude(), accurate));
}


double TrackDataAbstractPoint::bearingTo(const TrackDataAbstractPoint *other) const
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


int TrackDataAbstractPoint::timeTo(const TrackDataAbstractPoint *other) const
{
    return (time().secsTo(other->time()));
}


void TrackDataAbstractPoint::copyData(const TrackDataAbstractPoint *other)
{
    mLatitude = other->mLatitude;
    mLongitude = other->mLongitude;
    mElevation = other->mElevation;
    mDateTime = other->mDateTime;
}

//////////////////////////////////////////////////////////////////////////
//									//
//  TrackDataFolder							//
//									//
//////////////////////////////////////////////////////////////////////////

TrackDataFolder::TrackDataFolder(const QString &nm)
    : TrackDataItem(nm, "folder_%02d", &counterFolder)
{
#ifdef MEMORY_TRACKING
    ++allocFolder;
#endif
}


QString TrackDataFolder::path() const
{
    QStringList p(name());
    const TrackDataItem *pnt = parent();

    while (pnt!=NULL && dynamic_cast<const TrackDataFile *>(pnt)==NULL)
    {
        p.prepend(pnt->name());
        pnt = pnt->parent();
    }

    return (p.join("/"));
}

//////////////////////////////////////////////////////////////////////////
//									//
//  TrackDataTrackpoint							//
//									//
//////////////////////////////////////////////////////////////////////////

TrackDataTrackpoint::TrackDataTrackpoint(const QString &nm)
    : TrackDataAbstractPoint(nm, "point_%04d", &counterTrackpoint)
{
#ifdef MEMORY_TRACKING
    ++allocTrackpoint;
#endif
}

//////////////////////////////////////////////////////////////////////////
//									//
//  TrackDataWaypoint							//
//									//
//////////////////////////////////////////////////////////////////////////

TrackDataWaypoint::TrackDataWaypoint(const QString &nm)
    : TrackDataAbstractPoint(nm, "wpt_%03d", &counterWaypoint)
{
#ifdef MEMORY_TRACKING
    ++allocWaypoint;
#endif
}


TrackData::WaypointType TrackDataWaypoint::waypointType() const
{
    QString n = metadata("stop");			// first try saved stop data
    if (!n.isEmpty()) return (TrackData::WaypointStop);	// this means it's a stop

    n = metadata("link");				// then get saved link name
    if (n.isEmpty()) n = metadata("media");		// compatibility with old metadata
    if (n.isEmpty()) n = name();			// lastly try our waypoint name

    if (n.contains(QRegExp("\\.3gp$"))) return (TrackData::WaypointAudioNote);
    if (n.contains(QRegExp("\\.mp4$"))) return (TrackData::WaypointVideoNote);
    if (n.contains(QRegExp("\\.jpg$"))) return (TrackData::WaypointPhoto);
    return (TrackData::WaypointNormal);
}


QString TrackDataWaypoint::iconName() const
{
    switch (waypointType())
    {
case TrackData::WaypointNormal:		return ("favorites");
case TrackData::WaypointAudioNote:	return ("speaker");
case TrackData::WaypointVideoNote:	return ("mixer-video");
case TrackData::WaypointPhoto:		return ("image-x-generic");
case TrackData::WaypointStop:		return ("media-playback-stop");
default:				return ("unknown");
    }
}


bool TrackDataWaypoint::isMediaType() const
{
    const TrackData::WaypointType wpt = waypointType();
    return (wpt==TrackData::WaypointAudioNote ||
            wpt==TrackData::WaypointVideoNote ||
            wpt==TrackData::WaypointPhoto);
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
    qDebug() << "point" << sizeof(TrackDataAbstractPoint) << "bytes";
    qDebug() << "folder" << sizeof(TrackDataFolder) << "bytes";
    qDebug() << "trackpoint" << sizeof(TrackDataTrackpoint) << "bytes";
    qDebug() << "waypoint" << sizeof(TrackDataWaypoint) << "bytes";
    qDebug() << "style" << sizeof(Style) << "bytes";
    qDebug() << "***********";
}


MemoryTracker::~MemoryTracker()
{
    qDebug() << "*********** Memory statistics:";
    qDebug() << "file allocated" << allocFile << "items, total" << allocFile*sizeof(TrackDataFile) << "bytes";
    qDebug() << "track allocated" << allocTrack << "items, total" << allocTrack*sizeof(TrackDataTrack) << "bytes";
    qDebug() << "segment allocated" << allocSegment << "items, total" << allocSegment*sizeof(TrackDataSegment) << "bytes";
    qDebug() << "folder allocated" << allocFolder << "items, total" << allocFolder*sizeof(TrackDataFolder) << "bytes";
    qDebug() << "trackpoint allocated" << allocTrackpoint << "items, total" << allocTrackpoint*sizeof(TrackDataTrackpoint) << "bytes";
    qDebug() << "waypoint allocated" << allocWaypoint << "items, total" << allocWaypoint*sizeof(TrackDataWaypoint) << "bytes";
    qDebug() << "style allocated" << allocStyle << "items, total" << allocStyle*sizeof(Style) << "bytes";
    qDebug() << "child list allocated" << allocChildren;
    qDebug() << "metadata allocated" << allocMetadata;
    qDebug() << "***********";
}

#endif
