
#ifndef TRACKDATA_H
#define TRACKDATA_H

#include <math.h>

#include <qstring.h>
#include <qlist.h>
#include <qdatetime.h>

#include <kurl.h>


class Style;
class TrackDataItem;


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

    QString formattedLatLong(double lat, double lon);

    double sumTotalTravelDistance(const QList<TrackDataItem *> &items);

    unsigned sumTotalTravelTime(const QList<TrackDataItem *> &items);

    QString formattedDuration(unsigned t);
    QString formattedTime(const QDateTime &dt);




};







class TrackDataItem
{
public:
    TrackDataItem(const QString &desc);
    virtual ~TrackDataItem();

    QString name() const				{ return (mName); }
    void setName(const QString &newName)		{ mName = newName; }

    virtual QString iconName() const = 0;

    unsigned long selectionId() const			{ return (mSelectionId); }
    void setSelectionId(unsigned long id)		{ mSelectionId = id; }

    int childCount() const				{ return (mChildItems.count()); }
    TrackDataItem *childAt(int idx) const		{ return (mChildItems[idx]); }
    int childIndex(const TrackDataItem *data) const	{ return (mChildItems.indexOf(const_cast<TrackDataItem *>(data))); }
    TrackDataItem *parent() const			{ return (mParent); }

    void addChildItem(TrackDataItem *data);
    TrackDataItem *removeLastChildItem();

    const Style *style() const;
    void setStyle(const Style &s);

    virtual QWidget *createPropertiesGeneralPage(const QList<TrackDataItem *> items, QWidget *pnt = NULL) = 0;
    virtual QWidget *createPropertiesDetailPage(const QList<TrackDataItem *> items, QWidget *pnt = NULL) = 0;
    virtual QWidget *createPropertiesStylePage(const QList<TrackDataItem *> items, QWidget *pnt = NULL) = 0;

    virtual BoundingArea boundingArea() const;
    virtual TimeRange timeSpan() const;
    virtual double totalTravelDistance() const;
    virtual unsigned int totalTravelTime() const;

    bool setFileModified(bool state);

protected:
    TrackDataItem(const QString &desc, const char *format, int *counter);

private:
    TrackDataItem(const TrackDataItem &other);
    TrackDataItem &operator=(const TrackDataItem &other);

    void init();

    QString mName;
    QList<TrackDataItem *> mChildItems;
    TrackDataItem *mParent;
    unsigned long mSelectionId;
    Style *mStyle;
};







class TrackDataRoot : public TrackDataItem
{
public:
    TrackDataRoot(const QString &desc);
    virtual ~TrackDataRoot()				{}

    QString iconName() const				{ return ("unknown"); }

    QWidget *createPropertiesGeneralPage(const QList<TrackDataItem *> items, QWidget *pnt = NULL);
    QWidget *createPropertiesDetailPage(const QList<TrackDataItem *> items, QWidget *pnt = NULL);
    QWidget *createPropertiesStylePage(const QList<TrackDataItem *> items, QWidget *pnt = NULL);

};








class TrackDataFile : public TrackDataItem
{
public:
    TrackDataFile(const QString &desc);
    virtual ~TrackDataFile()				{}

    KUrl fileName() const				{ return (mFileName); }
    void setFileName(const KUrl &file)			{ mFileName = file; }
    QString iconName() const;

    bool isModified() const				{ return (mModified); }
    void setModified(bool state)			{ mModified = state; }

    QWidget *createPropertiesGeneralPage(const QList<TrackDataItem *> items, QWidget *pnt = NULL);
    QWidget *createPropertiesDetailPage(const QList<TrackDataItem *> items, QWidget *pnt = NULL);
    QWidget *createPropertiesStylePage(const QList<TrackDataItem *> items, QWidget *pnt = NULL);

private:
    static int sCounter;

    KUrl mFileName;
    bool mModified;
};






class TrackDataTrack : public TrackDataItem
{
public:
    TrackDataTrack(const QString &desc);
    virtual ~TrackDataTrack()				{}

    QString iconName() const				{ return ("chart_track"); }

    QWidget *createPropertiesGeneralPage(const QList<TrackDataItem *> items, QWidget *pnt = NULL);
    QWidget *createPropertiesDetailPage(const QList<TrackDataItem *> items, QWidget *pnt = NULL);
    QWidget *createPropertiesStylePage(const QList<TrackDataItem *> items, QWidget *pnt = NULL);

private:
    static int sCounter;
};



class TrackDataSegment : public TrackDataItem
{
public:
    TrackDataSegment(const QString &desc);
    virtual ~TrackDataSegment()				{}

    QString iconName() const				{ return ("chart_segment"); }

    QWidget *createPropertiesGeneralPage(const QList<TrackDataItem *> items, QWidget *pnt = NULL);
    QWidget *createPropertiesDetailPage(const QList<TrackDataItem *> items, QWidget *pnt = NULL);
    QWidget *createPropertiesStylePage(const QList<TrackDataItem *> items, QWidget *pnt = NULL);

    TimeRange timeSpan() const;

private:
    static int sCounter;
};



class TrackDataPoint : public TrackDataItem
{
public:
    TrackDataPoint(const QString &desc);
    virtual ~TrackDataPoint()				{}

    QString iconName() const				{ return ("chart_point"); }

    QWidget *createPropertiesGeneralPage(const QList<TrackDataItem *> items, QWidget *pnt = NULL);
    QWidget *createPropertiesDetailPage(const QList<TrackDataItem *> items, QWidget *pnt = NULL);
    QWidget *createPropertiesStylePage(const QList<TrackDataItem *> items, QWidget *pnt = NULL);

    void setLatLong(double lat, double lon)		{ mLatitude = lat; mLongitude = lon; }
    void setElevation(double ele)			{ mElevation = ele; }
    void setTime(const QDateTime &dt)			{ mDateTime = dt; }

    void setHdop(const QString &h)			{ mHdop = h; }
    void setSpeed(const QString &s)			{ mSpeed = s; }

    double elevation() const				{ return (mElevation); }
    QDateTime time() const				{ return (mDateTime); }
    double latitude() const				{ return (mLatitude); }
    double longitude() const				{ return (mLongitude); }

    QString hdop() const				{ return (mHdop); }
    QString speed() const				{ return (mSpeed); }

    QString formattedElevation() const;
    QString formattedTime() const;
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
    QString mHdop;
    QString mSpeed;
};




#endif							// TRACKDATA_H
