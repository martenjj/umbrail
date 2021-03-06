//////////////////////////////////////////////////////////////////////////
//									//
//  Project:	Umbrail - GPX track viewer and editor			//
//									//
//////////////////////////////////////////////////////////////////////////
//									//
//  Copyright (c) 2014-2021 Jonathan Marten <jjm@keelhaul.me.uk>	//
//  Home and download page: <http://github.com/martenjj/umbrail>	//
//									//
//  This program is free software; you can redistribute it and/or	//
//  modify it under the terms of the GNU General Public License as	//
//  published by the Free Software Foundation, either version 3 of	//
//  the License or (at your option) any later version.			//
//									//
//  It is distributed in the hope that it will be useful, but		//
//  WITHOUT ANY WARRANTY;  without even the implied warranty of		//
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the	//
//  GNU General Public License for more details.			//
//									//
//  You should have received a copy of the GNU General Public License	//
//  along with this program;  see the file COPYING for further		//
//  details.  If not, see <http://gnu.org/licenses/gpl>.      		//
//									//
//////////////////////////////////////////////////////////////////////////

#ifndef LAYERBASE_H
#define LAYERBASE_H
 
#include <klocalizedstring.h>
#include <marble/LayerInterface.h>
#include <marble/GeoDataCoordinates.h>
#include <marble/GeoDataLineString.h>
#include "applicationdatainterface.h"

using namespace Marble;

class QMouseEvent;
class QElapsedTimer;
class QPainter;
class TrackDataItem;
class TrackDataAbstractPoint;
class MapView;


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


class LayerBase : public QObject, public ApplicationDataInterface, public Marble::LayerInterface
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
