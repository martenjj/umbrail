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

    qreal zValue() const override		{ return (2.0); }
    QString id() const override			{ return ("tracks"); }
    QString name() const override		{ return (i18n("Tracks")); }

protected:
    bool isApplicableItem(const TrackDataItem *item) const override;
    bool isDirectContainer(const TrackDataItem *item) const override;
    bool isIndirectContainer(const TrackDataItem *item) const override;

    void doPaintItem(const TrackDataItem *item, GeoPainter *painter, bool isSelected) const override;
    void doPaintDrag(const SelectionRun *run, GeoPainter *painter) const override;
};

#endif							// TRACKSLAYER_H
