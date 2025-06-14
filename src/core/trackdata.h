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
#include <qcolor.h>

#define ISNAN(x)		std::isnan(x)		// to cover variations

#define DEGREES_TO_RADIANS(x)	(((x)*2*M_PI)/360)	// angle conversion
#define RADIANS_TO_DEGREES(x)	(((x)*360)/(2*M_PI))


class QWidget;
class QTimeZone;
class TrackDataItem;
class TrackDataFile;
class TrackDataFolder;
class TrackDataContainer;
class TrackPropertiesPage;
class PointIcon;
class CategoryList;

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
    // accessed by TrackDataItem::mediaType()
    enum MediaType
    {
        MediaNormal,
        MediaAudioNote,
        MediaVideoNote,
        MediaPhoto,
        MediaStop,
        MediaAny
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

    // User flags for waypoints, from navmarks/src/pointdata.h
    enum WaypointFlag
    {
        NoFlags = 0x00,
        HomePoint = 0x01,
        NoExport = 0x02,
        NewlyImported = 0x04,
    };
    Q_DECLARE_FLAGS(WaypointFlags, WaypointFlag)

    BoundingArea unifyBoundingAreas(const QList<TrackDataItem *> *items);
    TimeRange unifyTimeSpans(const QList<TrackDataItem *> *items);
    unsigned sumTotalChildCount(const QList<TrackDataItem *> *items);

    QString formattedLatLong(double lat, double lon, bool blankIfUnknown = false);
    QString formattedDuration(unsigned t, bool blankIfZero = false);
    QString formattedTime(const QDateTime &dt, const QTimeZone *tz = nullptr);
    QString formattedWaypointStatus(TrackData::WaypointStatus status, bool blankForNone = false);
    QString iconForWaypointStatus(TrackData::WaypointStatus status);

    QStringList formattedAddress(const QVariant &street, const QVariant &city,
                                 const QVariant &state, const QVariant &pcode,
                                 const QVariant &cntry);
    /**
     * Find a folder by name or path.
     *
     * @param path Path of the folder to find, names separated by '/'
     * @param root Root item to start path search from
     * @return The specified folder if it exists, otherwise @c NULL
     **/
    TrackDataFolder *findFolderByPath(const QString &path, const TrackDataContainer *root);

    QVariant valueOrNull(const QVariant &v);

    //  The state of an item's colour data - colour and inherit flag - needs to be
    //  able to be stored in a single metadata item.  Since we do not support
    //  alpha blending of item colours, the alpha component of the colour value
    //  is used to encode the inherit flag, with 255 meaning a colour to be used
    //  and 254 meaning inherit.  This is stored and maintained in the item
    //  metadata and GPX files so that the colour persists throughout, except in
    //  some special cases where only the RGB is used for compatibility.  Any use
    //  of the colour should test for validity using TrackData::colourUnlessInherit().
    //
    //  By experimentation:  QColor::fromString("#234567").alpha() = 255
    //                       QColor::fromString("#FE234567").alpha() = 254
    //                       QColor::fromString("#FF234567").alpha() = 255
    //                       QColor().alpha() = 255
    //
    //  The inherit flag is encoded in this way, instead of setting the colour value
    //  to an invalid QColor, so that the RGB value is not lost when the inherit flag
    //  is toggled.
    //
    // The overload taking a QColor must appear before the one taking a QVariant,
    // otherwise there is a compile (GCC 14) and runtime error:
    //
    //   src/core/trackdata.h:221: warning: infinite recursion detected
    //   src/core/trackdata.h:221: note: recursive call
    //
    inline QColor colourUnlessInherit(const QColor &c)		{ return (c.alpha()==255 ? c : QColor()); }
    inline QColor colourUnlessInherit(const QVariant &v)	{ return (colourUnlessInherit(v.value<QColor>())); }
}

//////////////////////////////////////////////////////////////////////////
//									//
//  TrackDataItem							//
//									//
//  This is an abstract data item that simply has a name, parent and	//
//  metadata.								//
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

    virtual const PointIcon *icon() const;

    // Only a TrackDataContainer can have children, but any
    // type of item can have a parent.
    TrackDataContainer *parent() const			{ return (mParent); }

    // Return a status message indicating that one or more of this item type
    // is selected by the user in the application's GUI.  It is displayed in
    // the status bar by FilesController::slotUpdateActionState().  A mixed
    // selection is handled specially there and will never be seen here.
    virtual QString statusMessage(int num) const = 0;

    // Return a tool tip for the hovered item in the application's GUI.
    // This only ever applies to a single item.
    virtual QString toolTip() const = 0;

    unsigned long selectionId() const			{ return (mSelectionId); }
    void setSelectionId(unsigned long id)		{ mSelectionId = id; }

    const TrackDataFile *root() const;

    QVariant metadata(int idx) const;
    QVariant metadata(const QByteArray &key) const;
    void setMetadata(int idx, const QVariant &value);
    void setMetadata(const QByteArray &key, const QVariant &value);
    void copyMetadata(const TrackDataItem *other, bool overwrite = false);

    QString timeZone() const;
    TrackData::MediaType mediaType() const;

    virtual BoundingArea boundingArea() const;
    virtual TimeRange timeSpan() const;

protected:
    TrackDataItem(const char *format, int *counter);

    virtual QString iconName() const = 0;

private:
    TrackDataItem(const TrackDataItem &other) = delete;
    TrackDataItem &operator=(const TrackDataItem &other) = delete;

    void init();

    QString mName;
    bool mExplicitName;
    QVector<QVariant> *mMetadata;
    unsigned long mSelectionId;

    friend class TrackDataContainer;
    TrackDataContainer *mParent;
};

