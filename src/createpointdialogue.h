
#ifndef CREATEPOINTDIALOGUE_H
#define CREATEPOINTDIALOGUE_H


#include <dialogbase.h>


class QTreeView;
class QLineEdit;
class TrackDataAbstractPoint;
class TrackDataItem;
class FilesController;
class LatLongWidget;


class CreatePointDialogue : public DialogBase
{
    Q_OBJECT

public:
    explicit CreatePointDialogue(FilesController *fc, bool routeMode, QWidget *pnt = nullptr);
    virtual ~CreatePointDialogue() = default;

    void setSourcePoint(const TrackDataAbstractPoint *point);
    void setSourceLatLong(double lat, double lon);
    void setDestinationContainer(const TrackDataItem *item);

    TrackDataItem *selectedContainer() const;
    QString pointName() const;
    void pointPosition(qreal *latp, qreal *lonp);

private slots:
    void slotSetButtonStates();

private:
    QLineEdit *mNameEdit;
    LatLongWidget *mLatLongEdit;
    QTreeView *mContainerList;
};

#endif							// CREATEPOINTDIALOGUE_H
