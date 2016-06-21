
#include "mapview.h"

#include <qmenu.h>
#include <qevent.h>
#include <qapplication.h>
#include <qdebug.h>

#include <klocalizedstring.h>
#include <kmessagebox.h>
#include <kxmlguifactory.h>
#include <kcolorscheme.h>

#include <marble/MarbleWidgetInputHandler.h>
#include <marble/GeoDataPlacemark.h>
#include <marble/AbstractFloatItem.h>

#include "trackdata.h"
#include "mainwindow.h"
#include "mapcontroller.h"
#include "style.h"
#include "settings.h"
#include "trackslayer.h"
#include "waypointslayer.h"
#include "routeslayer.h"
#include "stopslayer.h"


// see http://techbase.kde.org/Projects/Marble/MarbleMarbleWidget 
MapView::MapView(QWidget *pnt)
    : MarbleWidget(pnt),
      MainWindowInterface(pnt)
{
    qDebug();

    mRunnerManager = NULL;

    setMapThemeId("earth/openstreetmap/openstreetmap.dgml");

    setShowOverviewMap(false);
    setShowScaleBar(false);
    setShowCompass(false);
    setShowCrosshairs(false);
    setProjection(Marble::Mercator);

    // Replace existing context menu with our own
    MarbleWidgetInputHandler *ih = inputHandler();
    disconnect(ih, SIGNAL(rmbRequest(int,int)), NULL, NULL);
    connect(ih, SIGNAL(rmbRequest(int,int)), SLOT(slotRmbRequest(int,int)));

    // Modify cursor shape
    installEventFilter(this);

    // Tracks display layer
    mTracksLayer = new TracksLayer(this);
    connect(mTracksLayer, SIGNAL(draggedPoints(qreal,qreal)), SIGNAL(draggedPoints(qreal,qreal)));
    addLayer(mTracksLayer);

    // Waypoints display layer
    mWaypointsLayer = new WaypointsLayer(this);
    connect(mWaypointsLayer, SIGNAL(draggedPoints(qreal,qreal)), SIGNAL(draggedPoints(qreal,qreal)));
    addLayer(mWaypointsLayer);

    // Routes display layer
    mRoutesLayer = new RoutesLayer(this);
    connect(mRoutesLayer, SIGNAL(draggedPoints(qreal,qreal)), SIGNAL(draggedPoints(qreal,qreal)));
    addLayer(mRoutesLayer);

    // Temporary stops display layer
    mStopsLayer = new StopsLayer;			// don't want auto delete for this
    addLayer(mStopsLayer);
}


MapView::~MapView()
{
    delete mStopsLayer;
    qDebug() << "done";
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
    Settings::setMapOverlays(overlays(true));
}


void MapView::slotRmbRequest(int mx, int my)
{
    mPopupX = mx;					// save for actions
    mPopupY = my;

    QMenu *popup = static_cast<QMenu *>(
        mainWindow()->factory()->container("mapview_contextmenu",
                                           mainWindow()));
    if (popup!=NULL) popup->exec(mapToGlobal(QPoint(mx, my)));
}


// from MarbleWidgetPopupMenu::startReverseGeocoding()
void MapView::slotFindAddress()
{
    if (mRunnerManager==NULL)
    {
        mRunnerManager = new ReverseGeocodingRunnerManager(model(), this);
        connect(mRunnerManager, SIGNAL(reverseGeocodingFinished(GeoDataCoordinates, GeoDataPlacemark)),
                SLOT(slotShowAddressInformation(GeoDataCoordinates,GeoDataPlacemark)));
    }

    GeoDataCoordinates coords;
    if (mouseCoordinates(&coords)) mRunnerManager->reverseGeocoding(coords);
}


// from MarbleWidgetPopupMenu::showAddressInformation()
void MapView::slotShowAddressInformation(const GeoDataCoordinates &coords,
                                         const GeoDataPlacemark &placemark)
{
    QString text = placemark.address();
    text.replace(", ", "\n");
    KMessageBox::information(this, text, i18n("Address"));
}


// from MarbleWidgetPopupMenu::mouseCoordinates()
bool MapView::mouseCoordinates(GeoDataCoordinates *coords) const
{
    qreal lat = 0.0;
    qreal lon = 0.0;

    const bool valid = geoCoordinates(mPopupX, mPopupY, lon, lat, GeoDataCoordinates::Radian);
    if (valid) *coords = GeoDataCoordinates(lon, lat);
    return (valid);
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


QStringList MapView::overlays(bool visibleOnly) const
{
    QStringList result;
    QList<AbstractFloatItem *> items = floatItems();
    for (QList<AbstractFloatItem *>::const_iterator it = items.constBegin();
         it!=items.constEnd(); ++it)
    {
        AbstractFloatItem *item = (*it);
        if (item==NULL) continue;

        if (!visibleOnly || item->visible()) result.append(item->nameId());
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
        if (item!=NULL) item->setVisible(list.contains(item->nameId()));
    }
}


QAction *MapView::actionForOverlay(const QString &id) const
{
    const AbstractFloatItem *item = floatItem(id);
    if (item==NULL) return (NULL);

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
    if (a==NULL) return;

    AbstractFloatItem *item = floatItem(a->data().toString());
    if (item==NULL) return;				// item ID from user data

    bool nowVisible = !item->visible();
    item->setVisible(nowVisible);
    a->setChecked(nowVisible);
}


QColor MapView::resolveLineColour(const TrackDataItem *tdi)
{
    // Resolving the line colour (in the case where it is not selected) could
    // potentially be an expensive operation - needing to examine not only the
    // style of the current item, but also all of its parents, up to the top
    // level file, then finally the application's default style.  However, this
    // search will only need to be performed once per track segment, of which it
    // is expected that there will be at most a few tens of them existing within
    // a typical project.  So the overhead here is not likely to be significant.

    while (tdi!=NULL)					// search to root of tree
    {
        const Style *s = tdi->style();			// get style from item
        //qDebug() << "style for" << tdi->name() << "is" << *s;
        if (s->hasLineColour())				// has a style set?
        {
            //qDebug() << "inherited from" << tdi->name();
            return (s->lineColour());			// use colour from that
        }
        tdi = tdi->parent();				// up to parent item
    }

    return (Style::globalStyle()->lineColour());	// finally application default
}


QColor MapView::resolvePointColour(const TrackDataItem *tdi)
{
    while (tdi!=NULL)					// search to root of tree
    {
        const Style *s = tdi->style();			// get style from item
        if (s->hasPointColour())				// has a style set?
        {
            return (s->pointColour());			// use colour from that
        }
        tdi = tdi->parent();				// up to parent item
    }

    return (Style::globalStyle()->pointColour());	// finally application default
}


void MapView::setMovePointsMode(bool on)
{
    qDebug() << on;
    mTracksLayer->setMovePointsMode(on);
    mWaypointsLayer->setMovePointsMode(on);
    mRoutesLayer->setMovePointsMode(on);
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


void MapView::setStopLayerData(const QList<const TrackDataWaypoint *> *data)
{
    mStopsLayer->setStopsData(data);
    update();
}
