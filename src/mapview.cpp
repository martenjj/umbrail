
#include "mapview.h"

#include <qmenu.h>
#include <qimage.h>

#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kxmlguifactory.h>
#include <kaction.h>
#include <kcolorscheme.h>
#include <kglobalsettings.h>

#include <marble/GeoPainter.h>
#include <marble/MarbleWidgetInputHandler.h>
#include <marble/MarbleModel.h>
#include <marble/GeoDataPlacemark.h>
#include <marble/AbstractFloatItem.h>

#include "filesmodel.h"
#include "filesview.h"
#include "trackdata.h"
#include "mainwindow.h"
#include "style.h"
#include "settings.h"



static const int POINTS_PER_BLOCK = 50;			// points drawn per polyline
static const int POINT_CIRCLE_SIZE = 10;		// size of point circle
static const int POINT_SELECTED_WIDTH = 3;		// line width for selected points


#undef DEBUG_PAINTING



// see http://techbase.kde.org/Projects/Marble/MarbleMarbleWidget 
MapView::MapView(QWidget *pnt)
    : MarbleWidget(pnt)
{
    kDebug();

    mMainWindow = qobject_cast<MainWindow *>(pnt);

    mFilesModel = NULL;
    mFilesView = NULL;
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

    // Watch for system palette changes
    connect(KGlobalSettings::self(), SIGNAL(kdisplayPaletteChanged()), SLOT(slotSystemPaletteChanged()));
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
    if (filesModel()==NULL) return;			// no data to use!

    mSelectionId = (filesView()!=NULL ? filesView()->selectionId() : 0);

    // Paint the data in two passes.  The first does all non-selected tracks,
    // the second selected ones.  This is so that selected tracks show up
    // on top of all non-selected ones.  In the absence of any selection,
    // all tracks will be painted in time order (i.e. later tracks on top
    // of earlier ones).
    paintDataTree(filesModel()->rootFileItem(), painter, false, false);
    paintDataTree(filesModel()->rootFileItem(), painter, true, false);
}


