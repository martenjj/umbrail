
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

private:
    QString mName;
    QList<TrackDataItem *> mChildItems;
    TrackDataItem *mParent;
};







class TrackDataRoot : public TrackDataItem
{
public:
    TrackDataRoot(const QString &name);
    virtual ~TrackDataRoot()				{}

    QString iconName() const				{ return ("unknown"); }
};








class TrackDataFile : public TrackDataItem
{
public:
    TrackDataFile(const QString &name);
    virtual ~TrackDataFile()				{}

    KUrl fileName() const				{ return (mFileName); }
    void setFileName(const KUrl &file)			{ mFileName = file; }
    QString iconName() const;

private:
    KUrl mFileName;
};






class TrackDataTrack : public TrackDataItem
{
public:
    TrackDataTrack(const QString &name);
    virtual ~TrackDataTrack()				{}

    QString iconName() const				{ return ("chart_track"); }

};



class TrackDataSegment : public TrackDataItem
{
public:
    TrackDataSegment(const QString &name);
    virtual ~TrackDataSegment()				{}

    QString iconName() const				{ return ("chart_segment"); }

};



class TrackDataPoint : public TrackDataItem
{
public:
    TrackDataPoint(const QString &name);
    virtual ~TrackDataPoint()				{}

    QString iconName() const				{ return ("bookmarks"); }

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
    double mLatitude;
    double mLongitude;
    double mElevation;
    QDateTime mDateTime;
};




#endif							// TRACKDATA_H
