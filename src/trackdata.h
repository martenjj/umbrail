
#ifndef TRACKDATA_H
#define TRACKDATA_H

#include <math.h>

#include <qstring.h>
#include <qlist.h>
#include <qdatetime.h>
#include <qvector.h>

#include <kurl.h>

class KTimeZone;
class Style;
class TrackDataItem;
class TrackDataFolder;
class TrackPropertiesPage;

//////////////////////////////////////////////////////////////////////////
//									//
//  TrackPropertiesInterface						//
//									//
//////////////////////////////////////////////////////////////////////////

class TrackPropertiesInterface
{
public:
    virtual ~TrackPropertiesInterface()			{}
    virtual TrackPropertiesPage *createPropertiesGeneralPage(const QList<TrackDataItem *> *items, QWidget *pnt = NULL) const = 0;
    virtual TrackPropertiesPage *createPropertiesDetailPage(const QList<TrackDataItem *> *items, QWidget *pnt = NULL) const = 0;
    virtual TrackPropertiesPage *createPropertiesStylePage(const QList<TrackDataItem *> *items, QWidget *pnt = NULL) const = 0;
    virtual TrackPropertiesPage *createPropertiesMetadataPage(const QList<TrackDataItem *> *items, QWidget *pnt = NULL) const = 0;
};

//////////////////////////////////////////////////////////////////////////
//									//
//  TimeRange								//
//									//
//////////////////////////////////////////////////////////////////////////

class TimeRange
{
public:
    TimeRange()						{}
    TimeRange(const QDateTime &sp, const QDateTime &fp)
        : mStart(sp), mFinish(fp)			{}

    ~TimeRange()					{}

    QDateTime start() const				{ return (mStart); }
    QDateTime finish() const				{ return (mFinish); }
    bool isValid() const				{ return (mStart.isValid() && mFinish.isValid()); }
    unsigned timeSpan() const				{ return (mStart.secsTo(mFinish)); }

    TimeRange united(const TimeRange &other) const;

    static const TimeRange null;

private:
    QDateTime mStart;
    QDateTime mFinish;
};

//////////////////////////////////////////////////////////////////////////
//									//
//  BoundingArea							//
//									//
//////////////////////////////////////////////////////////////////////////

class BoundingArea
{
public:
    BoundingArea()
        : mLatNorth(NAN), mLatSouth(NAN),
          mLonWest(NAN), mLonEast(NAN)			{}
    BoundingArea(double lat, double lon)
        : mLatNorth(lat), mLatSouth(lat),
          mLonWest(lon), mLonEast(lon)			{}
    ~BoundingArea()					{}

    double north() const				{ return (mLatNorth); }
    double south() const				{ return (mLatSouth); }
    double east() const					{ return (mLonEast); }
    double west() const					{ return (mLonWest); }
    bool isValid() const				{ return (!isnan(mLatNorth) && !isnan(mLonWest)); }

    BoundingArea united(const BoundingArea &other) const;

    static const BoundingArea null;

private:
    double mLatNorth;
    double mLatSouth;
    double mLonWest;
    double mLonEast;
};

//////////////////////////////////////////////////////////////////////////
//									//
//  TrackData								//
//									//
//////////////////////////////////////////////////////////////////////////

namespace TrackData
{
    // These types are only used for classifying the tree view selection.
    // They are not stored in the individual data classes, we rely on
    // RTTI to distinguish between them.
    enum Type
    {
        None,
        Mixed,
        File,
        Track,
        Segment,
        Point,
        Folder,
        Waypoint
    };

    // Finer grained classification for waypoints,
    // accessed by TrackDataWaypoint::waypointType()
    enum WaypointType
    {
        WaypointNormal,
        WaypointAudioNote,
        WaypointVideoNote,
        WaypointPhoto,
        WaypointStop,
        WaypointAny
    };

    // User status for waypoints
    enum WaypointStatus
    {
        StatusInvalid = -1,
        StatusNone = 0,
        StatusTodo,
        StatusDone,
        StatusQuestion
    };

    BoundingArea unifyBoundingAreas(const QList<TrackDataItem *> *items);
    TimeRange unifyTimeSpans(const QList<TrackDataItem *> *items);
    double sumTotalTravelDistance(const QList<TrackDataItem *> *items, bool tracksOnly = true);
    unsigned sumTotalTravelTime(const QList<TrackDataItem *> *items);
    unsigned sumTotalChildCount(const QList<TrackDataItem *> *items);

    QString formattedLatLong(double lat, double lon, bool blankIfUnknown = false);
    QString formattedDuration(unsigned t, bool blankIfZero = false);
    QString formattedTime(const QDateTime &dt, const KTimeZone *tz = NULL);

    /**
     * Find a child folder under a specified parent.
     *
     * @param name Name of the folder to find.
     * @param pnt The parent or root folder to search under.
     * @param findOne If no matching folder is found, then return a
     * single one (see below).
     *
     * @return If the named folder was found then it is returned.
     * If the named folder was not found but only a single folder exists,
     * then it is returned if @p findOne is @c true.  Otherwise, @c NULL
     * is returned.
     **/
    TrackDataFolder *findChildFolder(const QString &name, const TrackDataItem *pnt, bool findOne = false);
};

