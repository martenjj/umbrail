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

#ifndef TRACKDATA_H
#define TRACKDATA_H

#include <math.h>

#include <qvariant.h>
#include <qlist.h>
#include <qdatetime.h>
#include <qvector.h>
#include <qurl.h>

#define ISNAN(x)		std::isnan(x)		// to cover variations

#define DEGREES_TO_RADIANS(x)	(((x)*2*M_PI)/360)	// angle conversion
#define RADIANS_TO_DEGREES(x)	(((x)*360)/(2*M_PI))


class QWidget;
class QTimeZone;
class QIcon;
class TrackDataItem;
class TrackDataFolder;
class TrackPropertiesPage;

//////////////////////////////////////////////////////////////////////////
//									//
//  TrackPropertiesInterface						//
//									//
//////////////////////////////////////////////////////////////////////////

#define DEFINE_PROPERTIES_PAGE_INTERFACE(PAGETYPE)		\
    virtual TrackPropertiesPage *				\
        createProperties ## PAGETYPE ## Page(			\
            const QList<TrackDataItem *> *items,		\
            QWidget *pnt = nullptr) const = 0;

#define DEFINE_PROPERTIES_PAGE(PAGETYPE)			\
    TrackPropertiesPage *					\
        createProperties ## PAGETYPE ## Page(			\
            const QList<TrackDataItem *> *items,		\
            QWidget *pnt = nullptr) const override;

// TODO: optional ones non-pure with default null implementation
class TrackPropertiesInterface
{
public:
    virtual ~TrackPropertiesInterface() = default;
    DEFINE_PROPERTIES_PAGE_INTERFACE(General)
    DEFINE_PROPERTIES_PAGE_INTERFACE(Detail)
    DEFINE_PROPERTIES_PAGE_INTERFACE(Style)
    DEFINE_PROPERTIES_PAGE_INTERFACE(Plot)
    DEFINE_PROPERTIES_PAGE_INTERFACE(Metadata)
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

    QDateTime start() const				{ return (mStart); }
    QDateTime finish() const				{ return (mFinish); }
    bool isValid() const				{ return (mStart.isValid() && mFinish.isValid()); }
    unsigned timeSpan() const				{ return (mStart.secsTo(mFinish)); }

    TimeRange united(const TimeRange &other) const;

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

    double north() const				{ return (mLatNorth); }
    double south() const				{ return (mLatSouth); }
    double east() const					{ return (mLonEast); }
    double west() const					{ return (mLonWest); }
    bool isValid() const				{ return (!ISNAN(mLatNorth) && !ISNAN(mLonWest)); }

    BoundingArea united(const BoundingArea &other) const;

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
        Route,
        Segment,
        Trackpoint,
        Folder,
        Waypoint,
        Routepoint
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
        StatusQuestion,
        StatusUnwanted,
    };

    BoundingArea unifyBoundingAreas(const QList<TrackDataItem *> *items);
    TimeRange unifyTimeSpans(const QList<TrackDataItem *> *items);
    unsigned sumTotalChildCount(const QList<TrackDataItem *> *items);

    QString formattedLatLong(double lat, double lon, bool blankIfUnknown = false);
    QString formattedDuration(unsigned t, bool blankIfZero = false);
    QString formattedTime(const QDateTime &dt, const QTimeZone *tz = nullptr);
    QString formattedWaypointStatus(TrackData::WaypointStatus status, bool blankForNone = false);

    /**
     * Find a folder by name or path.
     *
     * @param path Path of the folder to find, names separated by '/'
     * @param item Root item to start path search from
     * @return The specified folder if it exists, otherwise, @c nullptr
     **/
    TrackDataFolder *findFolderByPath(const QString &path, const TrackDataItem *root);

    QVariant valueOrNull(const QVariant &value);
}

//////////////////////////////////////////////////////////////////////////
//									//
//  TrackDataItem							//
//									//
//////////////////////////////////////////////////////////////////////////

class TrackDataItem
{
public:
    virtual ~TrackDataItem();

    virtual TrackData::Type type() const = 0;

    QString name() const				{ return (mName); }
    void setName(const QString &newName, bool explicitName);
    bool hasExplicitName() const			{ return (mExplicitName); }

    virtual QIcon icon() const;

    int childCount() const				{ return (mChildren==nullptr ? 0 : mChildren->count()); }
    TrackDataItem *childAt(int idx) const		{ Q_ASSERT(mChildren!=nullptr); return (mChildren->at(idx)); }
    int childIndex(const TrackDataItem *data) const	{ Q_ASSERT(mChildren!=nullptr); return (mChildren->indexOf(const_cast<TrackDataItem *>(data))); }
    TrackDataItem *parent() const			{ return (mParent); }

