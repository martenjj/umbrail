// -*-mode:c++ -*-

#ifndef WAYPOINTSLAYER_H
#define WAYPOINTSLAYER_H
 
#include <marble/LayerInterface.h>

using namespace Marble;

class QMouseEvent;
class QElapsedTimer;
class TrackDataItem;
class TrackDataPoint;
class MapView;
// class SelectionRun;


class WaypointsLayer : public QObject, public LayerInterface
{
    Q_OBJECT

public:
    explicit WaypointsLayer(QWidget *pnt = NULL);
    ~WaypointsLayer();

    QStringList renderPosition() const;
    qreal zValue() const;
    bool render(GeoPainter *painter, ViewportParams *viewport,
                const QString &renderPos = "NONE", GeoSceneLayer *layer = NULL);

    bool eventFilter(QObject *obj, QEvent *ev);

    void setMovePointsMode(bool on);

signals:
    void draggedPoints(qreal latOff, qreal lonOff);

private:
    MapView *mapView() const			{ return (mMapView); }

    void paintDataTree(const TrackDataItem *tdi, GeoPainter *painter, bool doSelected, bool parentSelected);

    const TrackDataPoint *findClickedPoint(const TrackDataItem *tdi);
    bool testClickTolerance(const QMouseEvent *mev) const;
    void findSelectionInTree(const TrackDataItem *tdi);

private:
    MapView *mMapView;

    unsigned long mSelectionId;
    bool mMovePointsMode;

    QElapsedTimer *mClickTimer;
    int mClickX;
    int mClickY;
    const TrackDataPoint *mClickedPoint;
//     QList<SelectionRun> *mDraggingPoints;
    double mLatOff, mLonOff;

    double mLatMax, mLatMin;
    double mLonMax, mLonMin;
};

#endif							// WAYPOINTSLAYER_H
