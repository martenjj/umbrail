// -*-mode:c++ -*-

#ifndef STOPSLAYER_H
#define STOPSLAYER_H
 
#include <qstring.h>
#include <marble/LayerInterface.h>

using namespace Marble;

class QWidget;
class TrackDataWaypoint;


class StopsLayer : public Marble::LayerInterface
{
public:
    explicit StopsLayer(QWidget *pnt = NULL);
    virtual ~StopsLayer();

    qreal zValue() const override		{ return (5.0); }
    QStringList renderPosition() const;

    bool render(GeoPainter *painter, ViewportParams *viewport,
                const QString &renderPos = "NONE", GeoSceneLayer *layer = NULL);

    void setStopsData(const QList<const TrackDataWaypoint *> *data);

private:
    const QList<const TrackDataWaypoint *> *mStopsData;
};

#endif							// WAYPOINTSLAYER_H
