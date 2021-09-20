

#include "trackdata.h"

#include <qregexp.h>
#include <qdebug.h>
#include <qtimezone.h>
#include <qicon.h>
#include <qstandardpaths.h>

#include <klocalizedstring.h>
#include <kiconloader.h>

#include <kio/global.h>

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
    if (items==nullptr) return (TimeRange::null);
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
    if (items==nullptr) return (BoundingArea::null);
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
    if (items==nullptr) return (0);
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
    if (tz==nullptr) return (QLocale().toString(dt, QLocale::ShortFormat));

    QDateTime tzdt = dt.toUTC().toTimeZone(*tz);
    // QDateTime tzdt = tz->toZoneTime(dt.toUTC());
    //qDebug() << dt << "->" << tzdt << tz->abbreviation(dt);
    return (QLocale().toString(tzdt, QLocale::ShortFormat)+" "+tz->abbreviation(dt));
}


TrackDataFolder *TrackData::findFolderByPath(const QString &path, const TrackDataItem *root)
{
    if (path.isEmpty()) return (nullptr);		// check for null path
    const QStringList names = path.split('/');		// list of folder names

    const TrackDataItem *item = root;
    foreach (const QString &name, names)		// descend through path names
    {
        const int cnt = item->childCount();
        if (cnt==0) return (nullptr);			// no children under this item

        const TrackDataItem *folderItem = nullptr;
        for (int i = 0; i<cnt; ++i)			// search through children
        {
            const TrackDataFolder *fold = dynamic_cast<const TrackDataFolder *>(item->childAt(i));
            if (fold!=nullptr)				// child item is a folder
            {
                if (fold->name()==name)			// folder name matches
                {
                    folderItem = fold;			// continue from this item
                    break;
                }
            }
        }

        if (folderItem==nullptr) return (nullptr);	// no child folder found
        item = folderItem;				// continue descent from here
    }

    return (const_cast<TrackDataFolder *>(dynamic_cast<const TrackDataFolder *>(item)));
}


QString TrackData::formattedWaypointStatus(TrackData::WaypointStatus status, bool blankForNone)
{
    switch (status)
    {
case TrackData::StatusNone:		return (blankForNone ? QString() : i18n("(None)"));
case TrackData::StatusTodo:		return (i18n("To Do"));
case TrackData::StatusDone:		return (i18n("Done"));
case TrackData::StatusQuestion:		return (i18n("Uncertain"));
case TrackData::StatusUnwanted:		return (i18n("Unwanted"));
case TrackData::StatusInvalid:		return (i18n("(Invalid)"));
default:				return (i18n("(Unknown %1)", status));
    }
}

//////////////////////////////////////////////////////////////////////////
//									//
//  TrackDataItem							//
//									//
//////////////////////////////////////////////////////////////////////////

TrackDataItem::TrackDataItem(const char *format, int *counter)
{
    init();
    if (format!=nullptr) mName = QString::asprintf(format, ++(*counter));
}


void TrackDataItem::init()
{
    mChildren = nullptr;				// no children yet
    mParent = nullptr;					// not attached to parent
    mMetadata = nullptr;				// no metadata yet
    mSelectionId = 1;					// nothing selected yet
    mExplicitName = false;
}


TrackDataItem::~TrackDataItem()
{
    if (mChildren!=nullptr) qDeleteAll(*mChildren);
    delete mChildren;
    delete mMetadata;
}


void TrackDataItem::setName(const QString &newName, bool explicitName)
{
    mName = newName;
    mExplicitName = explicitName;
}


void TrackDataItem::addChildItem(TrackDataItem *data, int idx)
{
    if (data->parent()!=nullptr) qWarning() << "item" << data->name() << "already has parent" << data->parent()->name();
    Q_ASSERT(data->parent()==nullptr);

    if (mChildren==nullptr)
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
    Q_ASSERT(mChildren!=nullptr);
    Q_ASSERT(!mChildren->isEmpty());
    TrackDataItem *data = mChildren->takeLast();
    data->mParent = nullptr;				// now no longer has parent
    return (data);
}


TrackDataItem *TrackDataItem::takeFirstChildItem()
{
    Q_ASSERT(mChildren!=nullptr);
    Q_ASSERT(!mChildren->isEmpty());
    TrackDataItem *data = mChildren->takeFirst();
    data->mParent = nullptr;				// now no longer has parent
    return (data);
}


TrackDataItem *TrackDataItem::takeChildItem(int idx)
{
    Q_ASSERT(mChildren!=nullptr);
    Q_ASSERT(idx>=0 && idx<mChildren->count());
    TrackDataItem *data = mChildren->takeAt(idx);
    data->mParent = nullptr;				// now no longer has parent
    return (data);
}


