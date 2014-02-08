// -*-mode:c++ -*-

#ifndef MAPCONTROLLER_H
#define MAPCONTROLLER_H
 
#include <qobject.h>

//#include "pointdata.h"

#include <marble/MapThemeManager.h>

using namespace Marble;

class KConfig;

class MapView;
class MainWindow;
class TrackDataItem;



class MapController : public QObject
{
    Q_OBJECT

public:
    MapController(QObject *parent = NULL);
    ~MapController();

    MapView *view() const			{ return (mView); }

    void readProperties();
    void saveProperties();

    QString save(KConfig *conf);
    QString load(const KConfig *conf);
    void clear();

    void gotoSelection(const QList<TrackDataItem *> &items);

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

private:
    MainWindow *mainWindow() const;

private:
    MapView *mView;
    MapThemeManager *mThemeManager;

    double mHomeLat;
    double mHomeLong;
    double mHomeZoom;
};
 
#endif							// MAPCONTROLLER_H
