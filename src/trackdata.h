
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
class TrackPropertiesPage;


class TimeRange
{
public:
    TimeRange()					{}
    TimeRange(const QDateTime &sp, const QDateTime &fp)
        : mStart(sp), mFinish(fp)		{}

    ~TimeRange()				{}

    QDateTime start() const			{ return (mStart); }
    QDateTime finish() const			{ return (mFinish); }
    bool isValid() const			{ return (mStart.isValid() && mFinish.isValid()); }
    unsigned timeSpan() const			{ return (mStart.secsTo(mFinish)); }

    TimeRange united(const TimeRange &other) const;

    static const TimeRange null;

private:
    QDateTime mStart;
    QDateTime mFinish;
};





class BoundingArea
{
public:
    BoundingArea()
        : mLatNorth(NAN), mLatSouth(NAN),
          mLonWest(NAN), mLonEast(NAN)		{}
    BoundingArea(double lat, double lon)
        : mLatNorth(lat), mLatSouth(lat),
          mLonWest(lon), mLonEast(lon)		{}
    ~BoundingArea()				{}

    double north() const			{ return (mLatNorth); }
    double south() const			{ return (mLatSouth); }
    double east() const				{ return (mLonEast); }
    double west() const				{ return (mLonWest); }
    bool isValid() const			{ return (!isnan(mLatNorth) && !isnan(mLonWest)); }

    BoundingArea united(const BoundingArea &other) const;

    static const BoundingArea null;

private:
    double mLatNorth;
    double mLatSouth;
    double mLonWest;
    double mLonEast;
};





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
        Point
    };

    BoundingArea unifyBoundingAreas(const QList<TrackDataItem *> &items);
    TimeRange unifyTimeSpans(const QList<TrackDataItem *> &items);

    double sumTotalTravelDistance(const QList<TrackDataItem *> &items);
    unsigned sumTotalTravelTime(const QList<TrackDataItem *> &items);

    QString formattedLatLong(double lat, double lon);
    QString formattedDuration(unsigned t);
    QString formattedTime(const QDateTime &dt, const KTimeZone *tz = NULL);
};







class TrackDataItem
{
public:
    virtual ~TrackDataItem();

    QString name() const				{ return (mName); }
    void setName(const QString &newName)		{ mName = newName; }

    int childCount() const				{ return (mChildItems.count()); }
    TrackDataItem *childAt(int idx) const		{ return (mChildItems[idx]); }
    int childIndex(const TrackDataItem *data) const	{ return (mChildItems.indexOf(const_cast<TrackDataItem *>(data))); }
    TrackDataItem *parent() const			{ return (mParent); }

    void addChildItem(TrackDataItem *data, bool atStart = false);
    TrackDataItem *takeLastChildItem();
    TrackDataItem *takeFirstChildItem();

// TODO: can move to Displayable?
    virtual BoundingArea boundingArea() const;
    virtual TimeRange timeSpan() const;
    virtual double totalTravelDistance() const;
    virtual unsigned int totalTravelTime() const;

protected:
    TrackDataItem(const QString &nm, const char *format = NULL, int *counter = NULL);

private:
    TrackDataItem(const TrackDataItem &other);
    TrackDataItem &operator=(const TrackDataItem &other);

    void init();

    QString mName;
    // TODO: can this list be lazy allocated?
    QList<TrackDataItem *> mChildItems;
    TrackDataItem *mParent;
};






class TrackDataDisplayable : public TrackDataItem
{
public:
    virtual ~TrackDataDisplayable();

    virtual QString iconName() const = 0;

    void setMetadata(int idx, const QString &value);
    QString metadata(int idx) const;
    QString metadata(const QString &key) const;
    void copyMetadata(const TrackDataDisplayable *other, bool overwrite = false);

    unsigned long selectionId() const			{ return (mSelectionId); }
    void setSelectionId(unsigned long id)		{ mSelectionId = id; }

    const Style *style() const;
    void setStyle(const Style &s);