void TrackDataItem::removeChildItem(TrackDataItem *item)
{
    Q_ASSERT(mChildren!=nullptr);
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


void TrackDataItem::setMetadata(int idx, const QVariant &value)
{
    if (mMetadata==nullptr)				// allocate array if needed
    {
#ifdef MEMORY_TRACKING
        ++allocMetadata;
#endif
        mMetadata = new QVector<QVariant>;
    }

    const int cnt = mMetadata->count();			// current size of array
    if (idx>=cnt) mMetadata->resize(idx+1);		// need to allocate more

    QVariant val = value;				// provided new value

    // Strings are a special case;  setting a null string item sets a null QVariant
    // as the value.  This is so that QVariant::isNull() can be used to test the
    // metadata value, without having to convert it to a string, and will give the
    // expected result.  In this application an empty string is always considered
    // to be equivalent to there being no metadata value.
    //
    // Results obtained by experimentation:
    //
    //   QVariant()		->	isValid()=false		isNull()=true
    //	 QVariant("str")	->	isValid()=true		isNull()=false
    //	 QVariant("")		->	isValid()=true		isNull()=false
    //	 QVariant(QString())	->	isValid()=true		isNull()=true
    //
    // Do not do this test with QVariant::canConvert(QMetaType::QString),
    // there are many types that can be converted to a QString but we
    // want to make sure that the value really is a string.
    if (val.type()==QVariant::String || val.type()==QVariant::ByteArray)
    {
        if (val.toString().isEmpty()) val.clear();
    }

    // The same reasoning as above applies to a colour value.
    if (val.type()==QVariant::Color)
    {
        if (!val.value<QColor>().isValid()) val.clear();
    }

    mMetadata->replace(idx, val);				// set value of variant
}


void TrackDataItem::setMetadata(const QByteArray &key, const QVariant &value)
{
    setMetadata(DataIndexer::index(key), value);
}


QVariant TrackDataItem::metadata(int idx) const
{
    if (mMetadata==nullptr) return (QVariant());
    return (mMetadata->value(idx));			// performs the bounds checking
}


QVariant TrackDataItem::metadata(const QByteArray &key) const
{
    if (mMetadata==nullptr) return (QVariant());
    return (metadata(DataIndexer::index(key)));
}


void TrackDataItem::copyMetadata(const TrackDataItem *other, bool overwrite)
{
    if (other->mMetadata==nullptr) return;		// nothing to copy

    for (int idx = 0; idx<other->mMetadata->size(); ++idx)
    {
        QVariant om = other->mMetadata->at(idx);
        if (om.isNull()) continue;			// no source metadata
        QVariant tm = this->metadata(idx);
        if (!tm.isNull() && !overwrite) continue;	// already in destination and no overwrite
        this->setMetadata(idx, om);
    }
}


QString TrackDataItem::timeZone() const
{
    const TrackDataItem *item = this;
    while (item!=nullptr)
    {
        const QVariant &v = item->metadata("timezone");	// look for timezone in metadata
        if (!v.isNull()) return (v.toString());
        item = item->parent();				// if present, use that
    }

    return (QString());					// no time zone in item tree
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
    Q_ASSERT(firstPoint!=nullptr);
    if (num==1) return (TimeRange(firstPoint->time(), firstPoint->time()));

    const TrackDataTrackpoint *lastPoint = dynamic_cast<const TrackDataTrackpoint *>(childAt(num-1));
    Q_ASSERT(lastPoint!=nullptr);
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
}


double TrackDataAbstractPoint::elevation() const
{
    const QVariant v = metadata("ele");
    if (v.isNull()) return (NAN);
    return (v.toDouble());
}


QString TrackDataAbstractPoint::formattedElevation() const
{
    const double e = elevation();
    if (ISNAN(e)) return (i18nc("an unknown quantity", "unknown"));
    return (i18nc("@item:intable Number with unit of metres", "%1 m", QString::number(e, 'f', 1)));
}


QDateTime TrackDataAbstractPoint::time() const
{
    const QVariant v = metadata("time");
    if (v.isNull()) return (QDateTime());
    return (v.toDateTime());
}


QString TrackDataAbstractPoint::formattedTime(bool withZone) const
{
    const QDateTime dt = time();
    if (!dt.isValid()) return (i18nc("an unknown date/time", "unknown"));

    if (withZone)
    {
        QString zoneName = timeZone();
        if (!zoneName.isEmpty())
        {
            QTimeZone tz(zoneName.toLatin1());
            if (tz.isValid()) return (TrackData::formattedTime(dt, &tz));
            qWarning() << "unknown time zone" << zoneName;
        }
    }

    return (TrackData::formattedTime(dt));
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

    while (pnt!=nullptr && dynamic_cast<const TrackDataFile *>(pnt)==nullptr)
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
    QVariant n = metadata("stop");			// first try saved stop data
    if (!n.isNull()) return (TrackData::WaypointStop);	// this means it's a stop

    n = metadata("link");				// then get saved link name
    // TODO: eliminate "media" here and in MediaPlayer, translate in importer
    if (n.isNull()) n = metadata("media");		// compatibility with old metadata
    if (n.isNull()) n = name();				// lastly try our waypoint name
    if (n.isNull()) return (TrackData::WaypointNormal);	// no media data present

    QString ns = n.toString();
    // TODO: should get MIME type for extension and then compare against recognised ones
    // or even look for a general category (audio/... video/... image/... respectively)
    if (ns.contains(QRegExp("\\.3gp$", Qt::CaseInsensitive))) return (TrackData::WaypointAudioNote);
    if (ns.contains(QRegExp("\\.mp4$", Qt::CaseInsensitive))) return (TrackData::WaypointVideoNote);
    if (ns.contains(QRegExp("\\.jpg$", Qt::CaseInsensitive))) return (TrackData::WaypointPhoto);
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

    const QColor col = metadata("pointcolor").value<QColor>();
    if (!col.isValid()) return (TrackDataItem::icon());
#ifdef DEBUG_ICONS
    qDebug() << "need icon for waypoint" << name() << "colour" << col.name();
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
    qDebug() << "child list allocated" << allocChildren;
    qDebug() << "metadata allocated" << allocMetadata;
    qDebug() << "***********";
}

#endif
