
#ifndef TRACKDATA_H
#define TRACKDATA_H

#include <qstring.h>
#include <qlist.h>
#include <qdatetime.h>

#include <kurl.h>


class QDateTime;




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
};








class TrackDataItem
{
public:
    TrackDataItem(const QString &desc);
    virtual ~TrackDataItem();

    virtual QString name() const			{ return (mName); }
//    virtual void setDescription(const QString &desc)	{ mDescription = desc; }

    virtual QString iconName() const = 0;

    int childCount() const				{ return (mChildItems.count()); }
    TrackDataItem *childAt(int idx) const		{ return (mChildItems[idx]); }
    int childIndex(const TrackDataItem *data) const	{ return (mChildItems.indexOf(const_cast<TrackDataItem *>(data))); }
    TrackDataItem *parent() const			{ return (mParent); }

    void addChildItem(TrackDataItem *data);
    TrackDataItem *removeLastChildItem();

protected:
    TrackDataItem(const QString &desc, const char *format, int *counter);

private:
    void init();

    QString mName;
    QList<TrackDataItem *> mChildItems;
    TrackDataItem *mParent;
};







class TrackDataRoot : public TrackDataItem
{
public:
    TrackDataRoot(const QString &desc);
    virtual ~TrackDataRoot()				{}

    QString iconName() const				{ return ("unknown"); }
};








class TrackDataFile : public TrackDataItem
{
public:
    TrackDataFile(const QString &desc);
    virtual ~TrackDataFile()				{}

    KUrl fileName() const				{ return (mFileName); }
    void setFileName(const KUrl &file)			{ mFileName = file; }
    QString iconName() const;

private:
    static int sCounter;

    KUrl mFileName;
};






class TrackDataTrack : public TrackDataItem
{
public:
    TrackDataTrack(const QString &desc);
    virtual ~TrackDataTrack()				{}

    QString iconName() const				{ return ("chart_track"); }

private:
    static int sCounter;
};



class TrackDataSegment : public TrackDataItem
{
public:
    TrackDataSegment(const QString &desc);
    virtual ~TrackDataSegment()				{}

    QString iconName() const				{ return ("chart_segment"); }

private:
    static int sCounter;
};



class TrackDataPoint : public TrackDataItem
{
public:
    TrackDataPoint(const QString &desc);
    virtual ~TrackDataPoint()				{}

    QString iconName() const				{ return ("chart_point"); }

    void setLatLong(double lat, double lon)		{ mLatitude = lat; mLongitude = lon; }
    void setElevation(double ele)			{ mElevation = ele; }
    void setTime(const QDateTime &dt)			{ mDateTime = dt; }

    double elevation() const				{ return (mElevation); }
    QDateTime time() const				{ return (mDateTime); }
    double latitude() const				{ return (mLatitude); }
    double longitude() const				{ return (mLongitude); }

    QString formattedElevation() const;
    QString formattedTime() const;

private:
    static int sCounter;

    double mLatitude;
    double mLongitude;
    double mElevation;
    QDateTime mDateTime;
};




#endif							// TRACKDATA_H
