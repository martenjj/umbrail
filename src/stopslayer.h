// -*-mode:c++ -*-

#ifndef STOPSLAYER_H
#define STOPSLAYER_H
 
#include <marble/LayerInterface.h>

using namespace Marble;

class TrackDataStop;


class StopsLayer : public Marble::LayerInterface
{
public:
    explicit StopsLayer(QWidget *pnt = NULL);
    virtual ~StopsLayer();

    qreal zValue() const			{ return (3.0); }
    QStringList renderPosition() const;

    bool render(GeoPainter *painter, ViewportParams *viewport,
                const QString &renderPos = "NONE", GeoSceneLayer *layer = NULL);

    void setStopsData(const QList<const TrackDataStop *> *data);

private:
    const QList<const TrackDataStop *> *mStopsData;
};

#endif							// WAYPOINTSLAYER_H
