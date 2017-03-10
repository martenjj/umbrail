// -*-mode:c++ -*-

#ifndef WAYPOINTSLAYER_H
#define WAYPOINTSLAYER_H
 
#include <layerbase.h>


class WaypointsLayer : public LayerBase
{
    Q_OBJECT

public:
    explicit WaypointsLayer(QWidget *pnt = NULL);
    virtual ~WaypointsLayer();

    qreal zValue() const override		{ return (2.0); }
    QString id() const override			{ return ("waypoints"); }
    QString name() const override		{ return (i18n("Waypoints")); }

protected:
    virtual bool isApplicableItem(const TrackDataItem *item) const;
    virtual bool isDirectContainer(const TrackDataItem *item) const;
    virtual bool isIndirectContainer(const TrackDataItem *item) const;

    virtual void doPaintItem(const TrackDataItem *item, GeoPainter *painter, bool isSelected) const;
    virtual void doPaintDrag(const SelectionRun *run, GeoPainter *painter) const;
};

#endif							// WAYPOINTSLAYER_H
