// -*-mode:c++ -*-

#ifndef TRACKSLAYER_H
#define TRACKSLAYER_H
 
#include <layerbase.h>


class TracksLayer : public LayerBase
{
    Q_OBJECT

public:
    explicit TracksLayer(QWidget *pnt = NULL);
    virtual ~TracksLayer();

    qreal zValue() const override		{ return (1.0); }
    QString id() const override			{ return ("tracks"); }
    QString name() const override		{ return (i18n("Tracks")); }

protected:
    virtual bool isApplicableItem(const TrackDataItem *item) const;
    virtual bool isDirectContainer(const TrackDataItem *item) const;
    virtual bool isIndirectContainer(const TrackDataItem *item) const;

    virtual void doPaintItem(const TrackDataItem *item, GeoPainter *painter, bool isSelected) const;
    virtual void doPaintDrag(const SelectionRun *run, GeoPainter *painter) const;
};

#endif							// TRACKSLAYER_H