    void addChildItem(TrackDataItem *data, int idx = -1);
    TrackDataItem *takeFirstChildItem();
    TrackDataItem *takeLastChildItem();
    TrackDataItem *takeChildItem(int idx);
    void removeChildItem(TrackDataItem *item);

    unsigned long selectionId() const			{ return (mSelectionId); }
    void setSelectionId(unsigned long id)		{ mSelectionId = id; }

    QVariant metadata(int idx) const;
    QVariant metadata(const QByteArray &key) const;
    void setMetadata(int idx, const QVariant &value);
    void setMetadata(const QByteArray &key, const QVariant &value);
    void copyMetadata(const TrackDataItem *other, bool overwrite = false);

    virtual BoundingArea boundingArea() const;
    virtual TimeRange timeSpan() const;
    QString timeZone() const;

protected:
    TrackDataItem(const char *format = nullptr, int *counter = nullptr);

    virtual QString iconName() const = 0;

private:
    TrackDataItem(const TrackDataItem &other) = delete;
    TrackDataItem &operator=(const TrackDataItem &other) = delete;

    void init();

    QString mName;
    bool mExplicitName;
    QList<TrackDataItem *> *mChildren;
    QVector<QVariant> *mMetadata;
    TrackDataItem *mParent;
    unsigned long mSelectionId;
    // Style *mStyle;
};

//////////////////////////////////////////////////////////////////////////
//									//
//  TrackDataFile							//
//									//
//////////////////////////////////////////////////////////////////////////

class TrackDataFile : public TrackDataItem, public TrackPropertiesInterface
{
public:
    explicit TrackDataFile();
    virtual ~TrackDataFile() = default;

    TrackData::Type type() const override		{ return (TrackData::File); }

    QUrl fileName() const				{ return (mFileName); }
    void setFileName(const QUrl &file);

    DEFINE_PROPERTIES_PAGE(General)
    DEFINE_PROPERTIES_PAGE(Detail)
    DEFINE_PROPERTIES_PAGE(Style)
    DEFINE_PROPERTIES_PAGE(Plot)
    DEFINE_PROPERTIES_PAGE(Metadata)

protected:
    QString iconName() const override;

private:
    QUrl mFileName;
};

//////////////////////////////////////////////////////////////////////////
//									//
//  TrackDataTrack							//
//									//
//////////////////////////////////////////////////////////////////////////

class TrackDataTrack : public TrackDataItem, public TrackPropertiesInterface
{
public:
    explicit TrackDataTrack();
    virtual ~TrackDataTrack() = default;

    TrackData::Type type() const override		{ return (TrackData::Track); }

    DEFINE_PROPERTIES_PAGE(General)
    DEFINE_PROPERTIES_PAGE(Detail)
    DEFINE_PROPERTIES_PAGE(Style)
    DEFINE_PROPERTIES_PAGE(Plot)
    DEFINE_PROPERTIES_PAGE(Metadata)

protected:
    QString iconName() const override			{ return ("chart_track"); }
};

//////////////////////////////////////////////////////////////////////////
//									//
//  TrackDataSegment							//
//									//
//////////////////////////////////////////////////////////////////////////

class TrackDataSegment : public TrackDataItem, public TrackPropertiesInterface
{
public:
    explicit TrackDataSegment();
    virtual ~TrackDataSegment() = default;

    TrackData::Type type() const override		{ return (TrackData::Segment); }

    DEFINE_PROPERTIES_PAGE(General)
    DEFINE_PROPERTIES_PAGE(Detail)
    DEFINE_PROPERTIES_PAGE(Style)
    DEFINE_PROPERTIES_PAGE(Plot)
    DEFINE_PROPERTIES_PAGE(Metadata)

    TimeRange timeSpan() const override;

protected:
    QString iconName() const override			{ return ("chart_segment"); }
};

//////////////////////////////////////////////////////////////////////////
//									//
//  TrackDataFolder							//
//									//
//////////////////////////////////////////////////////////////////////////

class TrackDataFolder : public TrackDataItem, public TrackPropertiesInterface
{
public:
    explicit TrackDataFolder();
    virtual ~TrackDataFolder() = default;

    TrackData::Type type() const override		{ return (TrackData::Folder); }

    DEFINE_PROPERTIES_PAGE(General)
    DEFINE_PROPERTIES_PAGE(Detail)
    DEFINE_PROPERTIES_PAGE(Style)
    DEFINE_PROPERTIES_PAGE(Plot)
    DEFINE_PROPERTIES_PAGE(Metadata)

    QString path() const;

protected:
    QString iconName() const override			{ return ("folder-favorites"); }
};

