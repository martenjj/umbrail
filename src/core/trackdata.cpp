//////////////////////////////////////////////////////////////////////////
//									//
//  Project:	Umbrail - GPX track viewer and editor			//
//									//
//////////////////////////////////////////////////////////////////////////
//									//
//  Copyright (c) 2014-2022 Jonathan Marten <jjm@keelhaul.me.uk>	//
//  Home and download page: <http://github.com/martenjj/umbrail>	//
//									//
//  This program is free software; you can redistribute it and/or	//
//  modify it under the terms of the GNU General Public License as	//
//  published by the Free Software Foundation, either version 3 of	//
//  the License or (at your option) any later version.			//
//									//
//  It is distributed in the hope that it will be useful, but		//
//  WITHOUT ANY WARRANTY;  without even the implied warranty of		//
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the	//
//  GNU General Public License for more details.			//
//									//
//  You should have received a copy of the GNU General Public License	//
//  along with this program;  see the file COPYING for further		//
//  details.  If not, see <http://gnu.org/licenses/gpl>.      		//
//									//
//////////////////////////////////////////////////////////////////////////

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
#include "pointicon.h"
#include "pointiconprovider.h"
#include "category.h"

//////////////////////////////////////////////////////////////////////////
//									//
//  Debugging switches							//
//									//
//////////////////////////////////////////////////////////////////////////

#undef MEMORY_TRACKING
#undef DEBUG_ICONS
#define DEBUG_MERGE

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
static int counterContainer = 0;

#ifdef MEMORY_TRACKING
static int allocFile = 0;
static int allocTrack = 0;
static int allocRoute = 0;
static int allocSegment = 0;
static int allocTrackpoint = 0;
static int allocFolder = 0;
static int allocWaypoint = 0;
static int allocRoutepoint = 0;
static int allocContainer = 0;
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
    if (items==nullptr) return (TimeRange());
    int num = items->count();
    if (num==0) return (TimeRange());

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
    if (items==nullptr) return (BoundingArea());
    int num = items->count();
    if (num==0) return (BoundingArea());

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
    const QStringList names = path.split('/', Qt::SkipEmptyParts);
							// list of folder names
    const TrackDataItem *item = root;
    for (const QString &name : names)			// descend through path names
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


QString TrackData::iconForWaypointStatus(TrackData::WaypointStatus status)
{
    switch (status)
    {
default:
case TrackData::StatusNone:		return ("unknown");
case TrackData::StatusTodo:		return ("task-ongoing");
case TrackData::StatusDone:		return ("task-complete");
case TrackData::StatusQuestion:		return ("task-attempt");
case TrackData::StatusUnwanted:		return ("task-reject");
case TrackData::StatusInvalid:		return ("task-delegate");
    }
}


// based on NavMarks PointData::displayAddress()
QStringList TrackData::formattedAddress(const QVariant &street,
                                        const QVariant &city,
                                        const QVariant &state,
                                        const QVariant &pcode,
                                        const QVariant &cntry)
{
    QStringList result;

    // "StreetAddress", which may be multiple lines
    if (!street.isNull()) result.append(street.toString().split("\n", Qt::SkipEmptyParts));

    // "City"
    if (!city.isNull()) result.append(city.toString());

    // "State", if not the same as "City"
    // and not the same as the first two of "PostalCode" (France département)
    const QString &s = state.toString();
    const QString &p = pcode.toString();
    if (!s.isEmpty() && state!=city && !(s.length()==2 && s==p.left(2))) result.append(s);

    const QString &c = cntry.toString();
    if (!p.isEmpty() && !c.isEmpty())
    {
        // "PostalCode - Country" if both are present
        result.append(p+" - "+c);
    }
    else
    {
        // "PostalCode" or "Country"
        if (!p.isEmpty()) result.append(p);
        if (!c.isEmpty()) result.append(c);
    }

    return (result);
}


QVariant TrackData::valueOrNull(const QVariant &value)
{
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
    //   QVariant("str")	->	isValid()=true		isNull()=false
    //   QVariant("")		->	isValid()=true		isNull()=false
    //   QVariant(QString())	->	isValid()=true		isNull()=true
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

    // And also to a string list.  No other sort of list is ever
    // stored in item metadata.
    if (val.type()==QVariant::StringList)
    {
        if (val.toStringList().isEmpty()) val.clear();
    }

    return (val);
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
    mExplicitName = false;				// explicit name not set
}


