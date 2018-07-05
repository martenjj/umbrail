

#include "trackdata.h"

#include <qregexp.h>
#include <qdebug.h>
#include <qtimezone.h>
#include <qicon.h>
#include <qstandardpaths.h>

#include <klocalizedstring.h>
#include <kiconloader.h>

#include <kio/global.h>

#include "style.h"
#include "dataindexer.h"
#include "waypointimageprovider.h"

//////////////////////////////////////////////////////////////////////////
//									//
//  Debugging switches							//
//									//
//////////////////////////////////////////////////////////////////////////

#undef MEMORY_TRACKING
#undef DEBUG_ICONS

//////////////////////////////////////////////////////////////////////////
//									//
//  Internal constants							//
//									//
//////////////////////////////////////////////////////////////////////////

const TimeRange TimeRange::null = TimeRange();
const BoundingArea BoundingArea::null = BoundingArea();

//////////////////////////////////////////////////////////////////////////
//									//
//  Internal static							//
//									//
//////////////////////////////////////////////////////////////////////////

static int counterFile = 0;
static int counterTrack = 0;
static int counterRoute = 0;
static int counterSegment = 0;
static int counterTrackpoint = 0;
static int counterFolder = 0;
static int counterWaypoint = 0;
static int counterRoutepoint = 0;

#ifdef MEMORY_TRACKING
static int allocFile = 0;
static int allocTrack = 0;
static int allocRoute = 0;
static int allocSegment = 0;
static int allocTrackpoint = 0;
static int allocFolder = 0;
static int allocWaypoint = 0;
static int allocRoutepoint = 0;
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
            .arg(QChar(0xB0))
            .arg(min, 2, 10, QLatin1Char('0'))
            .arg(d, 4, 'f', 1, QLatin1Char('0'))
            .arg(mark));
}


QString TrackData::formattedLatLong(double lat, double lon, bool blankIfUnknown)
{
    if (ISNAN(lat) || ISNAN(lon))
    {
        if (blankIfUnknown) return ("");
        return (i18nc("an unknown quantity", "unknown"));
    }

    QString latStr = toDMS(lat, 2, 'N', 'S');
    QString lonStr = toDMS(lon, 3, 'E', 'W');
    return (latStr+" "+lonStr);
}


