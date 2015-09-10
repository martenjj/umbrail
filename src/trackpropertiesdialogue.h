
#ifndef TRACKPROPERTIESDIALOGUE_H
#define TRACKPROPERTIESDIALOGUE_H


#include <qlist.h>

#include <kdialog.h>

#include "trackdata.h"

class KTabWidget;

class TrackDataItem;
class Style;

class TrackItemGeneralPage;
class TrackItemDetailPage;
class TrackItemStylePage;
class TrackItemMetadataPage;


class TrackPropertiesDialogue : public KDialog
{
    Q_OBJECT

public:
    TrackPropertiesDialogue(const QList<TrackDataItem *> *items, QWidget *pnt = NULL);
    virtual ~TrackPropertiesDialogue();

    QString newItemName() const;
    QString newItemDesc() const;
    Style newStyle() const;
    QString newTrackType() const;
    QString newTimeZone() const;
    bool newPointPosition(double *newLat, double *newLon) const;
    TrackData::WaypointStatus newWaypointStatus() const;

protected slots:
    void slotDataChanged();

private:
    KTabWidget *mTabWidget;

    TrackItemGeneralPage *mGeneralPage;
    TrackItemDetailPage *mDetailPage;
    TrackItemStylePage *mStylePage;
    TrackItemMetadataPage *mMetadataPage;
};

#endif							// TRACKPROPERTIESDIALOGUE_H
