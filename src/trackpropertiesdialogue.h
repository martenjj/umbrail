
#ifndef TRACKPROPERTIESDIALOGUE_H
#define TRACKPROPERTIESDIALOGUE_H


#include <qlist.h>

#include <dialogbase.h>
#include <dialogstatesaver.h>

#include "trackdata.h"

class QTabWidget;

class TrackDataItem;
//class Style;

class TrackItemGeneralPage;
class TrackItemDetailPage;
class TrackItemStylePage;
class TrackItemPlotPage;
class TrackItemMetadataPage;


class TrackPropertiesDialogue : public DialogBase, public DialogStateSaver
{
    Q_OBJECT

public:
    TrackPropertiesDialogue(const QList<TrackDataItem *> *items, QWidget *pnt = NULL);
    virtual ~TrackPropertiesDialogue() = default;

    QString newItemName() const;
    QString newItemDesc() const;
    QColor newColour() const;
    QString newTrackType() const;
    QString newTimeZone() const;
    bool newPointPosition(double *newLat, double *newLon) const;
    TrackData::WaypointStatus newWaypointStatus() const;
    QString newBearingData() const;
    QString newRangeData() const;

    static void setNextPageIndex(int page);

protected:
    void saveConfig(QDialog *dialog, KConfigGroup &grp) const override;
    void restoreConfig(QDialog *dialog, const KConfigGroup &grp) override;

private:
    void addPage(TrackPropertiesPage *page, const QString &title, bool enabled = true);

protected slots:
    void slotDataChanged();

private:
    QTabWidget *mTabWidget;
    TrackData::Type mItemType;

    TrackItemGeneralPage *mGeneralPage;
    TrackItemDetailPage *mDetailPage;
    TrackItemStylePage *mStylePage;
    TrackItemPlotPage *mPlotPage;
    TrackItemMetadataPage *mMetadataPage;
};

#endif							// TRACKPROPERTIESDIALOGUE_H
