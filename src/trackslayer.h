// -*-mode:c++ -*-

#ifndef TRACKSLAYER_H
#define TRACKSLAYER_H
 
#include <marble/LayerInterface.h>

using namespace Marble;

class QMouseEvent;
class QElapsedTimer;
class TrackDataItem;
class TrackDataPoint;
class MapView;
class SelectionRun;


class TracksLayer : public QObject, public LayerInterface
{
    Q_OBJECT

public:
    explicit TracksLayer(QWidget *pnt = NULL);
    ~TracksLayer();

    QStringList renderPosition() const;
    qreal zValue() const;
    bool render(GeoPainter *painter, ViewportParams *viewport,
                const QString &renderPos = "NONE", GeoSceneLayer *layer = NULL);

    bool eventFilter(QObject *obj, QEvent *ev);

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

    QElapsedTimer *mClickTimer;
    int mClickX;
    int mClickY;
    const TrackDataPoint *mClickedPoint;
    QList<SelectionRun> *mDraggingPoints;
    double mLatOff, mLonOff;

    double mLatMax, mLatMin;
    double mLonMax, mLonMin;
};

#endif							// TRACKSLAYER_H
