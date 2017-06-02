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

    qreal zValue() const override		{ return (3.0); }
    QString id() const override			{ return ("routes"); }
    QString name() const override		{ return (i18n("Routes")); }

protected:
    bool isApplicableItem(const TrackDataItem *item) const override;
    bool isDirectContainer(const TrackDataItem *item) const override;
    bool isIndirectContainer(const TrackDataItem *item) const override;

    void doPaintItem(const TrackDataItem *item, GeoPainter *painter, bool isSelected) const override;
    void doPaintDrag(const SelectionRun *run, GeoPainter *painter) const override;
};

#endif							// ROUTESLAYER_H