//////////////////////////////////////////////////////////////////////////
//									//
//  TrackDataAbstractPoint						//
//									//
//////////////////////////////////////////////////////////////////////////

class TrackDataAbstractPoint : public TrackDataItem
{
public:
    TrackDataAbstractPoint(const char *format, int *counter);
    virtual ~TrackDataAbstractPoint() = default;

    void setLatLong(double lat, double lon)		{ mLatitude = lat; mLongitude = lon; }

    double elevation() const;
    QDateTime time() const;
    double latitude() const				{ return (mLatitude); }
    double longitude() const				{ return (mLongitude); }

    QString formattedElevation() const;
    QString formattedTime(bool withZone = false) const;
    QString formattedPosition() const;

    BoundingArea boundingArea() const override;
    TimeRange timeSpan() const override;
    double distanceTo(const TrackDataAbstractPoint *other, bool accurate = false) const;
    double distanceTo(double lat, double lon, bool accurate = false) const;
    double bearingTo(const TrackDataAbstractPoint *other) const;
    int timeTo(const TrackDataAbstractPoint *other) const;

private:
    double mLatitude;
    double mLongitude;
};

//////////////////////////////////////////////////////////////////////////
//									//
//  TrackDataTrackpoint							//
//									//
//////////////////////////////////////////////////////////////////////////

class TrackDataTrackpoint : public TrackDataAbstractPoint, public TrackPropertiesInterface
{
public:
    explicit TrackDataTrackpoint();
    virtual ~TrackDataTrackpoint() = default;

    TrackData::Type type() const override		{ return (TrackData::Trackpoint); }

    DEFINE_PROPERTIES_PAGE(General)
    DEFINE_PROPERTIES_PAGE(Detail)
    DEFINE_PROPERTIES_PAGE(Style)
    DEFINE_PROPERTIES_PAGE(Plot)
    DEFINE_PROPERTIES_PAGE(Metadata)

protected:
    QString iconName() const override			{ return ("chart_point"); }
};

//////////////////////////////////////////////////////////////////////////
//									//
//  TrackDataWaypoint							//
//									//
//////////////////////////////////////////////////////////////////////////

class TrackDataWaypoint : public TrackDataAbstractPoint, public TrackPropertiesInterface
{
public:
    explicit TrackDataWaypoint();
    virtual ~TrackDataWaypoint() = default;

    TrackData::Type type() const override		{ return (TrackData::Waypoint); }

    QIcon icon() const override;

    TrackData::WaypointType waypointType() const;
    bool isMediaType() const;

    DEFINE_PROPERTIES_PAGE(General)
    DEFINE_PROPERTIES_PAGE(Detail)
    DEFINE_PROPERTIES_PAGE(Style)
    DEFINE_PROPERTIES_PAGE(Plot)
    DEFINE_PROPERTIES_PAGE(Metadata)

protected:
    QString iconName() const override;
};

//////////////////////////////////////////////////////////////////////////
//									//
//  TrackDataRoute							//
//									//
//////////////////////////////////////////////////////////////////////////

class TrackDataRoute : public TrackDataItem, public TrackPropertiesInterface
{
public:
    explicit TrackDataRoute();
    virtual ~TrackDataRoute() = default;

    TrackData::Type type() const override		{ return (TrackData::Route); }

    QString iconName() const override			{ return ("chart_route"); }

    DEFINE_PROPERTIES_PAGE(General)
    DEFINE_PROPERTIES_PAGE(Detail)
    DEFINE_PROPERTIES_PAGE(Style)
    DEFINE_PROPERTIES_PAGE(Plot)
    DEFINE_PROPERTIES_PAGE(Metadata)
};

//////////////////////////////////////////////////////////////////////////
//									//
//  TrackDataRoutepoint							//
//									//
//////////////////////////////////////////////////////////////////////////

class TrackDataRoutepoint : public TrackDataAbstractPoint, public TrackPropertiesInterface
{
public:
    explicit TrackDataRoutepoint();
    virtual ~TrackDataRoutepoint() = default;

    TrackData::Type type() const override		{ return (TrackData::Routepoint); }

    // There is a "chart_routepoint" icon (present for completeness),
    // but the flag looks better on the map and plot.  So use it in the
    // data model also.
    QString iconName() const override			{ return ("flag"); }

    DEFINE_PROPERTIES_PAGE(General)
    DEFINE_PROPERTIES_PAGE(Detail)
    DEFINE_PROPERTIES_PAGE(Style)
    DEFINE_PROPERTIES_PAGE(Plot)
    DEFINE_PROPERTIES_PAGE(Metadata)
};

#endif							// TRACKDATA_H
