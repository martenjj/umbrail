// -*-mode:c++ -*-

#ifndef LAYERBASE_H
#define LAYERBASE_H

 
#include <klocalizedstring.h>
#include <marble/LayerInterface.h>
#include <marble/GeoDataCoordinates.h>
#include <marble/GeoDataLineString.h>
#include "mainwindowinterface.h"

using namespace Marble;

class QMouseEvent;
class QElapsedTimer;
class QPainter;
class TrackDataItem;
class TrackDataAbstractPoint;
class MapView;

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

class SelectionRun
{
public:
    SelectionRun()					{}
    ~SelectionRun()					{}

    void addPoint(const GeoDataCoordinates &coord)	{ mThesePoints.append(coord); }
    void setPrevPoint(const GeoDataCoordinates &coord)	{ mPrevPoint = coord; }
    void setNextPoint(const GeoDataCoordinates &coord)	{ mNextPoint = coord; }

    void clear();
    bool isEmpty() const				{ return (mThesePoints.isEmpty()); }

    const GeoDataCoordinates *prevPoint() const		{ return (&mPrevPoint); }
    const GeoDataCoordinates *nextPoint() const		{ return (&mNextPoint); }
    const GeoDataLineString *thesePoints() const	{ return (&mThesePoints); }

private:
    GeoDataCoordinates mPrevPoint;
    GeoDataLineString mThesePoints;
    GeoDataCoordinates mNextPoint;
};



////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////


class LayerBase : public QObject, public MainWindowInterface, public Marble::LayerInterface
{
    Q_OBJECT

public:
    explicit LayerBase(QWidget *pnt = nullptr);
    virtual ~LayerBase();

    QStringList renderPosition() const override;
    virtual qreal zValue() const override = 0;
    virtual QString id() const = 0;
    virtual QString name() const = 0;

    bool render(GeoPainter *painter, ViewportParams *viewport,
                const QString &renderPos = "NONE", GeoSceneLayer *layer = nullptr) override;

    bool eventFilter(QObject *obj, QEvent *ev) override;

    void setMovePointsMode(bool on);
    bool isVisible() const				{ return (mVisible); }
    void setVisible(bool on)				{ mVisible = on; }
    void cancelDrag();

signals:
    void draggedPoints(qreal latOff, qreal lonOff);

protected:
    unsigned long mSelectionId;

protected:
    virtual bool isApplicableItem(const TrackDataItem *item) const = 0;
    virtual bool isDirectContainer(const TrackDataItem *item) const = 0;
    virtual bool isIndirectContainer(const TrackDataItem *item) const = 0;

    virtual void doPaintItem(const TrackDataItem *item, GeoPainter *painter, bool isSelected) const = 0;
    virtual void doPaintDrag(const SelectionRun *run, GeoPainter *painter) const = 0;

    GeoDataCoordinates applyOffset(const GeoDataCoordinates &coords) const;
    void setSelectionColours(QPainter *painter, bool setBrush = true) const;

    ViewportParams *viewport() const			{ return (mViewport); }

private:
    void paintDataTree(const TrackDataItem *item, GeoPainter *painter, bool doSelected, bool parentSelected);
    const TrackDataAbstractPoint *findClickedPoint(const TrackDataItem *item);
    bool testClickTolerance(const QMouseEvent *mev) const;
    virtual void findSelectionInTree(const TrackDataItem *item);

private slots:
    void slotInstallEventFilter();

private:
    bool mVisible;
    bool mMovePointsMode;

    QElapsedTimer *mClickTimer;
    int mClickX;
    int mClickY;
    const TrackDataAbstractPoint *mClickedPoint;
    QList<SelectionRun> *mDraggingPoints;
    double mLatOff, mLonOff;

    double mLatMax, mLatMin;
    double mLonMax, mLonMin;

    ViewportParams *mViewport;
};

#endif							// LAYERBASE_H
