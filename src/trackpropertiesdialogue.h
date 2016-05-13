
#ifndef TRACKPROPERTIESDIALOGUE_H
#define TRACKPROPERTIESDIALOGUE_H


#include <qlist.h>

#include <dialogbase.h>
#include <dialogstatesaver.h>

#include "trackdata.h"

class KTabWidget;

class TrackDataItem;
class Style;

class TrackItemGeneralPage;
class TrackItemDetailPage;
class TrackItemStylePage;
class TrackItemMetadataPage;


class TrackPropertiesStateSaver : public DialogStateSaver
{
    Q_OBJECT

public:
    TrackPropertiesStateSaver(QDialog *pnt) : DialogStateSaver(pnt)	{}
    virtual ~TrackPropertiesStateSaver() = default;

protected:
    void saveConfig(QDialog *dialog, KConfigGroup &grp) const;
    void restoreConfig(QDialog *dialog, const KConfigGroup &grp);
};


class TrackPropertiesDialogue : public DialogBase
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

    KTabWidget *tabWidget() const			{ return (mTabWidget); }

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
