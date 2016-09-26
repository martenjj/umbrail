
#ifndef TRACKPROPERTIESDIALOGUE_H
#define TRACKPROPERTIESDIALOGUE_H


#include <qlist.h>

#include <dialogbase.h>
#include <dialogstatesaver.h>

#include "trackdata.h"

class QTabWidget;

class TrackDataItem;
class Style;

class TrackItemGeneralPage;
class TrackItemDetailPage;
class TrackItemStylePage;
class TrackItemMetadataPage;


class TrackPropertiesDialogue : public DialogBase, public DialogStateSaver
{
    Q_OBJECT

public:
    TrackPropertiesDialogue(const QList<TrackDataItem *> *items, QWidget *pnt = NULL);
    virtual ~TrackPropertiesDialogue() = default;

    QString newItemName() const;
    QString newItemDesc() const;
    Style newStyle() const;
    QString newTrackType() const;
    QString newTimeZone() const;
    bool newPointPosition(double *newLat, double *newLon) const;
    TrackData::WaypointStatus newWaypointStatus() const;

    void saveConfig(QDialog *dialog, KConfigGroup &grp) const;
    void restoreConfig(QDialog *dialog, const KConfigGroup &grp);

protected slots:
    void slotDataChanged();

private:
    QTabWidget *mTabWidget;

    TrackItemGeneralPage *mGeneralPage;
    TrackItemDetailPage *mDetailPage;
    TrackItemStylePage *mStylePage;
    TrackItemMetadataPage *mMetadataPage;
};

#endif							// TRACKPROPERTIESDIALOGUE_H
