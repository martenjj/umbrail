// -*-mode:c++ -*-

#ifndef MAPCONTROLLER_H
#define MAPCONTROLLER_H
 
#include <qobject.h>
#include "applicationdatainterface.h"
#include "mapbrowser.h"

#include <marble/MapThemeManager.h>

using namespace Marble;

class KConfig;

class MapView;
class TrackDataItem;


class MapController : public QObject, public ApplicationDataInterface
{
    Q_OBJECT

public:
    explicit MapController(QObject *pnt = nullptr);
    virtual ~MapController();

    MapView *view() const			{ return (mView); }

    void readProperties();
    void saveProperties();

    void clear();

    void gotoSelection(const QList<TrackDataItem *> &items);
    void openExternalMap(MapBrowser::MapProvider map, const QList<TrackDataItem *> &items);

    static QString positionToString(double lat, double lon, int zoom);
    static bool positionFromString(const QString &str, double *plat, double *plon, int *pzoom);

public slots:               
    void slotGoHome();
    void slotSetHome();
    void slotResetZoom();
    void slotSetZoom();
    void slotSelectTheme();
    void slotSaveImage();
    void slotAboutMarble();

protected slots:
    void slotShowPosition( const QString &pos);
    void slotZoomChanged(int zoom);

signals:
    void statusMessage(const QString &text);
    void modified();

    void mapZoomChanged(bool canZoomIn, bool canZoomOut);
    void mapDraggedPoints(qreal latOff, qreal lonOff);

private slots:
    void slotMapThemeSelected(const QString &themeId);

private:
    MapView *mView;
    MapThemeManager *mThemeManager;

    double mHomeLat;
    double mHomeLong;
    int mHomeZoom;
};
 
#endif							// MAPCONTROLLER_H