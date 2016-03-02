// -*-mode:c++ -*-

#ifndef MAPVIEW_H
#define MAPVIEW_H
 
#include <marble/MarbleWidget.h>
#include <marble/ReverseGeocodingRunnerManager.h>

using namespace Marble;

class KAction;
class FilesModel;
class FilesView;
class MainWindow;
class TrackDataItem;
class TrackDataWaypoint;
class TracksLayer;
class WaypointsLayer;
class StopsLayer;


class MapView : public MarbleWidget
{
    Q_OBJECT

public:
    MapView(QWidget *pnt = NULL);
    virtual ~MapView();

    void readProperties();
    void saveProperties();

    QString currentPosition() const;
    void setCurrentPosition(const QString &str);

    QStringList overlays(bool visibleOnly) const;
    KAction *actionForOverlay(const QString &id) const;
    void showOverlays(const QStringList &list);
    void setMovePointsMode(bool on);

    static QColor resolveLineColour(const TrackDataItem *tdd);

    void setFilesModel(FilesModel *mod)			{ mFilesModel = mod; }
    void setFilesView(FilesView *view)			{ mFilesView = view; }

    void setStopLayerData(const QList<const TrackDataWaypoint *> *data);

    MainWindow *mainWindow() const			{ return (mMainWindow); }
    FilesModel *filesModel() const			{ return (mFilesModel); }
    FilesView *filesView() const			{ return (mFilesView); }

public slots:               
    void slotRmbRequest(int mx, int my);
    void slotFindAddress();
    void slotShowOverlay();
    void slotAddWaypoint();

protected:
    bool eventFilter(QObject *obj, QEvent *ev);

signals:
    void draggedPoints(qreal latOff, qreal lonOff);
    void createWaypoint(qreal lat, qreal lon);

private:
    bool mouseCoordinates(GeoDataCoordinates *coords) const;

private slots:
    void slotShowAddressInformation(const GeoDataCoordinates &coords, const GeoDataPlacemark &placemark);
    void slotSystemPaletteChanged();

private:
    FilesModel *mFilesModel;
    FilesView *mFilesView;
    MainWindow *mMainWindow;
    ReverseGeocodingRunnerManager *mRunnerManager;
    int mPopupX;
    int mPopupY;

    TracksLayer *mTracksLayer;
    WaypointsLayer *mWaypointsLayer;
    StopsLayer *mStopsLayer;
};

#endif							// MAPVIEW_H
