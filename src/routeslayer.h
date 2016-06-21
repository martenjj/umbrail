// -*-mode:c++ -*-

#ifndef ROUTESLAYER_H
#define ROUTESLAYER_H
 
#include <layerbase.h>


class RoutesLayer : public LayerBase
{
    Q_OBJECT

public:
    explicit RoutesLayer(QWidget *pnt = NULL);
    virtual ~RoutesLayer();

    qreal zValue() const			{ return (1.0); }

protected:
    virtual bool isApplicableItem(const TrackDataItem *item) const;
    virtual bool isDirectContainer(const TrackDataItem *item) const;
    virtual bool isIndirectContainer(const TrackDataItem *item) const;

    virtual void doPaintItem(const TrackDataItem *item, GeoPainter *painter, bool isSelected) const;
    virtual void doPaintDrag(const SelectionRun *run, GeoPainter *painter) const;
};

#endif							// ROUTESLAYER_H
