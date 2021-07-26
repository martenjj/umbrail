
#include "mapview.h"

#include <qmenu.h>
#include <qevent.h>
#include <qapplication.h>
#include <qdebug.h>

#include <klocalizedstring.h>
#include <kmessagebox.h>
#include <kxmlguifactory.h>
#include <kcolorscheme.h>
#include <ktoggleaction.h>
#include <kactioncollection.h>

#include <marble/MarbleWidgetInputHandler.h>
#include <marble/AbstractFloatItem.h>

#include "trackdata.h"
#include "mainwindow.h"
#include "mapcontroller.h"
#include "settings.h"
#include "trackslayer.h"
#include "waypointslayer.h"
#include "routeslayer.h"
#include "stopslayer.h"
#include "positioninfodialogue.h"


// see http://techbase.kde.org/Projects/Marble/MarbleMarbleWidget 
MapView::MapView(QWidget *pnt)
    : MarbleWidget(pnt),
      ApplicationDataInterface(pnt)
{
    qDebug();

//    mRunnerManager = nullptr;

    setMapThemeId("earth/openstreetmap/openstreetmap.dgml");

    setShowOverviewMap(false);
    setShowScaleBar(false);
    setShowCompass(false);
    setShowCrosshairs(false);
    setProjection(Marble::Mercator);

    // Replace existing context menu with our own
    MarbleWidgetInputHandler *ih = inputHandler();
    disconnect(ih, SIGNAL(rmbRequest(int,int)), nullptr, nullptr);
    connect(ih, SIGNAL(rmbRequest(int,int)), SLOT(slotRmbRequest(int,int)));

    installEventFilter(this);				// modify cursor shape

    addLayer(new TracksLayer(this));			// tracks display layer
    addLayer(new WaypointsLayer(this));			// waypoints display layer
    addLayer(new RoutesLayer(this));			// routes display layer

    mStopsLayer = new StopsLayer;			// temporary stops display layer
    MarbleWidget::addLayer(mStopsLayer);
}


MapView::~MapView()
{
    delete mStopsLayer;
    qDebug() << "done";
}


void MapView::addLayer(LayerBase *layer)
{
    qDebug() << layer->id();
    connect(layer, SIGNAL(draggedPoints(qreal,qreal)), SIGNAL(draggedPoints(qreal,qreal)));
    mLayers[layer->id()] = layer;
    MarbleWidget::addLayer(layer);
}


QString MapView::currentPosition() const
{
    return (MapController::positionToString(centerLatitude(), centerLongitude(), zoom()));
}


void MapView::setCurrentPosition(const QString &str)
{
    qDebug() << str;

    double lat,lon;
    int z;
    if (MapController::positionFromString(str, &lat, &lon, &z))
    {
        centerOn(lon, lat);
        zoomView(z);
    }
}


void MapView::readProperties()
{
    QString s = Settings::mapCurrent();
    if (!s.isEmpty()) setCurrentPosition(s);

    // TODO: crosshairs etc?
    QString themeId = Settings::mapTheme();
    if (!themeId.isEmpty()) setMapThemeId(themeId);
    showOverlays(Settings::mapOverlays());
}


void MapView::saveProperties()
{
    QString s = currentPosition();
    qDebug() << "  current" << s;
    Settings::setMapCurrent(s);

    // TODO: crosshairs etc?
    Settings::setMapTheme(mapThemeId());
    Settings::setMapOverlays(allOverlays(true));
}


void MapView::slotRmbRequest(int mx, int my)
{
    mPopupX = mx;					// save for actions
    mPopupY = my;

    QMenu *popup = static_cast<QMenu *>(
        mainWindow()->factory()->container("mapview_contextmenu",
                                           mainWindow()));
    if (popup==nullptr) return;

    QAction *act = mainWindow()->actionCollection()->action("map_add_waypoint");
    if(act!=nullptr) act->setEnabled(!isReadOnly());
    act = mainWindow()->actionCollection()->action("map_add_routepoint");
    if(act!=nullptr) act->setEnabled(!isReadOnly());

    popup->exec(mapToGlobal(QPoint(mx, my)));
}


void MapView::slotFindAddress()
{
    PositionInfoDialogue pd(mPopupX, mPopupY, this);
    pd.exec();
}


void MapView::slotAddWaypoint()
{
   qreal lat = 0.0;
   qreal lon = 0.0;
   const bool valid = geoCoordinates(mPopupX, mPopupY, lon, lat, GeoDataCoordinates::Degree);
   if (valid) emit createWaypoint(lat, lon);
}


void MapView::slotAddRoutepoint()
{
   qreal lat = 0.0;
   qreal lon = 0.0;
   const bool valid = geoCoordinates(mPopupX, mPopupY, lon, lat, GeoDataCoordinates::Degree);
   if (valid) emit createRoutepoint(lat, lon);
}


QStringList MapView::allOverlays(bool visibleOnly) const
{
    QStringList result;
    QList<AbstractFloatItem *> items = floatItems();
    for (QList<AbstractFloatItem *>::const_iterator it = items.constBegin();
         it!=items.constEnd(); ++it)
    {
        AbstractFloatItem *item = (*it);
        if (item==nullptr) continue;

        if (!visibleOnly || item->visible()) result.append(item->nameId());
    }

    qDebug() << "visibleOnly" << visibleOnly << "->" << result;
    return (result);
}


