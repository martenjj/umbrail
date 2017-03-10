// -*-mode:c++ -*-

#ifndef MAPVIEW_H
#define MAPVIEW_H
 
#include <qmap.h>
#include <marble/MarbleWidget.h>
#include <marble/ReverseGeocodingRunnerManager.h>
#include "mainwindowinterface.h"

using namespace Marble;

class QAction;
class TrackDataItem;
class TrackDataWaypoint;
class LayerBase;
class StopsLayer;


class MapView : public MarbleWidget, public MainWindowInterface
{
    Q_OBJECT

public:
    MapView(QWidget *pnt = NULL);
    virtual ~MapView();

    void readProperties();
    void saveProperties();

    QString currentPosition() const;
    void setCurrentPosition(const QString &str);

    QStringList allOverlays(bool visibleOnly) const;
    QStringList allLayers(bool visibleOnly) const;

    QAction *actionForOverlay(const QString &id) const;
    QAction *actionForLayer(const QString &id) const;
    void showOverlays(const QStringList &list);
    void setMovePointsMode(bool on);

    static QColor resolveLineColour(const TrackDataItem *tdi);
    static QColor resolvePointColour(const TrackDataItem *tdi);

    void setStopLayerData(const QList<const TrackDataWaypoint *> *stops);

public slots:               
    void slotRmbRequest(int mx, int my);
    void slotFindAddress();
    void slotShowOverlay();
    void slotShowLayer();
    void slotAddWaypoint();
    void slotAddRoutepoint();

protected:
    bool eventFilter(QObject *obj, QEvent *ev);

signals:
    void draggedPoints(qreal latOff, qreal lonOff);
    void createWaypoint(qreal lat, qreal lon);
    void createRoutepoint(qreal lat, qreal lon);

private:
    bool mouseCoordinates(GeoDataCoordinates *coords) const;
    void addLayer(LayerBase *layer);

private slots:
    void slotShowAddressInformation(const GeoDataCoordinates &coords, const GeoDataPlacemark &placemark);

private:
    ReverseGeocodingRunnerManager *mRunnerManager;
    int mPopupX;
    int mPopupY;

    QMap<QString,LayerBase *> mLayers;			// normal display layers
    StopsLayer *mStopsLayer;				// this one is special
};

#endif							// MAPVIEW_H
