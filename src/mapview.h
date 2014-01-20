// -*-mode:c++ -*-

#ifndef MAPVIEW_H
#define MAPVIEW_H
 
#include <marble/MarbleWidget.h>
#include <marble/ReverseGeocodingRunnerManager.h>

using namespace Marble;


class KAction;
class KConfigGroup;

class PointsModel;
class MainWindow;


class MapView : public MarbleWidget
{
    Q_OBJECT

public:
    MapView(QWidget *parent = NULL);
    ~MapView();

    void readProperties(const KConfigGroup &grp);
    void saveProperties(KConfigGroup &grp);

    void setModel(PointsModel *mdl)		{ mModel = mdl; }

    QStringList overlays(bool visibleOnly) const;
    KAction *actionForOverlay(const QString &id) const;
    void showOverlays(const QStringList &list);

public slots:               
    void slotRmbRequest(int x, int y);
    void slotFindAddress();
    void slotShowOverlay();

protected:
    virtual void customPaint(GeoPainter* painter);

protected slots:

private:
    MainWindow *mainWindow() const		{ return (mMainWindow); }

    bool mouseCoordinates(GeoDataCoordinates *coords);

private slots:
    void slotShowAddressInformation(const GeoDataCoordinates &coords, const GeoDataPlacemark &placemark);

private:
    PointsModel *mModel;
    MainWindow *mMainWindow;
    ReverseGeocodingRunnerManager *mRunnerManager;
    int mPopupX;
    int mPopupY;
};


#endif							// MAPVIEW_H
