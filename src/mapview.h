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


class MapView : public MarbleWidget
{
    Q_OBJECT

public:
    MapView(QWidget *pnt = NULL);
    ~MapView();

    void readProperties();
    void saveProperties();

    void setFilesModel(FilesModel *mod)		{ mFilesModel = mod; }
    void setFilesView(FilesView *view)		{ mFilesView = view; }

    QStringList overlays(bool visibleOnly) const;
    KAction *actionForOverlay(const QString &id) const;
    void showOverlays(const QStringList &list);

    static QColor resolveLineColour(const TrackDataItem *tdi);

public slots:               
    void slotRmbRequest(int mx, int my);
    void slotFindAddress();
    void slotShowOverlay();

protected:
    virtual void customPaint(GeoPainter* painter);

protected slots:

private:
    MainWindow *mainWindow() const		{ return (mMainWindow); }
    FilesModel *filesModel() const		{ return (mFilesModel); }
    FilesView *filesView() const		{ return (mFilesView); }

    bool mouseCoordinates(GeoDataCoordinates *coords);

    void paintDataTree(const TrackDataItem *tdi, GeoPainter *painter, bool parentSelected);

private slots:
    void slotShowAddressInformation(const GeoDataCoordinates &coords, const GeoDataPlacemark &placemark);

private:
    FilesModel *mFilesModel;
    FilesView *mFilesView;
    MainWindow *mMainWindow;
    ReverseGeocodingRunnerManager *mRunnerManager;
    int mPopupX;
    int mPopupY;
    unsigned long mSelectionId;
};


#endif							// MAPVIEW_H