//////////////////////////////////////////////////////////////////////////
//									//
//  TrackDataItem							//
//									//
//////////////////////////////////////////////////////////////////////////

class TrackDataItem
{
public:
    virtual ~TrackDataItem();

    QString name() const				{ return (mName); }
    void setName(const QString &newName)		{ mName = newName; }

    virtual QString iconName() const = 0;

    int childCount() const				{ return (mChildren==NULL ? 0 : mChildren->count()); }
    TrackDataItem *childAt(int idx) const		{ Q_ASSERT(mChildren!=NULL); return (mChildren->at(idx)); }
    int childIndex(const TrackDataItem *data) const	{ Q_ASSERT(mChildren!=NULL); return (mChildren->indexOf(const_cast<TrackDataItem *>(data))); }
    TrackDataItem *parent() const			{ return (mParent); }

    void addChildItem(TrackDataItem *data, int idx = -1);
    TrackDataItem *takeFirstChildItem();
    TrackDataItem *takeLastChildItem();
    TrackDataItem *takeChildItem(int idx);
    void removeChildItem(TrackDataItem *item);

    unsigned long selectionId() const			{ return (mSelectionId); }
    void setSelectionId(unsigned long id)		{ mSelectionId = id; }

    const Style *style() const;
    void setStyle(const Style &s);

    void setMetadata(int idx, const QString &value);
    QString metadata(int idx) const;
    QString metadata(const QString &key) const;
    void copyMetadata(const TrackDataItem *other, bool overwrite = false);

    virtual BoundingArea boundingArea() const;
    virtual TimeRange timeSpan() const;
    virtual double totalTravelDistance() const;
    virtual unsigned int totalTravelTime() const;
    QString timeZone() const;

protected:
    TrackDataItem(const QString &nm, const char *format = NULL, int *counter = NULL);

private:
    TrackDataItem(const TrackDataItem &other);
    TrackDataItem &operator=(const TrackDataItem &other);

    void init();

    QString mName;
    QList<TrackDataItem *> *mChildren;
    QVector<QString> *mMetadata;
    TrackDataItem *mParent;
    unsigned long mSelectionId;
    Style *mStyle;
};

//////////////////////////////////////////////////////////////////////////
//									//
//  TrackDataFile							//
//									//
//////////////////////////////////////////////////////////////////////////

class TrackDataFile : public TrackDataItem, public TrackPropertiesInterface
{
public:
    TrackDataFile(const QString &nm);
    virtual ~TrackDataFile()				{}

    KUrl fileName() const				{ return (mFileName); }
    void setFileName(const KUrl &file)			{ mFileName = file; }
    QString iconName() const;

    TrackPropertiesPage *createPropertiesGeneralPage(const QList<TrackDataItem *> *items, QWidget *pnt = NULL) const;
    TrackPropertiesPage *createPropertiesDetailPage(const QList<TrackDataItem *> *items, QWidget *pnt = NULL) const;
    TrackPropertiesPage *createPropertiesStylePage(const QList<TrackDataItem *> *items, QWidget *pnt = NULL) const;
    TrackPropertiesPage *createPropertiesMetadataPage(const QList<TrackDataItem *> *items, QWidget *pnt = NULL) const;

private:
    KUrl mFileName;
};

//////////////////////////////////////////////////////////////////////////
//									//
//  TrackDataTrack							//
//									//
//////////////////////////////////////////////////////////////////////////

class TrackDataTrack : public TrackDataItem, public TrackPropertiesInterface
{
public:
    TrackDataTrack(const QString &nm);
    virtual ~TrackDataTrack()				{}

    QString iconName() const				{ return ("chart_track"); }

    TrackPropertiesPage *createPropertiesGeneralPage(const QList<TrackDataItem *> *items, QWidget *pnt = NULL) const;
    TrackPropertiesPage *createPropertiesDetailPage(const QList<TrackDataItem *> *items, QWidget *pnt = NULL) const;
    TrackPropertiesPage *createPropertiesStylePage(const QList<TrackDataItem *> *items, QWidget *pnt = NULL) const;
    TrackPropertiesPage *createPropertiesMetadataPage(const QList<TrackDataItem *> *items, QWidget *pnt = NULL) const;
};

//////////////////////////////////////////////////////////////////////////
//									//
//  TrackDataSegment							//
//									//
//////////////////////////////////////////////////////////////////////////

class TrackDataSegment : public TrackDataItem, public TrackPropertiesInterface
{
public:
    TrackDataSegment(const QString &nm);
    virtual ~TrackDataSegment()				{}

    QString iconName() const				{ return ("chart_segment"); }

