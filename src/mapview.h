// -*-mode:c++ -*-

#ifndef MAPVIEW_H
#define MAPVIEW_H
 
#include <marble/MarbleWidget.h>
#include <marble/ReverseGeocodingRunnerManager.h>

using namespace Marble;


class KAction;

class FilesModel;
class MainWindow;
class TrackDataItem;


class MapView : public MarbleWidget
{
    Q_OBJECT

public:
    MapView(QWidget *parent = NULL);
    ~MapView();

    void readProperties();
    void saveProperties();

    void setFilesModel(FilesModel *mdl)		{ mModel = mdl; }

    QStringList overlays(bool visibleOnly) const;
    KAction *actionForOverlay(const QString &id) const;
    void showOverlays(const QStringList &list);

public slots:               
    void slotRmbRequest(int mx, int my);
    void slotFindAddress();
    void slotShowOverlay();

protected:
    virtual void customPaint(GeoPainter* painter);

protected slots:

private:
    MainWindow *mainWindow() const		{ return (mMainWindow); }
    FilesModel *filesModel() const		{ return (mModel); }

    bool mouseCoordinates(GeoDataCoordinates *coords);

    void paintDataTree(const TrackDataItem *tdi, GeoPainter *painter);

private slots:
    void slotShowAddressInformation(const GeoDataCoordinates &coords, const GeoDataPlacemark &placemark);

private:
    FilesModel *mModel;
    MainWindow *mMainWindow;
    ReverseGeocodingRunnerManager *mRunnerManager;
    int mPopupX;
    int mPopupY;
};


#endif							// MAPVIEW_H