QStringList MapView::allLayers(bool visibleOnly) const
{
    QStringList result;

    QStringList itemIds = mLayers.keys();
    foreach (const QString &itemId, itemIds)
    {
        const LayerBase *layer = mLayers.value(itemId);
        if (layer==nullptr) continue;

        if (!visibleOnly || layer->isVisible()) result.append(itemId);
    }

    qDebug() << "visibleOnly" << visibleOnly << "->" << result;
    return (result);
}


void MapView::showOverlays(const QStringList &list)
{
    QList<AbstractFloatItem *> items = floatItems();
    for (QList<AbstractFloatItem *>::const_iterator it = items.constBegin();
         it!=items.constEnd(); ++it)
    {
        AbstractFloatItem *item = (*it);
        if (item!=nullptr) item->setVisible(list.contains(item->nameId()));
    }
}


QAction *MapView::actionForOverlay(const QString &id) const
{
    const AbstractFloatItem *item = floatItem(id);
    if (item==nullptr) return (nullptr);

    QAction *a = new QAction(item->icon(),
                             i18n("%1 - %2", item->guiString(), item->description()),
                             mainWindow());
    a->setData(id);					// record ID for action
    a->setChecked(item->visible());			// set initial check state
    return (a);
}


void MapView::slotShowOverlay()
{
    QAction *a = static_cast<QAction*>(sender());	// action that was triggered
    if (a==nullptr) return;

    AbstractFloatItem *item = floatItem(a->data().toString());
    if (item==nullptr) return;				// item ID from user data

    bool nowVisible = !item->visible();
    item->setVisible(nowVisible);
    a->setChecked(nowVisible);
    update();
}


QAction *MapView::actionForLayer(const QString &id) const
{
    const LayerBase *layer = mLayers.value(id);
    if (layer==nullptr) return (nullptr);

    QAction *a = new KToggleAction(layer->name(), mainWindow());
    a->setData(id);					// record ID for action
    a->setChecked(layer->isVisible());			// set initial check state
    return (a);
}


void MapView::slotShowLayer()
{
    QAction *a = static_cast<QAction*>(sender());	// action that was triggered
    if (a==nullptr) return;

    LayerBase *layer = mLayers.value(a->data().toString());
    if (layer==nullptr) return;

    bool nowVisible = !layer->isVisible();
    layer->setVisible(nowVisible);
    a->setChecked(nowVisible);
    update();
}


static QColor resolveColour(const TrackDataItem *item, const char *key, const QColor &appDefault)
{
    // Resolving a colour is not a trivial operation - needing to examine not only
    // the metadata of the item, but also all of its parents, up to the top level file,
    // then finally the application's default.  However, this search will only need
    // to be performed once per track segment, of which it is expected that there
    // will be at most a few tens of them existing within a typical project and are
    // not nested arbitrarily deeply.  So the overhead here is not likely to be
    // significant.

    while (item!=nullptr)				// search to root of tree
    {
        const QVariant v = item->metadata(key);		// metadata from this item
        if (!v.isNull()) return (v.value<QColor>());	// colour value from that
        item = item->parent();				// up to parent item
    }

    return (appDefault);				// finally application default
}


QColor MapView::resolveLineColour(const TrackDataItem *tdi)
{
    return (resolveColour(tdi, "linecolor", Settings::lineColour()));
}


QColor MapView::resolvePointColour(const TrackDataItem *tdi)
{
    // Currently point colour is not inherited.  If set on the point item
    // then it will be used, otherwise waypoints will use the default icon
    // and other sorts of points will use the application setting.

    //return (resolveColour(tdi, "pointcolour", Settings::pointColour()));

    const QVariant v = tdi->metadata("pointcolor");	// metadata from this item
    if (!v.isNull()) return (v.value<QColor>());	// colour value from that
    return (QColor());					// no colour set
}


void MapView::setMovePointsMode(bool on)
{
    qDebug() << on;
    foreach (LayerBase *layer, mLayers)
    {
        layer->setMovePointsMode(on);
    }
}


void MapView::cancelDrag()
{
    qDebug();
    foreach (LayerBase *layer, mLayers)
    {
        layer->cancelDrag();
    }
}


bool MapView::eventFilter(QObject *obj, QEvent *ev)
{
    if (ev->type()==QEvent::CursorChange)
    {							// force our cursor shape
        // Marble forces the "hand" cursor while idle, see
        // MarbleWidgetDefaultInputHandler::eventFilter()
        if (cursor().shape()==Qt::OpenHandCursor) setCursor(Qt::CrossCursor);
    }
    else if (ev->type()==QEvent::PaletteChange)		// watch for palette changes
    {
        // formerly in slotSystemPaletteChanged()
        bool syscol = Settings::selectedUseSystemColours();
        qDebug() << "using system?" << syscol;
        if (syscol) update();
    }

    return (false);					// pass event on
}


void MapView::setStopLayerData(const QList<const TrackDataWaypoint *> *stops)
{
    mStopsLayer->setStopsData(stops);
    update();
}
