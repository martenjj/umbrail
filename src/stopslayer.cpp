
#include "stopslayer.h"

#include <kdebug.h>
#include <klocale.h>
#include <kcolorscheme.h>
#include <kiconloader.h>

#include <marble/GeoDataCoordinates.h>
#include <marble/GeoPainter.h>

#include "trackdata.h"

//////////////////////////////////////////////////////////////////////////
//									//
//  Debugging switches							//
//									//
//////////////////////////////////////////////////////////////////////////

#undef DEBUG_PAINTING

//////////////////////////////////////////////////////////////////////////
//									//
//									//
//////////////////////////////////////////////////////////////////////////

StopsLayer::StopsLayer(QWidget *pnt)
{
    kDebug();
    mStopsData = NULL;
}


StopsLayer::~StopsLayer()
{
    kDebug() << "done";
}


QStringList StopsLayer::renderPosition() const
{
    return (QStringList("HOVERS_ABOVE_SURFACE"));
}


bool StopsLayer::render(GeoPainter *painter, ViewportParams *viewport,
                        const QString &renderPos, GeoSceneLayer *layer)
{
    if (mStopsData==NULL) return (true);		// nothing to draw

    for (int i = 0; i<mStopsData->count(); ++i)
    {
        const TrackDataStop *tds = mStopsData->at(i);

#ifdef DEBUG_PAINTING
        kDebug() << "draw stop" << i << tds->name();
#endif
        GeoDataCoordinates coord(tds->longitude(), tds->latitude(),
                                 0, GeoDataCoordinates::Degree);

        // First the icon image
        const QPixmap img = KIconLoader::global()->loadIcon(tds->iconName(), KIconLoader::NoGroup, KIconLoader::SizeSmall,
                                                            KIconLoader::DefaultState, QStringList(), NULL, true);
        if (!img.isNull())				// icon image available
        {
            painter->drawPixmap(coord, img);
        }
        else						// draw our own marker
        {
            painter->setPen(QPen(Qt::red, 2));
            painter->setBrush(Qt::yellow);
            painter->drawEllipse(coord, 12, 12);
        }

        // Finally the waypoint text
        painter->save();
        painter->translate(15, 3);			// offset text from point
        painter->setPen(Qt::gray);			// draw with drop shadow
        painter->drawText(coord, tds->name());
        painter->translate(-1, -1);
        painter->setPen(Qt::black);
        painter->drawText(coord, tds->name());
        painter->restore();
    }

    return (true);
}


void StopsLayer::setStopsData(const QList<const TrackDataStop *> *data)
{
    mStopsData = data;
    if (mStopsData==NULL) kDebug() << "data cleared";
    else kDebug() << "data set" << mStopsData->count() << "points";
}