TrackDataItem::~TrackDataItem()
{
    if (mChildren!=nullptr) qDeleteAll(*mChildren);	// delete children if any
    delete mChildren;					// delete child list if present
    delete mMetadata;					// delete metadata if present
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
							// set value of variant
    mMetadata->replace(idx, TrackData::valueOrNull(value));
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


const PointIcon *TrackDataItem::icon() const
{
    // Named item type icons are always taken from the "system"
    // (which includes our application) namespace.
    return (PointIconProvider::self()->icon(this->iconName(), PointIcon::NamespaceSystem));
}


const TrackDataFile *TrackDataItem::root() const
{
    // Find the root file item that this item belongs to, or NULL
    // if the item is not part of the data tree.
    const TrackDataItem *item = this;
    const TrackDataFile *root = nullptr;
    while (item!=nullptr)
    {
        root = dynamic_cast<const TrackDataFile *>(item);
        if (root!=nullptr) break;
        item = item->parent();
    }

    return (root);
}


// Although in practice media and stops are only expected to be
// associated with TrackDataWaypoint items, this is in TrackDataItem
// so that the temporary item untyped provided by MetadataModel can
// be checked in the same way.
TrackData::MediaType TrackDataItem::mediaType() const
{
    QVariant n = metadata("stop");			// first try saved stop data
    if (!n.isNull()) return (TrackData::MediaStop);	// this means it's a stop

    n = metadata("link");				// then get saved link name
    // TODO: eliminate "media" here and in MediaPlayer, translate in importer
    if (n.isNull()) n = metadata("media");		// compatibility with old metadata
    if (n.isNull()) n = name();				// lastly try our waypoint name
    if (n.isNull()) return (TrackData::MediaNormal);	// no media data present

    QString ns = n.toString();
    // TODO: should get MIME type for extension and then compare against recognised ones
    // or even look for a general category (audio/... video/... image/... respectively)
    if (ns.contains(QRegExp("\\.3gp$", Qt::CaseInsensitive))) return (TrackData::MediaAudioNote);
    if (ns.contains(QRegExp("\\.mp4$", Qt::CaseInsensitive))) return (TrackData::MediaVideoNote);
    if (ns.contains(QRegExp("\\.jpg$", Qt::CaseInsensitive))) return (TrackData::MediaPhoto);
    return (TrackData::MediaNormal);
}

//////////////////////////////////////////////////////////////////////////
//									//
//  TrackDataContainer							//
//									//
//////////////////////////////////////////////////////////////////////////

TrackDataContainer::TrackDataContainer()
    : TrackDataItem("container_%04d", &counterContainer)
{
#ifdef MEMORY_TRACKING
    ++allocContainer;
#endif
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
    mCategories = nullptr;				// categories not yet set
}


TrackDataFile::~TrackDataFile()
{
    delete mCategories;					// delete categories if present
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
    if (num==0) return (TimeRange());

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


QString TrackDataWaypoint::iconName() const
{
    switch (mediaType())
    {
case TrackData::MediaNormal:		return ("favorites");
case TrackData::MediaAudioNote:		return ("speaker");
case TrackData::MediaVideoNote:		return ("mixer-video");
case TrackData::MediaPhoto:		return ("image-x-generic");
case TrackData::MediaStop:		return ("media-playback-stop");
default:				return ("unknown");
    }
}


bool TrackDataWaypoint::isMediaType() const
{
    const TrackData::MediaType wpt = mediaType();
    return (wpt==TrackData::MediaAudioNote ||
            wpt==TrackData::MediaVideoNote ||
            wpt==TrackData::MediaPhoto);
}


// Special icons for waypoints.
//
// Falling back to the base TrackDataItem::icon() will
// look for a system icon with the name as returned
// by TrackDataWaypoint::iconName() above.

const PointIcon *TrackDataWaypoint::icon() const
{
    // First priority: special waypoint type
    if (mediaType()!=TrackData::MediaNormal) return (TrackDataItem::icon());

    // Second priority: named symbol
    QVariant v = metadata("sym");
    if (!v.isNull())
    {
        const QString sym = v.toString();
        if (!sym.isEmpty())				// should always be the case
        {
#ifdef DEBUG_ICONS
            qDebug() << "for" << name() << "sym" << sym;
#endif
            const PointIcon *ic = PointIconProvider::self()->icon(sym);
            if (ic->isValid()) return (ic);
        }
    }

    // Third priority: explicit point colour or fallback colour
    //
    // As originally noted for MapView::resolvePointColour(), point colour
    // is currently not inherited.  If set on this item then it will be used,
    // otherwise waypoints will use the category colour or the default icon.
    // The top level file item colour or the application default waypoint
    // colour is never actually used.
    v = metadata("pointcolor");
    if (!v.isNull())
    {
        const QColor col = v.value<QColor>();
        if (col.isValid() && col.alpha()==255)		// valid colour and not inherit
        {
#ifdef DEBUG_ICONS
            qDebug() << "for" << name() << "colour" << col.name();
#endif
            const PointIcon *ic = PointIconProvider::self()->icon(col);
            if (ic->isValid()) return (ic);
        }
    }

    // Fourth priority: colour for category
    v = metadata("category");
    if (!v.isNull())
    {
        const QString cat = v.toStringList().first();	// first (primary) category only
        const TrackDataFile *root = this->root();	// go up to the root file item
        if (root!=nullptr)				// should always have been found
        {
            // If the file has categories available, then get the colour for
            // the waypoint category.
            const CategoryList *catMap = root->categories();
            if (catMap!=nullptr)			// categories set for file
            {
                const QColor col = catMap->category(cat).colour();
                if (col.isValid())			// colour is defined for category
                {
#ifdef DEBUG_ICONS
                    qDebug() << "for" << name() << "category" << cat << "->" << col.name();
#endif
                    const PointIcon *ic = PointIconProvider::self()->icon(col);
                    if (ic->isValid()) return (ic);
                }
            }
        }
    }

    // Lowest priority: default icon
    // waypointType() must be TrackData::WaypointNormal here
    return (TrackDataItem::icon());
}


// Merging waypoints, compatibility and automatic merge
//
// based on NavMarks PointData::canMerge() and PointData::mergeWith()

QStringList TrackDataWaypoint::formattedAddress() const
{
    return (TrackData::formattedAddress(metadata("StreetAddress"), metadata("City"),
                                        metadata("State"), metadata("PostalCode"),
                                        metadata("Country")));
}


#define LATLONGTOL              (5.0/(60*60*10))	// 0.5 seconds of angle,
							// about 15 metres at equator
#define ELEVTOL			(1.0)			// 1 metre
#define NAMEMIN			10			// minimum for prefix match
#define WAYPOINT		"Waypoint"		// default symbol name


static inline bool symbolIsValid(const QVariant &sym)
{
    return (sym.isValid() && sym.toString()!=WAYPOINT);
}


static inline bool addressIsValid(const QStringList &addr)
{
    return (!addr.isEmpty() && !addr.join("").isEmpty());
}


static TrackData::WaypointFlags mergedFlags(TrackData::WaypointFlags flags1, TrackData::WaypointFlags flags2)
{
    // If combining a NewlyImported point with an existing one,
    // turn off that flag.
    TrackData::WaypointFlags f = flags1;
    const bool combinedImport = !(f & TrackData::NewlyImported) &&
                                (flags2 & TrackData::NewlyImported);
    f |= flags2;
    if (combinedImport) f &= ~TrackData::NewlyImported;
    return (f);
}


//  Merge criteria for automatic merging:
//
//    Name		either match exactly, or one an exact prefix of the other
//    Symbol		either match exactly, or one is WAYPOINT or blank
//    Lat/Long		both present and equal within LATLONGTOL
//    Elevation         if both present, must match within ELEVTOL
//    Address		if both present, must match exactly
//    Categories	irrelevant (will be combined on merge)
//    Sources		irrelevant (will be combined on merge)
//    Flags		irrelevant (will be combined on merge)

bool TrackDataWaypoint::canMerge(const TrackDataWaypoint *other, bool positionOnly) const
{
    // Latitude/Longitude
    if (fabs(this->latitude()-other->latitude())>LATLONGTOL) return (false);
    if (fabs(this->longitude()-other->longitude())>LATLONGTOL) return (false);

    // If only a position check is required, there nothing else to do.
    // This is used for checking compatibility before a manual merge.
    if (positionOnly) return (true);

    // Name
    const QString &n1 = this->name();
    const QString &n2 = other->name();
#ifdef DEBUG_MERGE
    qDebug() << "trying" << n2 << "into" << n1;
#endif
    if (n1!=n2)						// not an exact match
    {
        int preflen = qMin(n1.length(), n2.length());	// length of shortest
        if (preflen<NAMEMIN) return (false);		// too short for this match
        if (n1.left(preflen)!=n2.left(preflen))
        {
#ifdef DEBUG_MERGE
            qDebug() << "can't merge - name";
#endif
            return (false);
        }
    }

    // Symbol
    const QVariant &s1 = this->metadata("sym");
    const QVariant &s2 = other->metadata("sym");
    if (s1!=s2)
    {
        if (s1.isValid() && s2.isValid())
        {
#ifdef DEBUG_MERGE
            qDebug() << "can't merge - sym" << s2 << s1;
#endif
            return (false);
        }
    }

    // Elevation
    const double e1 = this->elevation();
    const double e2 = other->elevation();
    if (!ISNAN(e1) && !ISNAN(e2))
    {
        if (fabs(e1-e2)>ELEVTOL)
        {
#ifdef DEBUG_MERGE
            qDebug() << "can't merge - ele" << e2 << e1;
#endif
            return (false);
        }
    }

    // Address
    const QStringList &a1 = this->formattedAddress();
    const QStringList &a2 = other->formattedAddress();
    if (addressIsValid(a1) && addressIsValid(a2))
    {
        if (a1!=a2)
        {
#ifdef DEBUG_MERGE
            qDebug() << "can't merge - address";
#endif
            return (false);
        }
    }

    // All matched
#ifdef DEBUG_MERGE
    qDebug() << "can merge";
#endif
    return (true);
}


// This assumes that the points are compatible for merging,
// in other words canMerge() above would return true.

void TrackDataWaypoint::mergeWith(const TrackDataWaypoint *other)
{
    // Name - if one is an exact prefix of the other then take the longest,
    // otherwise just accept the first.
    const QString &n1 = this->name();
    const QString &n2 = other->name();
    if (n1!=n2)						// not an exact match
    {
        if (n1.length()<n2.length())			// second is the longest
        {
            if (n2.left(n1.length())==n1) setName(n2, other->hasExplicitName());
        }
    }

    // Symbol - accept the first unless that is the default WAYPOINT or blank,
    // in which case use the other.
    const QVariant &s1 = this->metadata("sym");
    const QVariant &s2 = other->metadata("sym");
    if (!symbolIsValid(s1) && symbolIsValid(s2)) setMetadata("sym", s2);

    // Latitude/Longtitude - just accept the first
    // (we know that they are both valid).

    // Elevation - accept the first unless it is invalid,
    // in which case use the other.
    if (ISNAN(this->elevation())) setMetadata("ele", other->elevation());

    // Address - accept the first unless it is completely blank,
    // in which case use the other.  Thie decision has to be made
    // by looking at the complete address and choosing one or the
    // other;  looking at each metadata item individually as with
    // the "Other data" below could potentially result in an address
    // as a mixture of both sources.
    const QStringList &a1 = this->formattedAddress();
    const QStringList &a2 = other->formattedAddress();
    if (!addressIsValid(a1) && addressIsValid(a2))
    {
        // TODO: instead if handling all five of these metadata items
        // separately and formatting them back and forth for display,
        // would it be possible to handle them as a single list value
        // "address" (semicolon separated for display/edit, convert ';'
        // to ',' on import) and assemble/decompose it on import/export?

        this->setMetadata("StreetAddress", other->metadata("StreetAddress"));
        this->setMetadata("City", other->metadata("City"));
        this->setMetadata("State", other->metadata("State"));
        this->setMetadata("PostalCode", other->metadata("PostalCode"));
        this->setMetadata("Country", other->metadata("Country"));
    }

    // Categories - merge the two lists.
    const QStringList &c1 = this->metadata("category").toStringList();
    const QStringList &c2 = other->metadata("category").toStringList();
    if (!c2.isEmpty())					// if there is something to merge
    {
        QStringList res = c1;
        for (const QString &c : qAsConst(c2))
        {
            if (!res.contains(c)) res.append(c);
        }

        setMetadata("category", res);
    }

    // Sources - only accept the first list unless it is empty,
    // in which case use the other.  This is for an automatic
    // merge - for a manual merge the two lists are combined.
    const QVariant &o1 = this->metadata("origin");
    const QVariant &o2 = other->metadata("origin");
    if (o1.isNull() && !o2.isNull()) setMetadata("origin", o2);

    // Flags - combine the two.
    TrackData::WaypointFlags f1 = static_cast<TrackData::WaypointFlags>(this->metadata("flags").toInt());
    TrackData::WaypointFlags f2 = static_cast<TrackData::WaypointFlags>(other->metadata("flags").toInt());
    TrackData::WaypointFlags res = mergedFlags(f1, f2);
    if (res!=f1) setMetadata("flags", static_cast<int>(res));

    // Other data - accept the first unless it is empty,
    // in which case use the other.
    for (int idx = 0; idx<DataIndexer::count(); ++idx)
    {
        const QByteArray &name = DataIndexer::name(idx);
        if (DataIndexer::isInternalTag(name)) continue;
        // These metadata items have been merged specially above.
        if (name=="ele" || name=="sym" || name=="StreetAddress" ||
            name=="City" || name=="State" || name=="PostalCode" ||
            name=="Country" || name=="category" || name=="origin" ||
            name=="flags") continue;

        const QVariant &m1 = this->metadata(idx);
        const QVariant &m2 = other->metadata(idx);
        if (m1.isNull() && !m2.isNull()) setMetadata(idx, m2);
    }

#ifdef DEBUG_MERGE
    qDebug() << "merged" << n2 << "into" << n1;
#endif
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
    qDebug() << "container" << sizeof(TrackDataContainer) << "bytes";
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
    qDebug() << "container allocated" << allocContainer << "items, total" << allocContainer*sizeof(TrackDataContainer) << "bytes";
    qDebug() << "child list allocated" << allocChildren;
    qDebug() << "metadata allocated" << allocMetadata;
    qDebug() << "***********";
}

#endif
