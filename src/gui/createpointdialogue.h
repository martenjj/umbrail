
#ifndef CREATEPOINTDIALOGUE_H
#define CREATEPOINTDIALOGUE_H


#include <kfdialog/dialogbase.h>
#include "applicationdatainterface.h"


class QTreeView;
class QLineEdit;
class TrackDataAbstractPoint;
class TrackDataItem;
class LatLongWidget;


class CreatePointDialogue : public DialogBase, public ApplicationDataInterface
{
    Q_OBJECT

public:
    explicit CreatePointDialogue(bool routeMode, QWidget *pnt = nullptr);
    virtual ~CreatePointDialogue() = default;

    void setSourcePoint(const TrackDataAbstractPoint *point);
    void setSourceLatLong(double lat, double lon);
    void setDestinationContainer(const TrackDataItem *item);

    TrackDataItem *selectedContainer() const;
    QString pointName() const;
    void pointPosition(qreal *latp, qreal *lonp);

    bool canCreate() const				{ return (mCanCreate); }

private slots:
    void slotSetButtonStates();

private:
    QLineEdit *mNameEdit;
    LatLongWidget *mLatLongEdit;
    QTreeView *mContainerList;
    bool mCanCreate;
};

#endif							// CREATEPOINTDIALOGUE_H