//////////////////////////////////////////////////////////////////////////
//									//
//  TrackDataContainer							//
//									//
//  This is an abstract data item that, in addition to the name and	//
//  metadata provided by TrackDataItem, can also have children.		//
//  Note that any TrackDataItem can have a parent, which must be	//
//  a TrackDataContainer.						//
//									//
//////////////////////////////////////////////////////////////////////////

class TrackDataContainer : public TrackDataItem
{
public:
    virtual ~TrackDataContainer();

    int childCount() const				{ return (mChildren==nullptr ? 0 : mChildren->count()); }
    TrackDataItem *childAt(int idx) const		{ Q_ASSERT(mChildren!=nullptr); return (mChildren->at(idx)); }
    int childIndex(const TrackDataItem *data) const	{ Q_ASSERT(mChildren!=nullptr); return (mChildren->indexOf(const_cast<TrackDataItem *>(data))); }

    void addChildItem(TrackDataItem *data, int idx = -1);
    TrackDataItem *takeFirstChildItem();
    TrackDataItem *takeLastChildItem();
    TrackDataItem *takeChildItem(int idx);
    void removeChildItem(TrackDataItem *item);

    virtual BoundingArea boundingArea() const override;
    virtual TimeRange timeSpan() const override;

protected:
    TrackDataContainer(const char *format, int *counter);

private:
    QList<TrackDataItem *> *mChildren;
};

//////////////////////////////////////////////////////////////////////////
//									//
//  TrackDataFile							//
//									//
//////////////////////////////////////////////////////////////////////////

class TrackDataFile : public TrackDataContainer, public TrackPropertiesInterface
{
public:
    explicit TrackDataFile();
    virtual ~TrackDataFile();

    TrackData::Type type() const override		{ return (TrackData::File); }
    virtual QString statusMessage(int num) const override;
    virtual QString toolTip() const override;

    QUrl fileName() const				{ return (mFileName); }
    void setFileName(const QUrl &file);
    CategoryList *categories() const			{ return (mCategories); }
    void setCategories(CategoryList *list)		{ mCategories = list; }

    DEFINE_PROPERTIES_PAGE(General)
    DEFINE_PROPERTIES_PAGE(Detail)
    DEFINE_PROPERTIES_PAGE(Style)
    DEFINE_PROPERTIES_PAGE(Plot)
    DEFINE_PROPERTIES_PAGE(Metadata)

protected:
    QString iconName() const override;

private:
    QUrl mFileName;
    CategoryList *mCategories;
};

//////////////////////////////////////////////////////////////////////////
//									//
//  TrackDataTrack							//
//									//
//////////////////////////////////////////////////////////////////////////

class TrackDataTrack : public TrackDataContainer, public TrackPropertiesInterface
{
public:
    explicit TrackDataTrack();
    virtual ~TrackDataTrack() = default;

    TrackData::Type type() const override		{ return (TrackData::Track); }
    virtual QString statusMessage(int num) const override;
    virtual QString toolTip() const override;

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

class TrackDataSegment : public TrackDataContainer, public TrackPropertiesInterface
{
public:
    explicit TrackDataSegment();
    virtual ~TrackDataSegment() = default;

    TrackData::Type type() const override		{ return (TrackData::Segment); }
    virtual QString statusMessage(int num) const override;
    virtual QString toolTip() const override;

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

class TrackDataFolder : public TrackDataContainer, public TrackPropertiesInterface
{
public:
    explicit TrackDataFolder();
    virtual ~TrackDataFolder() = default;

    TrackData::Type type() const override		{ return (TrackData::Folder); }
    virtual QString statusMessage(int num) const override;
    virtual QString toolTip() const override;

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

    virtual BoundingArea boundingArea() const override;
    virtual TimeRange timeSpan() const override;
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
    virtual QString statusMessage(int num) const override;
    virtual QString toolTip() const override;

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
    virtual QString statusMessage(int num) const override;
    virtual QString toolTip() const override;

    const PointIcon *icon() const override;

    bool isMediaType() const;

    bool canMerge(const TrackDataWaypoint *other, bool positionOnly = false) const;
    void mergeWith(const TrackDataWaypoint *other);
    QStringList formattedAddress() const;

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

class TrackDataRoute : public TrackDataContainer, public TrackPropertiesInterface
{
public:
    explicit TrackDataRoute();
    virtual ~TrackDataRoute() = default;

    TrackData::Type type() const override		{ return (TrackData::Route); }
    virtual QString statusMessage(int num) const override;
    virtual QString toolTip() const override;

    DEFINE_PROPERTIES_PAGE(General)
    DEFINE_PROPERTIES_PAGE(Detail)
    DEFINE_PROPERTIES_PAGE(Style)
    DEFINE_PROPERTIES_PAGE(Plot)
    DEFINE_PROPERTIES_PAGE(Metadata)

protected:
    QString iconName() const override			{ return ("chart_route"); }
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
    virtual QString statusMessage(int num) const override;
    virtual QString toolTip() const override;

    DEFINE_PROPERTIES_PAGE(General)
    DEFINE_PROPERTIES_PAGE(Detail)
    DEFINE_PROPERTIES_PAGE(Style)
    DEFINE_PROPERTIES_PAGE(Plot)
    DEFINE_PROPERTIES_PAGE(Metadata)

protected:
    // There is a "chart_routepoint" icon (present for completeness),
    // but the flag looks better on the map and plot.  So use it in the
    // data model also.
    QString iconName() const override			{ return ("flag"); }
};

#endif							// TRACKDATA_H
