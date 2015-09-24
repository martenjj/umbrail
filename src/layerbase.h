// -*-mode:c++ -*-

#ifndef LAYERBASE_H
#define LAYERBASE_H
 
#include <marble/LayerInterface.h>
#include <marble/GeoDataCoordinates.h>
#include <marble/GeoDataLineString.h>

using namespace Marble;

class QMouseEvent;
class QElapsedTimer;
class QPainter;
class TrackDataItem;
class TrackDataAbstractPoint;
class MapView;


////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

class SelectionRun
{
public:
    SelectionRun()					{}
    ~SelectionRun()					{}

    void addPoint(const GeoDataCoordinates &coord)	{ mThesePoints.append(coord); }
    void setPrevPoint(const GeoDataCoordinates &coord)	{ mPrevPoint = coord; }
    void setNextPoint(const GeoDataCoordinates &coord)	{ mNextPoint = coord; }

    void clear();
    bool isEmpty() const				{ return (mThesePoints.isEmpty()); }

    const GeoDataCoordinates *prevPoint() const		{ return (&mPrevPoint); }
    const GeoDataCoordinates *nextPoint() const		{ return (&mNextPoint); }
    const GeoDataLineString *thesePoints() const	{ return (&mThesePoints); }

private:
    GeoDataCoordinates mPrevPoint;
    GeoDataLineString mThesePoints;
    GeoDataCoordinates mNextPoint;
};



////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////


class LayerBase : public QObject, public LayerInterface
{
    Q_OBJECT

public:
    explicit LayerBase(QWidget *pnt = NULL);
    virtual ~LayerBase();

    QStringList renderPosition() const;
    virtual qreal zValue() const = 0;
    bool render(GeoPainter *painter, ViewportParams *viewport,
                const QString &renderPos = "NONE", GeoSceneLayer *layer = NULL);

    bool eventFilter(QObject *obj, QEvent *ev);

    void setMovePointsMode(bool on);

signals:
    void draggedPoints(qreal latOff, qreal lonOff);

protected:
    unsigned long mSelectionId;

protected:
    MapView *mapView() const			{ return (mMapView); }

    virtual bool isApplicableItem(const TrackDataItem *item) const = 0;
    virtual bool isDirectContainer(const TrackDataItem *item) const = 0;
    virtual bool isIndirectContainer(const TrackDataItem *item) const = 0;

    virtual void doPaintItem(const TrackDataItem *item, GeoPainter *painter, bool isSelected) const = 0;
    virtual void doPaintDrag(const SelectionRun *run, GeoPainter *painter) const = 0;

    GeoDataCoordinates applyOffset(const GeoDataCoordinates &coords) const;
    void setSelectionColours(QPainter *painter, bool setBrush = true) const;

private:
    void paintDataTree(const TrackDataItem *item, GeoPainter *painter, bool doSelected, bool parentSelected);
    const TrackDataAbstractPoint *findClickedPoint(const TrackDataItem *item);
    bool testClickTolerance(const QMouseEvent *mev) const;
    virtual void findSelectionInTree(const TrackDataItem *item);

private:
    MapView *mMapView;

    bool mMovePointsMode;

    QElapsedTimer *mClickTimer;
    int mClickX;
    int mClickY;
    const TrackDataAbstractPoint *mClickedPoint;
    QList<SelectionRun> *mDraggingPoints;
    double mLatOff, mLonOff;

    double mLatMax, mLatMin;
    double mLonMax, mLonMin;
};

#endif							// LAYERBASE_H
