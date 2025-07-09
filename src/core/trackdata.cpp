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

#include <qdebug.h>
#include <qtimezone.h>
#include <qicon.h>
#include <qstandardpaths.h>

#include <klocalizedstring.h>
#include <kiconloader.h>
#include <kstringhandler.h>

#include <kio/global.h>

#include "dataindexer.h"
#include "pointicon.h"
#include "category.h"
#include "abstracticonprovider.h"

//////////////////////////////////////////////////////////////////////////
//									//
//  Debugging switches							//
//									//
//////////////////////////////////////////////////////////////////////////

#undef MEMORY_TRACKING
#undef DEBUG_ICONS
#undef DEBUG_MERGE

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
static int allocChildren = 0;
static int allocContainer = 0;
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
    for (int i = 0; i<items->count(); ++i)
    {
        const TrackDataContainer *tdc = AS(TrackDataContainer, items->at(i));
        if (tdc!=nullptr) num += tdc->childCount();
    }
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


TrackDataFolder *TrackData::findFolderByPath(const QString &path, const TrackDataContainer *root)
{
    if (root==nullptr) return (nullptr);		// must have a root
    if (path.isEmpty()) return (nullptr);		// check for null path
    const QStringList names = path.split('/', Qt::SkipEmptyParts);
							// list of folder names
    const TrackDataContainer *item = root;
    for (const QString &name : names)			// descend through path names
    {
        const int cnt = item->childCount();
        if (cnt==0) return (nullptr);			// no children under this item

        const TrackDataFolder *folderItem = nullptr;
        for (int i = 0; i<cnt; ++i)			// search through children
        {
            const TrackDataFolder *fold = AS(TrackDataFolder, item->childAt(i));
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
        item = static_cast<const TrackDataContainer *>(folderItem);
    }							// continue descent from here

    return (ASV(TrackDataFolder, const_cast<TrackDataContainer *>(item)));
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


QVariant TrackData::valueOrNull(const QVariant &v)
{
    QVariant val = v;					// provided new value

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
    if (val.typeId()==QVariant::String || val.typeId()==QVariant::ByteArray)
    {
        if (val.toString().isEmpty()) val.clear();
    }

    // The same reasoning as above applies to a colour value.
    if (val.typeId()==QVariant::Color)
    {
        if (!val.value<QColor>().isValid()) val.clear();
    }

    // And also to a string list.  No other sort of list is ever
    // stored in item metadata.
    if (val.typeId()==QVariant::StringList)
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
    mParent = nullptr;					// not attached to parent
    mMetadata = nullptr;				// no metadata yet
    mSelectionId = 1;					// nothing selected yet
    mExplicitName = false;				// explicit name not set
}


TrackDataItem::~TrackDataItem()
{
    delete mMetadata;					// delete metadata if present
}


void TrackDataItem::setName(const QString &newName, bool explicitName)
{
    mName = newName;
    mExplicitName = explicitName;
}


BoundingArea TrackDataItem::boundingArea() const
{
    const TrackDataContainer *tdc = AS(TrackDataContainer, this);
    return (tdc!=nullptr ? tdc->boundingArea() : BoundingArea());
}


TimeRange TrackDataItem::timeSpan() const
{
    const TrackDataContainer *tdc = AS(TrackDataContainer, this);
    return (tdc!=nullptr ? tdc->timeSpan() : TimeRange());
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
    if (key.isEmpty()) return (QVariant());
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
    // (which includes our application) namespace.  The colour
    // and shape do not apply to an icon in that namespace.
    return (PointIcon::create(this->iconName(), PointIcon::NamespaceSystem));
}


// This finds the TrackDataFile which is the ancestor of an item,
// although that may not necessarily be the root of the data tree.
// The categories and timezone properties of the root file item
// apply to everything below it.
const TrackDataFile *TrackDataItem::rootFileItem() const
{
    const TrackDataItem *item = this;
    const TrackDataFile *root = nullptr;
    while (item!=nullptr)
    {
        root = AS(TrackDataFile, item);
        if (root!=nullptr) break;
        item = item->parent();
    }

    return (root);
}

//////////////////////////////////////////////////////////////////////////
//									//
//  TrackDataContainer							//
//									//
//////////////////////////////////////////////////////////////////////////

TrackDataContainer::TrackDataContainer(const char *format, int *counter)
    : TrackDataItem(format, counter)
{
#ifdef MEMORY_TRACKING
    ++allocContainer;
#endif
    mChildren = nullptr;
}


TrackDataContainer::~TrackDataContainer()
{
    if (mChildren!=nullptr) qDeleteAll(*mChildren);	// delete children if any
    delete mChildren;					// delete child list if present
}

void TrackDataContainer::addChildItem(TrackDataItem *data, int idx)
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


TrackDataItem *TrackDataContainer::takeLastChildItem()
{
    Q_ASSERT(mChildren!=nullptr);
    Q_ASSERT(!mChildren->isEmpty());
    TrackDataItem *data = mChildren->takeLast();
    data->mParent = nullptr;				// now no longer has parent
    return (data);
}


TrackDataItem *TrackDataContainer::takeFirstChildItem()
{
    Q_ASSERT(mChildren!=nullptr);
    Q_ASSERT(!mChildren->isEmpty());
    TrackDataItem *data = mChildren->takeFirst();
    data->mParent = nullptr;				// now no longer has parent
    return (data);
}


TrackDataItem *TrackDataContainer::takeChildItem(int idx)
{
    Q_ASSERT(mChildren!=nullptr);
    Q_ASSERT(idx>=0 && idx<mChildren->count());
    TrackDataItem *data = mChildren->takeAt(idx);
    data->mParent = nullptr;				// now no longer has parent
    return (data);
}


void TrackDataContainer::removeChildItem(TrackDataItem *item)
{
    Q_ASSERT(mChildren!=nullptr);
    takeChildItem(mChildren->indexOf(item));
}


BoundingArea TrackDataContainer::boundingArea() const
{
    return (TrackData::unifyBoundingAreas(mChildren));
}


TimeRange TrackDataContainer::timeSpan() const
{
    return (TrackData::unifyTimeSpans(mChildren));
}

//////////////////////////////////////////////////////////////////////////
//									//
//  TrackDataFile							//
//									//
//////////////////////////////////////////////////////////////////////////

TrackDataFile::TrackDataFile()
    : TrackDataContainer("file_%02d", &counterFile)
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


QString TrackDataFile::selectionStatus(int num) const
{
    if (num==1) return (i18n("Selected file '%1'", name()));
    else return (i18np("Selected %1 file", "Selected %1 files", num));
}


QString TrackDataFile::toolTip() const
{
    return (i18np("File '%2' with %1 item", "File %2 with %1 items", childCount(), fileName().toDisplayString()));
}

//////////////////////////////////////////////////////////////////////////
//									//
//  TrackDataTrack							//
//									//
//////////////////////////////////////////////////////////////////////////

TrackDataTrack::TrackDataTrack()
    : TrackDataContainer("track_%02d", &counterTrack)
{
#ifdef MEMORY_TRACKING
    ++allocTrack;
#endif
}


QString TrackDataTrack::selectionStatus(int num) const
{
    if (num==1) return (i18n("Selected track '%1'", name()));
    else return (i18np("Selected %1 track", "Selected %1 tracks", num));
}


QString TrackDataTrack::toolTip() const
{
    return (i18np("Track with %1 segment", "Track with %1 segments", childCount()));
}

//////////////////////////////////////////////////////////////////////////
//									//
//  TrackDataSegment							//
//									//
//////////////////////////////////////////////////////////////////////////

TrackDataSegment::TrackDataSegment()
    : TrackDataContainer("segment_%02d", &counterSegment)
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

    const TrackDataTrackpoint *firstPoint = ASX(TrackDataTrackpoint, childAt(0));
    if (num==1) return (TimeRange(firstPoint->time(), firstPoint->time()));

    const TrackDataTrackpoint *lastPoint = ASX(TrackDataTrackpoint, childAt(num-1));
    return (TimeRange(firstPoint->time(), lastPoint->time()));
}


QString TrackDataSegment::selectionStatus(int num) const
{
    if (num==1) return (i18n("Selected segment '%1'", name()));
    else return (i18np("Selected %1 segment", "Selected %1 segments", num));
}


QString TrackDataSegment::toolTip() const
{
    return (i18np("Segment with %1 point", "Segment with %1 points", childCount()));
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
    : TrackDataContainer("folder_%02d", &counterFolder)
{
#ifdef MEMORY_TRACKING
    ++allocFolder;
#endif
}


QString TrackDataFolder::path() const
{
    QStringList p(name());
    const TrackDataItem *pnt = parent();

    while (pnt!=nullptr && !IS(TrackDataFile, pnt))
    {
        p.prepend(pnt->name());
        pnt = pnt->parent();
    }

    return (p.join("/"));
}


QString TrackDataFolder::selectionStatus(int num) const
{
    QString msg;
    if (num==1)
    {
        int numWaypoint = 0;
        int numTodo = 0;
        int numDone = 0;

        const int childs = childCount();
        for (int i = 0; i<childs; ++i)
        {
            const TrackDataWaypoint *tdw = AS(TrackDataWaypoint, childAt(i));
            if (tdw==nullptr) continue;
            ++numWaypoint;

            switch (tdw->metadata("status").toInt())
            {
case TrackData::StatusTodo:     ++numTodo;	break;
case TrackData::StatusDone:     ++numDone;	break;
            }
        }

        const int numOther = numWaypoint-(numTodo+numDone);
        if ((numTodo+numDone)==0) msg = i18n("Selected folder '%1': %2 waypoints", name(), numWaypoint);
        else if (numOther==0) msg = i18n("Selected folder '%1': %2 waypoints, %3 done, %4 to do", name(), numWaypoint, numDone, numTodo);
        else msg = i18n("Selected folder '%1': %2 waypoints, %3 done, %5 other, %4 to do", name(), numWaypoint, numDone, numTodo, numOther);
    }
    else msg = i18np("Selected %1 folder", "Selected %1 folders", num);
    return (msg);
}


QString TrackDataFolder::toolTip() const
{
    return (i18np("Folder with %1 item", "Folder with %1 items", childCount()));
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


QString TrackDataTrackpoint::selectionStatus(int num) const
{
    if (num==1) return (i18n("Selected point '%1'", name()));
    else return (i18np("Selected %1 point", "Selected %1 points", num));
}


QString TrackDataTrackpoint::toolTip() const
{
    const double e = elevation();
    return (ISNAN(e) ? i18n("Point at time %1", formattedTime(true))
                     : i18n("Point at time %1, elevation %2", formattedTime(true), formattedElevation()));
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


QString TrackDataWaypoint::selectionStatus(int num) const
{
    QString msg;
    if (num==1)
    {
        msg = i18n("Selected waypoint '%1'", name());
        const QString wptStatus = TrackData::formattedWaypointStatus(static_cast<TrackData::WaypointStatus>(metadata("status").toInt()), true);
        if (!wptStatus.isEmpty()) msg += (" ("+wptStatus+")");

        QString wptDesc = metadata("desc").toString();
        int idx = wptDesc.indexOf('\n');
        if (idx!=-1) wptDesc = wptDesc.left(idx);
        if (!wptDesc.isEmpty()) msg += (" \""+KStringHandler::rsqueeze(wptDesc, 50)+"\"");
    }
    else msg = i18np("Selected %1 waypoint", "Selected %1 waypoints", num);
    return (msg);
}


QString TrackDataWaypoint::toolTip() const
{
    // This and similar logic for other types is indeed an I18N
    // word puzzle, but the only alternative is a proliferation
    // of messages.
    QString res = i18n("Waypoint");

    const QDateTime dt = time();
    if (dt.isValid()) res += i18n(" at time %1", formattedTime(true));

    const double e = elevation();
    if (!ISNAN(e)) res += i18n(", elevation %1", formattedElevation());

    const QString wptStatus = formattedWaypointStatus(static_cast<TrackData::WaypointStatus>(metadata("status").toInt()), true);
    if (!wptStatus.isEmpty()) res += QString(" (%1)").arg(wptStatus);

    return (res);
}


bool TrackDataWaypoint::isMediaType() const
{
    const TrackData::MediaType wpt = mediaType();
    return (wpt==TrackData::MediaAudioNote ||
            wpt==TrackData::MediaVideoNote ||
            wpt==TrackData::MediaPhoto);
}


TrackData::MediaType TrackDataWaypoint::mediaType() const
{
    QVariant n = metadata("stop");			// first try saved stop data
    if (!n.isNull()) return (TrackData::MediaStop);	// this means it's a stop

    n = metadata("link");				// then get saved link name
    if (n.isNull()) n = metadata("media");		// compatibility with old metadata
    if (n.isNull()) n = name();				// lastly try our waypoint name
    if (n.isNull()) return (TrackData::MediaNormal);	// no media data present

    QString ns = n.toString();
    // TODO: should get MIME type for extension and then compare against recognised ones
    // or even look for a general category (audio/... video/... image/... respectively)
    if (ns.endsWith(".3gp", Qt::CaseInsensitive)) return (TrackData::MediaAudioNote);
    if (ns.endsWith(".mp4", Qt::CaseInsensitive)) return (TrackData::MediaVideoNote);
    if (ns.endsWith(".jpg", Qt::CaseInsensitive)) return (TrackData::MediaPhoto);
    return (TrackData::MediaNormal);
}


// Get the item category data, if the item has a category set and
// the file has a category map defining that category.
static const CategoryData *findCategoryData(const TrackDataItem *item)
{
    const CategoryData *d = nullptr;

    const QVariant cats = item->metadata("category");
    if (!cats.isNull())
    {							// first (primary) category only
        const QString cat = cats.toStringList().first();
        const TrackDataFile *root = item->rootFileItem();
        if (root!=nullptr)				// go up to the root file item,
        {						// should always have been found
            // If the file has categories available, then get the data for
            // the waypoint category from the map.  If the category is not
            // defined in the map then CategoryList::category() will return
            // a default-constructed CategoryData via QMap::value().
            const CategoryList *catMap = root->categories();
            if (catMap!=nullptr) d = catMap->category(cat);
        }
    }

    return (d);
}


// Find a metadata item which may be directly from the item itself, or
// defined for the item's category, or set for a parent item up to the
// root.

static QVariant findInheritedMetadata(const TrackDataItem *item, int idx, bool wantColour)
{
    // First try the item metadata directly.
    QVariant v = item->metadata(idx);
    // Do not use that value, however, if it signifies an inherited colour.
    if (wantColour && !TrackData::colourUnlessInherit(v).isValid()) v.clear();

    if (v.isNull())
    {
        // Then try the item category, if it is set and the file has
        // a category map defining that category.
        const CategoryData *catData = findCategoryData(item);
        if (catData!=nullptr)
        {
            if (wantColour)				// want a colour value
            {
                const QColor &col = TrackData::colourUnlessInherit(catData->colour());
                if (col.isValid()) v = col;		// valid colour is set
            }
            else					// want a string value
            {
                const QString &str = catData->shape();
                if (!str.isEmpty()) v = str;		// valid shape is set
            }
        }
    }

    if (v.isNull())
    {
        // Next try the item's parents, up to and including the root.  Only
        // the metadata which is requested is searched along this chain, not
        // a category.
        const TrackDataItem *pnt = item->parent();
        while (pnt!=nullptr)
        {
            v = pnt->metadata(idx);
            if (!v.isNull())				// metadata found, but check colour
            {
                if (wantColour)				// want a colour value,
                {					// but only if not inheriting
                    const QColor &col = TrackData::colourUnlessInherit(v);
                    if (!col.isValid()) v.clear();
                }
            }

            if (!v.isNull()) break;
            pnt = pnt->parent();
        }
    }

    // Return the value found, or a null variant if there was none.
    // If none was found then the caller will apply an appropriate default.
    return (v);
}


// Special icons for waypoints.
//
// Falling back to the base TrackDataItem::icon() will
// look for a system icon with the name as returned
// by TrackDataWaypoint::iconName() above.

const PointIcon *TrackDataWaypoint::icon() const
{
    // First priority:  special waypoint type
    if (mediaType()!=TrackData::MediaNormal) return (TrackDataItem::icon());

    QVariant v;
    const PointIcon *ic;

    // Next resolve the icon name - if there is none then there is no point
    // in trying an icon provider.  However, we need to either know the
    // intended icon namespace, or try the providers in priority order,
    // in order to know which metadata tag the name will be stored under.
    // The priority order is set by the order in which the providers are
    // created in PointIcon::initProviders().  If a provider is not enabled,
    // it is ignored here.
    const QByteArray set = metadata("symset").toByteArray();
    const AbstractIconProvider *usedProvider = nullptr;
    const auto *providers = PointIcon::allProviders();
    for (const AbstractIconProvider *provider : std::as_const(*providers))
    {
        if (!provider->isEnabled()) continue;
        if (!set.isEmpty() && set!=provider->internalName()) continue;
        v = metadata(provider->metadataKey());
        if (!v.isNull())
        {
            usedProvider = provider;
            break;
        }
    }

    if (v.isNull())
    {
        // If there is no name then try the waypoint category, which may
        // have a default icon name defined.  In imported files this will
        // only be present for OsmAnd, but keep the lookup non-specific.
        const CategoryData *catData = findCategoryData(this);
        if (catData!=nullptr)
        {
            const QString &icn = catData->icon();
            if (!icn.isEmpty()) v = icn;
        }
    }

    // All of the possible icon name sources have now been tried.
    const QString name = v.toString();

    // As well as the waypoint icon name, the colour and background shape
    // need to be resolved at this stage.  This is so that they can be
    // used to generate the icon image if the icon turns out to be an OsmAnd
    // one, either explicitly specified or found by name matching.  Even
    // if there is no icon name or the provider does not require a colour,
    // it will be used to generate the default "star" image.
    //
    // The inheritance priority for colour and shape is first any explicitly
    // specified for the waypoint, then from the category if defined, then
    // explicitly specified for any parent items up to the root.  Category,
    // if it needs to be used, is taken from the waypoint's primary category
    // only and is not inherited.
    //
    // Colour and shape are searched in this way independently, except that
    // the default shape is determined by the icon provider and for OsmAnd
    // icons is effectively "circle".  The default colour is again determined
    // by the icon provider;  in theory they should use the application default
    // if an icon colour is meaningful but currently the OsmAnd icon provider
    // does not.
    //
    // As originally noted for MapView::resolvePointColour(), the point colour
    // was not inherited from a parent folder or file and the application
    // default waypoint colour was never used.  However, in the interests of
    // consistency the parent search is now done, but the application default
    // colour is not curently used because that would need this core library
    // to have access to the application settings.

    // Colour may be needed below, but shape is only needed if there is an
    // icon name.
    const QVariant col = findInheritedMetadata(this, DataIndexer::index("pointcolor"), true);

    if (!name.isEmpty())
    {
        const QVariant shp = findInheritedMetadata(this, DataIndexer::index("background"), false);

        // Second priority:  if an icon name has been found, use the
        // appropriate provider found earlier (recorded in 'usedProvider')
        // to try to create an icon image.  If the icon name was taken from
        // the category map then no provider will have been found, so a name
        // search in provider priority order will be done by PointIcon.
        //
        // This may give incorrect results in the case where, for example,
        // the waypoint metadata is:
        //
        //   symset	(null)
        //   sym	an invalid Garmin name
        //   icon	a valid OsmAnd name
        //
        // because finding the non-null "sym" earlier, at a higher priority,
        // would have noted the provider as Garmin.  The create() below will
        // then try to create a Garmin icon with that name and fail;  however,
        // the valid OsmAnd name will not be tried.  This scenario is not
        // likely to happen with either files correctly saved from this
        // application (because they will include an explicit "symset"), or
        // files imported from OsmAnd (because they will not contain the "sym"
        // tag anyway).
        PointIcon::IconNamespace nsp = (usedProvider!=nullptr ? usedProvider->namespaceId() : PointIcon::NamespaceAuto);
        ic = PointIcon::create(name, nsp, col, shp);
        if (ic->isValid()) return (ic);
    }

    // Third priority: if there is no icon name but there is a colour, then
    // generate the standard "star" symbol.
    if (!col.isNull())
    {
        const QColor c = TrackData::colourUnlessInherit(col);
        if (c.isValid())				// valid colour and not inherit
        {
#ifdef DEBUG_ICONS
            qDebug() << "for" << name() << "colour" << c.name();
#endif
            ic = PointIcon::create(c);
            if (ic->isValid()) return (ic);
        }
    }

    // Lowest priority:  the default icon.  mediaType() was checked at
    // the start and so we know that it must be TrackData::WaypointNormal
    // here.  In this case TrackDataWaypoint::iconName() will always return
    // the default "favorites".
    return (TrackDataItem::icon());
}


// Merging waypoints, compatibility and automatic merge
//
// based on NavMarks PointData::canMerge() and PointData::mergeWith()

QStringList TrackDataWaypoint::formattedAddress() const
{
    return (TrackData::formattedAddress(metadata("streetaddress"), metadata("city"),
                                        metadata("state"), metadata("postalcode"),
                                        metadata("country")));
}


#define LATLONGTOL              (5.0/(60*60*10))	// 0.5 seconds of angle,
							// about 15 metres at equator
#define ELEVTOL			(1.0)			// 1 metre
#define NAMEMIN			10			// minimum for prefix match
#define WAYPOINT		"Waypoint"		// default symbol name

static const QList<QByteArray> symbolTags = { "sym", "icon", "symset" };


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
//    Symbol/icon/set	either match exactly, or one is blank
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
            qDebug() << "can't merge - NAME";
#endif
            return (false);
        }
    }

    // Symbol, icon and set
    for (const QByteArray &tag : std::as_const(symbolTags))
    {
        const QVariant &s1 = this->metadata(tag);
        const QVariant &s2 = other->metadata(tag);
        if (s1!=s2)
        {
            // This also checks SYMSET for the default waypoint name,
            // but that value is not likely to be seen in a file.
            if (symbolIsValid(s1) && symbolIsValid(s2))
            {
#ifdef DEBUG_MERGE
                qDebug() << "can't merge -" << tag.toUpper() << s2 << s1;
#endif
                return (false);
            }
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
            qDebug() << "can't merge - ELE" << e2 << e1;
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

    // Symbol/icon/set - accept the first unless that is the default WAYPOINT
    // or blank, in which case use the other.  Treat each tag individually,
    // do not do any consistency checks or reconciliation of the values together.
    for (const QByteArray &tag : std::as_const(symbolTags))
    {
        if (!symbolIsValid(this->metadata(tag))) setMetadata(tag, other->metadata(tag));
    }

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

        this->setMetadata("streetaddress", other->metadata("streetaddress"));
        this->setMetadata("city", other->metadata("city"));
        this->setMetadata("state", other->metadata("state"));
        this->setMetadata("postalcode", other->metadata("postalcode"));
        this->setMetadata("country", other->metadata("country"));
    }

    // Categories - merge the two lists.
    const QStringList &c1 = this->metadata("category").toStringList();
    const QStringList &c2 = other->metadata("category").toStringList();
    if (!c2.isEmpty())					// if there is something to merge
    {
        QStringList res = c1;
        for (const QString &c : std::as_const(c2))
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
        if (name=="ele" || name=="sym" || name=="streetaddress" ||
            name=="city" || name=="state" || name=="postalcode" ||
            name=="country" || name=="category" || name=="origin" ||
            name=="flags" || name=="symset") continue;

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
    : TrackDataContainer("route_%02d", &counterRoute)
{
#ifdef MEMORY_TRACKING
    ++allocRoute;
#endif
}


QString TrackDataRoute::selectionStatus(int num) const
{
    if (num==1) return (i18n("Selected route '%1'", name()));
    else return (i18np("Selected %1 route", "Selected %1 routes", num));
}


QString TrackDataRoute::toolTip() const
{
    return (i18np("Route with %1 point", "Route with %1 points", childCount()));
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


QString TrackDataRoutepoint::selectionStatus(int num) const
{
    if (num==1) return (i18n("Selected routepoint '%1'", name()));
    else return (i18np("Selected %1 routepoint", "Selected %1 routepoints", num));
}


QString TrackDataRoutepoint::toolTip() const
{
    // Elevation is rare for routepoints, with no GUI to set it.
    return (i18n("Routepoint"));
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
