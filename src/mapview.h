// -*-mode:c++ -*-

#ifndef MAPVIEW_H
#define MAPVIEW_H
 
#include <marble/MarbleWidget.h>
#include <marble/ReverseGeocodingRunnerManager.h>

using namespace Marble;


class QElapsedTimer;
class KAction;
class FilesModel;
class FilesView;
class MainWindow;
class TrackDataItem;
class TrackDataPoint;


class MapView : public MarbleWidget
{
    Q_OBJECT

public:
    MapView(QWidget *pnt = NULL);
    virtual ~MapView();

    void readProperties();
    void saveProperties();

    void setFilesModel(FilesModel *mod)		{ mFilesModel = mod; }
    void setFilesView(FilesView *view)		{ mFilesView = view; }

    QString currentPosition() const;
    void setCurrentPosition(const QString &str);

    QStringList overlays(bool visibleOnly) const;
    KAction *actionForOverlay(const QString &id) const;
    void showOverlays(const QStringList &list);

    static QColor resolveLineColour(const TrackDataItem *tdd);

public slots:               
    void slotRmbRequest(int mx, int my);
    void slotFindAddress();
    void slotShowOverlay();

protected:
    virtual void customPaint(GeoPainter* painter);
    bool eventFilter(QObject *obj, QEvent *ev);

private:
    MainWindow *mainWindow() const		{ return (mMainWindow); }
    FilesModel *filesModel() const		{ return (mFilesModel); }
    FilesView *filesView() const		{ return (mFilesView); }

    bool mouseCoordinates(GeoDataCoordinates *coords);
    const TrackDataPoint *findClickedPoint(const TrackDataItem *tdi);

    void paintDataTree(const TrackDataItem *tdi, GeoPainter *painter, bool doSelected, bool parentSelected);

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
    unsigned long mSelectionId;

    QElapsedTimer *mClickTimer;
    int mClickX;
    int mClickY;
    const TrackDataPoint *mClickedPoint;

    double mLatMin;
    double mLonMin;
    double mLatMax;
    double mLonMax;
};


#endif							// MAPVIEW_H
