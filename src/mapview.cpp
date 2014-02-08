
#include "mapview.h"

#include <qmenu.h>
#include <qimage.h>

#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kxmlguifactory.h>
#include <kaction.h>

#include <marble/GeoPainter.h>
#include <marble/MarbleWidgetInputHandler.h>
#include <marble/MarbleModel.h>
#include <marble/GeoDataPlacemark.h>
#include <marble/AbstractFloatItem.h>

#include "filesmodel.h"
#include "trackdata.h"
#include "mainwindow.h"
#include "style.h"



// see http://techbase.kde.org/Projects/Marble/MarbleMarbleWidget 
MapView::MapView(QWidget *parent)
    : MarbleWidget(parent)
{
    kDebug();

    mMainWindow = qobject_cast<MainWindow *>(parent);

    mModel = NULL;
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
}


MapView::~MapView()
{
    kDebug() << "done";
}


// Do nothing here and next.  Map properties are saved in the project data file.
void MapView::readProperties()
{
}


void MapView::saveProperties()
{
}



// see http://techbase.kde.org/Projects/Marble/MarbleGeoPainter
void MapView::customPaint(GeoPainter *painter)
{
    if (filesModel()==NULL) return;
    paintDataTree(filesModel()->rootItem(), painter);
}


void MapView::paintDataTree(const TrackDataItem *tdi, GeoPainter *painter)
{
    int cnt = tdi->childCount();
    if (cnt==0) return;					// quick escape if no children

    kDebug() << tdi << tdi->name();

    const TrackDataItem *firstChild = tdi->childAt(0);	// look at first child
    if (dynamic_cast<const TrackDataPoint *>(firstChild)!=NULL)
    {							// see if it is a point
        kDebug() << "points for" << tdi->name();

///////////// TODO: line width/style: inherited or global setting
//        painter->setPen(QPen(Qt::black, 4));
        painter->setPen(QPen(
    Style::globalStyle()->lineColour()
, 4));

        GeoDataLineString lines;
        for (int i = 0; i<cnt; ++i)
        {
            const TrackDataPoint *tdp = dynamic_cast<const TrackDataPoint *>(tdi->childAt(i));
            if (tdp!=NULL)
            {
                GeoDataCoordinates coord(tdp->longitude(), tdp->latitude(),
                                         0, GeoDataCoordinates::Degree);
                lines.append(coord);
            }
        }

        painter->drawPolyline(lines);

///////////// TODO: points for selected segment




    }
    else						// not a point, so a container
    {
        for (int i = 0; i<cnt; ++i)			// recurse to paint children
        {
            paintDataTree(tdi->childAt(i), painter);
        }
    }
}








//    const int cnt = mModel->pointsCount();
//
//    for (int i = 0; i<cnt; ++i)
//    {
//        const PointData *pnt = mModel->pointAt(i);
//
//        GeoDataCoordinates coord(pnt->longtitude(),
//                                 pnt->latitude(),
//                                 0, GeoDataCoordinates::Degree);
//
//        const QImage *img = mainWindow()->pointsController()->iconsManager()->iconForName(pnt->symbol());
//        if (img!=NULL)					// icon image available
//        {
//            painter->drawImage(coord, *img);
//        }
//        else						// draw our own marker
//        {
//            painter->setPen(QPen(Qt::red, 2));
//            painter->setBrush(Qt::yellow);
//            painter->drawEllipse(coord, 10, 10);
//        }
//
//        painter->save();
//        painter->translate(12, 3);			// offset text from point
//        painter->setPen(Qt::gray);			// draw with drop shadow
//        painter->drawText(coord, pnt->name());
//        painter->translate(-1, -1);
//        painter->setPen(Qt::black);
//        painter->drawText(coord, pnt->name());
//        painter->restore();
//    }



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
bool MapView::mouseCoordinates(GeoDataCoordinates *coords)
{
    bool valid = true;
    qreal lat = 0.0;
    qreal lon = 0.0;

    valid = geoCoordinates(mPopupX, mPopupY, lon, lat, GeoDataCoordinates::Radian);
    if (valid) *coords = GeoDataCoordinates(lon, lat);
    return (valid);
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

    kDebug() << "visibleOnly" << visibleOnly << "->" << result;
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



KAction *MapView::actionForOverlay(const QString &id) const
{
    const AbstractFloatItem *item = floatItem(id);
    if (item==NULL) return (NULL);

    KAction *a = new KAction(KIcon(item->icon()),
                             i18n("%1 - %2", item->guiString(), item->description()),
                             mainWindow());
    a->setData(id);					// record ID for action
    a->setChecked(item->visible());			// set initial check state
    return (a);
}


void MapView::slotShowOverlay()
{
    KAction *a = static_cast<KAction*>(sender());	// action that was triggered
    if (a==NULL) return;

    AbstractFloatItem *item = floatItem(a->data().toString());
    if (item==NULL) return;				// item ID from user data

    bool nowVisible = !item->visible();
    item->setVisible(nowVisible);
    a->setChecked(nowVisible);
}