    QString timeZone() const;

    virtual TrackPropertiesPage *createPropertiesGeneralPage(const QList<TrackDataItem *> items, QWidget *pnt = NULL) = 0;
    virtual TrackPropertiesPage *createPropertiesDetailPage(const QList<TrackDataItem *> items, QWidget *pnt = NULL) = 0;
    virtual TrackPropertiesPage *createPropertiesStylePage(const QList<TrackDataItem *> items, QWidget *pnt = NULL) = 0;

protected:
    TrackDataDisplayable(const QString &nm, const char *format, int *counter);

private:
    unsigned long mSelectionId;
    // TODO: can this list be lazy allocated?
    QVector<QString> mMetadata;
    Style *mStyle;
};









class TrackDataRoot : public TrackDataItem
{
public:
    TrackDataRoot(const QString &nm);
    virtual ~TrackDataRoot()				{}
};






class TrackDataFile : public TrackDataDisplayable
{
public:
    TrackDataFile(const QString &nm);
    virtual ~TrackDataFile()				{}

    KUrl fileName() const				{ return (mFileName); }
    void setFileName(const KUrl &file)			{ mFileName = file; }
    QString iconName() const;

    TrackPropertiesPage *createPropertiesGeneralPage(const QList<TrackDataItem *> items, QWidget *pnt = NULL);
    TrackPropertiesPage *createPropertiesDetailPage(const QList<TrackDataItem *> items, QWidget *pnt = NULL);
    TrackPropertiesPage *createPropertiesStylePage(const QList<TrackDataItem *> items, QWidget *pnt = NULL);

private:
    static int sCounter;

    KUrl mFileName;
};






class TrackDataTrack : public TrackDataDisplayable
{
public:
    TrackDataTrack(const QString &nm);
    virtual ~TrackDataTrack()				{}

    QString iconName() const				{ return ("chart_track"); }

    TrackPropertiesPage *createPropertiesGeneralPage(const QList<TrackDataItem *> items, QWidget *pnt = NULL);
    TrackPropertiesPage *createPropertiesDetailPage(const QList<TrackDataItem *> items, QWidget *pnt = NULL);
    TrackPropertiesPage *createPropertiesStylePage(const QList<TrackDataItem *> items, QWidget *pnt = NULL);

private:
    static int sCounter;
};



class TrackDataSegment : public TrackDataDisplayable
{
public:
    TrackDataSegment(const QString &nm);
    virtual ~TrackDataSegment()				{}

    QString iconName() const				{ return ("chart_segment"); }

    TrackPropertiesPage *createPropertiesGeneralPage(const QList<TrackDataItem *> items, QWidget *pnt = NULL);
    TrackPropertiesPage *createPropertiesDetailPage(const QList<TrackDataItem *> items, QWidget *pnt = NULL);
    TrackPropertiesPage *createPropertiesStylePage(const QList<TrackDataItem *> items, QWidget *pnt = NULL);

    TimeRange timeSpan() const;

private:
    static int sCounter;
};



class TrackDataPoint : public TrackDataDisplayable
{
public:
    TrackDataPoint(const QString &nm);
    virtual ~TrackDataPoint()				{}

    QString iconName() const				{ return ("chart_point"); }

    TrackPropertiesPage *createPropertiesGeneralPage(const QList<TrackDataItem *> items, QWidget *pnt = NULL);
    TrackPropertiesPage *createPropertiesDetailPage(const QList<TrackDataItem *> items, QWidget *pnt = NULL);
    TrackPropertiesPage *createPropertiesStylePage(const QList<TrackDataItem *> items, QWidget *pnt = NULL);

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
    double distanceTo(const TrackDataPoint *other, bool accurate = false) const;
    double bearingTo(const TrackDataPoint *other) const;
    int timeTo(const TrackDataPoint *other) const;


private:
    static int sCounter;

    double mLatitude;
    double mLongitude;
    double mElevation;
    QDateTime mDateTime;
};




#endif							// TRACKDATA_H