QString TrackData::formattedDuration(unsigned t, bool blankIfZero)
{
    if (t==0 && blankIfZero) return ("");

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


QString TrackData::formattedTime(const QDateTime &dt, const QTimeZone *tz)
{
    if (!dt.isValid()) return (i18nc("an unknown quantity", "unknown"));
    if (tz==NULL) return (QLocale().toString(dt, QLocale::ShortFormat));

    QDateTime tzdt = dt.toUTC().toTimeZone(*tz);
    // QDateTime tzdt = tz->toZoneTime(dt.toUTC());
    //qDebug() << dt << "->" << tzdt << tz->abbreviation(dt);
    return (QLocale().toString(tzdt, QLocale::ShortFormat)+" "+tz->abbreviation(dt));
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

TrackDataItem::TrackDataItem(const char *format, int *counter)
{
    init();
    if (format!=nullptr) mName.sprintf(format, ++(*counter));
}


void TrackDataItem::init()
{
    mChildren = NULL;					// no children yet
    mParent = NULL;					// not attached to parent
    mStyle = NULL;					// no style set yet
    mMetadata = NULL;					// no metadata yet
    mSelectionId = 1;					// nothing selected yet
    mExplicitName = false;
}


TrackDataItem::~TrackDataItem()
{
    if (mChildren!=NULL) qDeleteAll(*mChildren);
    delete mChildren;
    delete mStyle;
    delete mMetadata;
}


void TrackDataItem::setName(const QString &newName, bool explicitName)
{
    mName = newName;
    mExplicitName = explicitName;
}


void TrackDataItem::addChildItem(TrackDataItem *data, int idx)
{
    if (data->parent()!=NULL) qWarning() << "item" << data->name() << "already has parent" << data->parent()->name();
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
    if (mMetadata==NULL) return (QString());
    int cnt = mMetadata->count();
    if (idx<0 || idx>=cnt) return (QString());
    return (mMetadata->at(idx));
}


QString TrackDataItem::metadata(const QString &key) const
{
    if (mMetadata==NULL) return (QString());
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
        //qDebug() << "set style for" << name();
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
    return (QString());
}


TrackDataFolder *TrackDataItem::findChildFolder(const QString &wantName) const
{
    if (mChildren==NULL) return (NULL);			// no children exist

    //qDebug() << "name" << wantName << "under" << name();
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


QIcon TrackDataItem::icon() const
{
    return (QIcon::fromTheme(this->iconName()));
}

//////////////////////////////////////////////////////////////////////////
//									//
//  TrackDataFile							//
//									//
//////////////////////////////////////////////////////////////////////////

TrackDataFile::TrackDataFile()
    : TrackDataItem("file_%02d", &counterFile)
{
#ifdef MEMORY_TRACKING
    ++allocFile;
#endif
}


void TrackDataFile::setFileName(const QUrl &file)
{
    mFileName = file;
    setName(file.fileName(), false);
}


QString TrackDataFile::iconName() const
{
    return (KIO::iconNameForUrl(mFileName));
}

//////////////////////////////////////////////////////////////////////////
//									//
//  TrackDataTrack							//
//									//
//////////////////////////////////////////////////////////////////////////

TrackDataTrack::TrackDataTrack()
    : TrackDataItem("track_%02d", &counterTrack)
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

TrackDataSegment::TrackDataSegment()
    : TrackDataItem("segment_%02d", &counterSegment)
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

TrackDataAbstractPoint::TrackDataAbstractPoint(const char *format, int *counter)
    : TrackDataItem(format, counter)
{
    mLatitude = mLongitude = NAN;
    mElevation = NAN;
}


QString TrackDataAbstractPoint::formattedElevation() const
{
    if (ISNAN(mElevation)) return (i18nc("an unknown quantity", "unknown"));
    return (i18nc("@item:intable Number with unit of metres", "%1 m", QString::number(mElevation, 'f', 1)));
}


QString TrackDataAbstractPoint::formattedTime(bool withZone) const
{
    if (withZone)
    {
        QString zoneName = timeZone();
        if (!zoneName.isEmpty())
        {
            QTimeZone tz(zoneName.toLatin1());
            if (tz.isValid()) return (TrackData::formattedTime(mDateTime, &tz));
            qWarning() << "unknown time zone" << zoneName;
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

    const double lat1 = DEGREES_TO_RADIANS(latitude());
    const double lon1 = DEGREES_TO_RADIANS(longitude());
    const double lat2 = DEGREES_TO_RADIANS(lat);
    const double lon2 = DEGREES_TO_RADIANS(lon);

    if (accurate)
    {
        // Spherical cosines for maximum accuracy
        const double d = (acos(sin(lat1)*sin(lat2) + cos(lat1)*cos(lat2)*cos(lon2-lon1)));
        if (!ISNAN(d)) return (d);
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

    double lat1 = DEGREES_TO_RADIANS(latitude());
    double lon1 = DEGREES_TO_RADIANS(longitude());
    double lat2 = DEGREES_TO_RADIANS(other->latitude());
    double lon2 = DEGREES_TO_RADIANS(other->longitude());

    double dphi = log(tan(M_PI/4+lat2/2)/tan(M_PI/4+lat1/2));
    double dlon = lon2-lon1;
    if (fabs(dlon)>M_PI) dlon = dlon>0 ? -(2*M_PI-dlon) : (2*M_PI+dlon);
    return (RADIANS_TO_DEGREES(atan2(dlon, dphi)));
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

TrackDataFolder::TrackDataFolder()
    : TrackDataItem("folder_%02d", &counterFolder)
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

TrackDataTrackpoint::TrackDataTrackpoint()
    : TrackDataAbstractPoint("point_%04d", &counterTrackpoint)
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

TrackDataWaypoint::TrackDataWaypoint()
    : TrackDataAbstractPoint("wpt_%03d", &counterWaypoint)
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


QIcon TrackDataWaypoint::icon() const
{
    if (waypointType()!=TrackData::WaypointNormal) return (TrackDataItem::icon());
    if (!style()->hasPointColour()) return (TrackDataItem::icon());

    // TODO: style inheritance
    QColor col = style()->pointColour();
#ifdef DEBUG_ICONS
    qDebug() << "need icon for waypoint" << name() << "col" << col.name();
#endif
    QIcon ic = WaypointImageProvider::self()->icon(col);

    if (ic.isNull()) return (TrackDataItem::icon());	// icon image not available
    return (ic);
}

//////////////////////////////////////////////////////////////////////////
//									//
//  TrackDataRoute							//
//									//
//////////////////////////////////////////////////////////////////////////

TrackDataRoute::TrackDataRoute()
    : TrackDataItem("route_%02d", &counterRoute)
{
#ifdef MEMORY_TRACKING
    ++allocRoute;
#endif
}

//////////////////////////////////////////////////////////////////////////
//									//
//  TrackDataRoutepoint							//
//									//
//////////////////////////////////////////////////////////////////////////

TrackDataRoutepoint::TrackDataRoutepoint()
    : TrackDataAbstractPoint("rpt_%04d", &counterRoutepoint)
{
#ifdef MEMORY_TRACKING
    ++allocRoutepoint;
#endif
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
    qDebug() << "route" << sizeof(TrackDataRoute) << "bytes";
    qDebug() << "segment" << sizeof(TrackDataSegment) << "bytes";
    qDebug() << "point" << sizeof(TrackDataAbstractPoint) << "bytes";
    qDebug() << "folder" << sizeof(TrackDataFolder) << "bytes";
    qDebug() << "trackpoint" << sizeof(TrackDataTrackpoint) << "bytes";
    qDebug() << "waypoint" << sizeof(TrackDataWaypoint) << "bytes";
    qDebug() << "routepoint" << sizeof(TrackDataRoutepoint) << "bytes";
    qDebug() << "style" << sizeof(Style) << "bytes";
    qDebug() << "***********";
}


MemoryTracker::~MemoryTracker()
{
    qDebug() << "*********** Memory statistics:";
    qDebug() << "file allocated" << allocFile << "items, total" << allocFile*sizeof(TrackDataFile) << "bytes";
    qDebug() << "track allocated" << allocTrack << "items, total" << allocTrack*sizeof(TrackDataTrack) << "bytes";
    qDebug() << "route allocated" << allocRoute << "items, total" << allocRoute*sizeof(TrackDataRoute) << "bytes";
    qDebug() << "segment allocated" << allocSegment << "items, total" << allocSegment*sizeof(TrackDataSegment) << "bytes";
    qDebug() << "folder allocated" << allocFolder << "items, total" << allocFolder*sizeof(TrackDataFolder) << "bytes";
    qDebug() << "trackpoint allocated" << allocTrackpoint << "items, total" << allocTrackpoint*sizeof(TrackDataTrackpoint) << "bytes";
    qDebug() << "waypoint allocated" << allocWaypoint << "items, total" << allocWaypoint*sizeof(TrackDataWaypoint) << "bytes";
    qDebug() << "routepoint allocated" << allocRoutepoint << "items, total" << allocRoutepoint*sizeof(TrackDataRoutepoint) << "bytes";
    qDebug() << "style allocated" << allocStyle << "items, total" << allocStyle*sizeof(Style) << "bytes";
    qDebug() << "child list allocated" << allocChildren;
    qDebug() << "metadata allocated" << allocMetadata;
    qDebug() << "***********";
}

#endif