    TrackPropertiesPage *createPropertiesGeneralPage(const QList<TrackDataItem *> *items, QWidget *pnt = NULL) const;
    TrackPropertiesPage *createPropertiesDetailPage(const QList<TrackDataItem *> *items, QWidget *pnt = NULL) const;
    TrackPropertiesPage *createPropertiesStylePage(const QList<TrackDataItem *> *items, QWidget *pnt = NULL) const;
    TrackPropertiesPage *createPropertiesMetadataPage(const QList<TrackDataItem *> *items, QWidget *pnt = NULL) const;

    TimeRange timeSpan() const;
};

//////////////////////////////////////////////////////////////////////////
//									//
//  TrackDataFolder							//
//									//
//////////////////////////////////////////////////////////////////////////

class TrackDataFolder : public TrackDataItem, public TrackPropertiesInterface
{
public:
    TrackDataFolder(const QString &nm);
    virtual ~TrackDataFolder()				{}

    QString iconName() const				{ return ("folder-favorites"); }

    TrackPropertiesPage *createPropertiesGeneralPage(const QList<TrackDataItem *> *items, QWidget *pnt = NULL) const;
    TrackPropertiesPage *createPropertiesDetailPage(const QList<TrackDataItem *> *items, QWidget *pnt = NULL) const;
    TrackPropertiesPage *createPropertiesStylePage(const QList<TrackDataItem *> *items, QWidget *pnt = NULL) const;
    TrackPropertiesPage *createPropertiesMetadataPage(const QList<TrackDataItem *> *items, QWidget *pnt = NULL) const;

    QString path() const;
};

//////////////////////////////////////////////////////////////////////////
//									//
//  TrackDataAbstractPoint						//
//									//
//////////////////////////////////////////////////////////////////////////

class TrackDataAbstractPoint : public TrackDataItem
{
public:
    TrackDataAbstractPoint(const QString &nm, const char *format, int *counter);
    virtual ~TrackDataAbstractPoint()			{}

    void setLatLong(double lat, double lon)		{ mLatitude = lat; mLongitude = lon; }
    void setElevation(double ele)			{ mElevation = ele; }
    void setTime(const QDateTime &dt)			{ mDateTime = dt; }

    double elevation() const				{ return (mElevation); }
    QDateTime time() const				{ return (mDateTime); }
    double latitude() const				{ return (mLatitude); }
    double longitude() const				{ return (mLongitude); }

    QString formattedElevation() const;
    QString formattedTime(bool withZone = false) const;
    QString formattedPosition() const;

    BoundingArea boundingArea() const;
    TimeRange timeSpan() const;
    double distanceTo(const TrackDataAbstractPoint *other, bool accurate = false) const;
    double distanceTo(double lat, double lon, bool accurate = false) const;
    double bearingTo(const TrackDataAbstractPoint *other) const;
    int timeTo(const TrackDataAbstractPoint *other) const;

    void copyData(const TrackDataAbstractPoint *other);

private:
    double mLatitude;
    double mLongitude;
    double mElevation;
    QDateTime mDateTime;
};

//////////////////////////////////////////////////////////////////////////
//									//
//  TrackDataTrackpoint							//
//									//
//////////////////////////////////////////////////////////////////////////

class TrackDataTrackpoint : public TrackDataAbstractPoint, public TrackPropertiesInterface
{
public:
    TrackDataTrackpoint(const QString &nm);
    virtual ~TrackDataTrackpoint()			{}

    QString iconName() const				{ return ("chart_point"); }

    TrackPropertiesPage *createPropertiesGeneralPage(const QList<TrackDataItem *> *items, QWidget *pnt = NULL) const;
    TrackPropertiesPage *createPropertiesDetailPage(const QList<TrackDataItem *> *items, QWidget *pnt = NULL) const;
    TrackPropertiesPage *createPropertiesStylePage(const QList<TrackDataItem *> *items, QWidget *pnt = NULL) const;
    TrackPropertiesPage *createPropertiesMetadataPage(const QList<TrackDataItem *> *items, QWidget *pnt = NULL) const;
};

//////////////////////////////////////////////////////////////////////////
//									//
//  TrackDataWaypoint							//
//									//
//////////////////////////////////////////////////////////////////////////

class TrackDataWaypoint : public TrackDataAbstractPoint, public TrackPropertiesInterface
{
public:
    TrackDataWaypoint(const QString &nm);
    virtual ~TrackDataWaypoint()			{}

    QString iconName() const;
    TrackData::WaypointType waypointType() const;

    TrackPropertiesPage *createPropertiesGeneralPage(const QList<TrackDataItem *> *items, QWidget *pnt = NULL) const;
    TrackPropertiesPage *createPropertiesDetailPage(const QList<TrackDataItem *> *items, QWidget *pnt = NULL) const;
    TrackPropertiesPage *createPropertiesStylePage(const QList<TrackDataItem *> *items, QWidget *pnt = NULL) const;
    TrackPropertiesPage *createPropertiesMetadataPage(const QList<TrackDataItem *> *items, QWidget *pnt = NULL) const;
};

#endif							// TRACKDATA_H
