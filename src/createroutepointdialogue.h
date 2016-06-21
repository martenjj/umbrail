
#ifndef CREATEROUTEPOINTDIALOGUE_H
#define CREATEROUTEPOINTDIALOGUE_H


#include <dialogbase.h>


class QTreeView;
class QLineEdit;
class TrackDataAbstractPoint;
class TrackDataRoute;
class FilesController;
class LatLongWidget;


class CreateRoutepointDialogue : public DialogBase
{
    Q_OBJECT

public:
    explicit CreateRoutepointDialogue(FilesController *fc, QWidget *pnt = NULL);
    virtual ~CreateRoutepointDialogue() = default;

    void setSourcePoint(const TrackDataAbstractPoint *point);
    void setSourceLatLong(double lat, double lon);
    void setDestinationRoute(const TrackDataRoute *route);

    TrackDataRoute *selectedRoute() const;
    QString routepointName() const;
    void routepointPosition(qreal *latp, qreal *lonp);

private slots:
    void slotSetButtonStates();

private:
    QLineEdit *mNameEdit;
    LatLongWidget *mLatLongEdit;
    QTreeView *mRouteList;
};

#endif							// CREATEROUTEPOINTDIALOGUE_H