void MapView::paintDataTree(const TrackDataDisplayable *tdd, GeoPainter *painter, 
                            bool doSelected, bool parentSelected)
{
    if (tdd==NULL) return;				// nothing to paint
    int cnt = tdd->childCount();
    if (cnt==0) return;					// quick escape if no children

    bool isSelected = parentSelected || (tdd->selectionId()==mSelectionId);
#ifdef DEBUG_PAINTING
    kDebug() << tdd->name() << "isselected" << isSelected << "doselected" << doSelected;
#endif

    // What is actually drawn on the map is a polyline representing a sequence
    // of points (with their parent container normally a track segment, but this
    // is not mandated).  So it is not necessary to recurse all the way down the
    // data tree to the individual points, we can stop at the level above if the
    // first child item (assumed to be representative of the others) is a point.

    const TrackDataItem *firstChild = tdd->childAt(0);
    if (dynamic_cast<const TrackDataPoint *>(firstChild)!=NULL)
    {							// is first child a point?
        if (!(isSelected ^ doSelected))			// check selected or not
        {
#ifdef DEBUG_PAINTING
            kDebug() << "points for" << tdd->name() << "count" << cnt;
#endif
            // Scan along the segment, assembling the coordinates into a list,
            // and draw them as a polyline.
            //
            // Some polyline segments appear to be not drawn if there are too many
            // of them at high magnifications - is this a limitation in Marble?
            // Split the drawing up if there are too many - this may make clipping
            // more efficient too.

            QColor col = resolveLineColour(tdd);
            painter->setBrush(Qt::NoBrush);
            painter->setPen(QPen(col, 4));

            GeoDataLineString lines;			// generated coordinate list
            int start = 0;				// start point in list
            while (start<(cnt-1))			// while more blocks to do
            {
#ifdef DEBUG_PAINTING
                kDebug() << "starting at" << start;
#endif
                lines.clear();				// empty the list
                int sofar = 0;				// points so far this block
                for (int i = start; i<cnt; ++i)		// up to end of list
                {
                    const TrackDataPoint *tdp = static_cast<const TrackDataPoint *>(tdd->childAt(i));
                    GeoDataCoordinates coord(tdp->longitude(), tdp->latitude(),
                                             0, GeoDataCoordinates::Degree);
                    lines.append(coord);		// add point to list
                    ++sofar;				// count how many this block
                    if (sofar>=POINTS_PER_BLOCK) break;	// too many, draw and start again
                }

#ifdef DEBUG_PAINTING
                kDebug() << "block of" << sofar << "points";
#endif
                if (lines.size()<2) break;		// nothing more to draw
                painter->drawPolyline(lines);		// draw track in its colour

                // Step back one point from the last one drawn, so that
                // the line connecting the last point of the previous
                // block to the first of the next is drawn.
                start += sofar-1;
            }

            if (isSelected)
            {
                // Next, if the line is selected, draw the point markers.  In the
                // same track colour, circled with a black cosmetic line.

                painter->setPen(QPen(Qt::black, 0));
                painter->setBrush(col);

                for (int i = 0; i<cnt; ++i)
                {
                    const TrackDataPoint *tdp = static_cast<const TrackDataPoint *>(tdd->childAt(i));
                    GeoDataCoordinates coord(tdp->longitude(), tdp->latitude(),
                                             0, GeoDataCoordinates::Degree);
                    painter->drawEllipse(coord, POINT_CIRCLE_SIZE, POINT_CIRCLE_SIZE);
                }

                // Finally, draw the selected points along the line, if there are any.
                // Do this last so that the selection markers always show up on top.

                if (Settings::selectedUseSystemColours())
                {					// use system selection colours
                    KColorScheme sch(QPalette::Active, KColorScheme::Selection);
                    painter->setPen(QPen(sch.background().color(), POINT_SELECTED_WIDTH));
                    painter->setBrush(sch.foreground());
                }
                else					// our own custom colours
                {
                    painter->setPen(QPen(Settings::selectedMarkOuter(), POINT_SELECTED_WIDTH));
                    painter->setBrush(Settings::selectedMarkInner());
                }

                for (int i = 0; i<cnt; ++i)
                {
                    const TrackDataPoint *tdp = static_cast<const TrackDataPoint *>(tdd->childAt(i));
                    if (tdp->selectionId()==mSelectionId)
                    {
                        GeoDataCoordinates coord(tdp->longitude(), tdp->latitude(),
                                                 0, GeoDataCoordinates::Degree);
                        painter->drawEllipse(coord, POINT_CIRCLE_SIZE, POINT_CIRCLE_SIZE);
                    }
                }
            }
        }
    }
    else						// first child not a point,
    {							// so we are higher container
        for (int i = 0; i<cnt; ++i)			// just recurse to paint children
        {
            const TrackDataDisplayable *childItem = static_cast<TrackDataDisplayable *>(tdd->childAt(i));
            paintDataTree(childItem, painter, doSelected, isSelected);
        }
    }
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




QColor MapView::resolveLineColour(const TrackDataDisplayable *tdd)
{
    // Resolving the line colour (in the case where it is not selected) could
    // potentially be an expensive operation - needing to examine not only the
    // style of the current item, but also all of its parents, then the project
    // default, then finally the application's default style.  However, this
    // search will only need to be performed once per track segment, of which it
    // is expected that there will be at most a few tens of them existing within
    // a typical project.  So the overhead here is not likely to be significant.

    while (tdd!=NULL)					// search to root of tree
    {
        const Style *s = tdd->style();			// get style from item
        //kDebug() << "style for" << tdi->name() << "is" << *s;
        if (s->hasLineColour())				// has a style set?
        {
            //kDebug() << "inherited from" << tdi->name();
            return (s->lineColour());			// use colour from that
        }
        tdd = dynamic_cast<const TrackDataDisplayable *>(tdd->parent());
    }							// up to parent item

    // TODO: project global style?
    return (Style::globalStyle()->lineColour());	// finally application default
}


void MapView::slotSystemPaletteChanged()
{
    bool syscol = Settings::selectedUseSystemColours();
    kDebug() << "using system?" << syscol;
    if (syscol) update();
}
