

#include "trackdata.h"

#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <kmimetype.h>


static const double DEGREES_TO_RADIANS = 1/57.295;	// multiplier


const TimeRange TimeRange::null = TimeRange();
const BoundingArea BoundingArea::null = BoundingArea();




TimeRange TimeRange::united(const TimeRange &other) const
{
    if (!isValid()) return (other);

    TimeRange result = *this;
    if (!other.isValid()) return (result);

    if (other.mStart<mStart) result.mStart = other.mStart;
    if (other.mFinish>mFinish) result.mFinish = other.mFinish;
    return (result);
}






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






QString TrackData::formattedLatLong(double lat, double lon)
{
    if (isnan(lat) || isnan(lon)) return (i18nc("an unknown quantity", "unknown"));
    QString latStr = toDMS(lat, 2, 'N', 'S');
    QString lonStr = toDMS(lon, 3, 'E', 'W');
    return (latStr+" "+lonStr);
}




QString TrackData::formattedDuration(unsigned t)
{
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



QString TrackData::formattedTime(const QDateTime &dt)
{
    if (!dt.isValid()) return (i18nc("an unknown quantity", "unknown"));
    return (KGlobal::locale()->formatDateTime(dt, KLocale::ShortDate, true));
}





TrackDataItem::TrackDataItem(const QString &desc)
{
    mName = desc;
    init();
}



TrackDataItem::TrackDataItem(const QString &desc, const char *format, int *counter)
{
    if (!desc.isEmpty()) mName = desc;
    else mName.sprintf(format, ++(*counter));
    init();
}



void TrackDataItem::init()
{
    mParent = NULL;
    mSelectionId = 1;
}



TrackDataItem::~TrackDataItem()
{
    qDeleteAll(mChildItems);
}



void TrackDataItem::addChildItem(TrackDataItem *data)
{
    data->mParent = this;				// set item parent
    mChildItems.append(data);
}



TrackDataItem *TrackDataItem::removeLastChildItem()
{
    Q_ASSERT(!mChildItems.isEmpty());
    TrackDataItem *data = mChildItems.takeLast();
    data->mParent = NULL;				// now no longer has parent
    return (data);
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






bool TrackDataItem::setFileModified(bool state)
{
    kDebug() << "to" << state << "for" << name();

    // The 'item' can point anywhere in the data tree.  So search upwards
    // until a suitable TrackDataFile is found, or the root of the tree
    // is reached (which should never happen).

    TrackDataItem *item = this;
    while (item!=NULL)
    {
        TrackDataFile *fileItem = dynamic_cast<TrackDataFile *>(item);
        if (fileItem!=NULL)				// found the file item
        {
            bool wasModified = fileItem->isModified();
            kDebug() << "file" << fileItem->name() << "was mod" << wasModified;
            fileItem->setModified(state);
            return (wasModified);
        }

        item = item->parent();				// back up the tree
    }

    kDebug() << "no ancestor file item!";
    return (false);
}







TrackDataRoot::TrackDataRoot(const QString &desc)
    : TrackDataItem(desc)
{
}





int TrackDataFile::sCounter = 0;

TrackDataFile::TrackDataFile(const QString &desc)
    : TrackDataItem(desc, "file_%02d", &TrackDataFile::sCounter)
{
    mModified = false;
}




QString TrackDataFile::iconName() const
{
    return (KMimeType::iconNameForUrl(mFileName));
}







int TrackDataTrack::sCounter = 0;

TrackDataTrack::TrackDataTrack(const QString &desc)
    : TrackDataItem(desc, "track_%02d", &TrackDataTrack::sCounter)
{
}




int TrackDataSegment::sCounter = 0;

TrackDataSegment::TrackDataSegment(const QString &desc)
    : TrackDataItem(desc, "segment_%02d", &TrackDataSegment::sCounter)
{
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







int TrackDataPoint::sCounter = 0;

TrackDataPoint::TrackDataPoint(const QString &desc)
    : TrackDataItem(desc, "point_%04d", &TrackDataPoint::sCounter)
{
    mLatitude = mLongitude = NAN;
    mElevation = NAN;
}


QString TrackDataPoint::formattedElevation() const
{
    if (isnan(mElevation)) return (i18nc("an unknown quantity", "unknown"));
    return (i18nc("@item:intable Number with unit of metres", "%1 m", QString::number(mElevation, 'f', 1)));
}



QString TrackDataPoint::formattedTime() const
{
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



double TrackDataPoint::distanceTo(const TrackDataPoint *other) const
{
    // See http://www.movable-type.co.uk/scripts/latlong.html
    // Pythagoras is good enough for small distances
    double lat1 = latitude()*DEGREES_TO_RADIANS;
    double lon1 = longitude()*DEGREES_TO_RADIANS;
    double lat2 = other->latitude()*DEGREES_TO_RADIANS;
    double lon2 = other->longitude()*DEGREES_TO_RADIANS;

    double x = (lon2-lon1)*cos((lat1+lat2)/2.0);
    double y = (lat2-lat1);

    return (sqrt(x*x + y*y));
}



int TrackDataPoint::timeTo(const TrackDataPoint *other) const
{
    return (time().secsTo(other->time()));
}

