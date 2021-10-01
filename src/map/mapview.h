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

#ifndef MAPVIEW_H
#define MAPVIEW_H
 
#include <qmap.h>
#include <marble/MarbleWidget.h>
#include "applicationdatainterface.h"

using namespace Marble;

class QAction;
class TrackDataItem;
class TrackDataWaypoint;
class LayerBase;
class StopsLayer;


class MapView : public MarbleWidget, public ApplicationDataInterface
{
    Q_OBJECT

public:
    explicit MapView(QWidget *pnt = nullptr);
    virtual ~MapView();

    void readProperties();
    void saveProperties();

    QString currentPosition() const;
    void setCurrentPosition(const QString &str);

    QStringList allOverlays(bool visibleOnly) const;
    QStringList allLayers(bool visibleOnly) const;

    QAction *actionForOverlay(const QString &id) const;
    QAction *actionForLayer(const QString &id) const;
    void showOverlays(const QStringList &list);
    void setMovePointsMode(bool on);

    static QColor resolveLineColour(const TrackDataItem *tdi);
    static QColor resolvePointColour(const TrackDataItem *tdi);

    void cancelDrag();
    void setStopLayerData(const QList<const TrackDataWaypoint *> *stops);

public slots:               
    void slotRmbRequest(int mx, int my);
    void slotFindAddress();
    void slotShowOverlay();
    void slotShowLayer();
    void slotAddWaypoint();
    void slotAddRoutepoint();

protected:
    bool eventFilter(QObject *obj, QEvent *ev) override;

signals:
    void draggedPoints(qreal latOff, qreal lonOff);
    void createWaypoint(qreal lat, qreal lon);
    void createRoutepoint(qreal lat, qreal lon);

private:
    void addLayer(LayerBase *layer);

private:
    int mPopupX;
    int mPopupY;

    QMap<QString,LayerBase *> mLayers;			// normal display layers
    StopsLayer *mStopsLayer;				// this one is special
};

#endif							// MAPVIEW_H
