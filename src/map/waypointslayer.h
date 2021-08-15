// -*-mode:c++ -*-

#ifndef WAYPOINTSLAYER_H
#define WAYPOINTSLAYER_H
 
#include <layerbase.h>


class WaypointsLayer : public LayerBase
{
    Q_OBJECT

public:
    explicit WaypointsLayer(QWidget *pnt = nullptr);
    virtual ~WaypointsLayer();

    qreal zValue() const override		{ return (4.0); }
    QString id() const override			{ return ("waypoints"); }
    QString name() const override		{ return (i18n("Waypoints")); }

protected:
    bool isApplicableItem(const TrackDataItem *item) const override;
    bool isDirectContainer(const TrackDataItem *item) const override;
    bool isIndirectContainer(const TrackDataItem *item) const override;

    void doPaintItem(const TrackDataItem *item, GeoPainter *painter, bool isSelected) const override;
    void doPaintDrag(const SelectionRun *run, GeoPainter *painter) const override;
};

#endif							// WAYPOINTSLAYER_H