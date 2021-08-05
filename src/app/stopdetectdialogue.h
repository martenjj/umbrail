
#ifndef STOPDETECTDIALOGUE_H
#define STOPDETECTDIALOGUE_H


#include <kfdialog/dialogbase.h>
#include "applicationdatainterface.h"

#include <qtimezone.h>


class QTimer;
class QListWidget;
class QLineEdit;
class QPushButton;
class QShowEvent;
class ValueSlider;
class TrackDataItem;
class TrackDataAbstractPoint;
class TrackDataWaypoint;
class FolderSelectWidget;


class StopDetectDialogue : public DialogBase, public ApplicationDataInterface
{
    Q_OBJECT

public:
    explicit StopDetectDialogue(QWidget *pnt = nullptr);
    virtual ~StopDetectDialogue();

protected:
    void showEvent(QShowEvent *ev) override;

protected slots:
    void slotShowOnMap();
    void slotMergeStops();
    void slotCommitResults();

private slots:
    void slotDetectStops();
    void slotSetButtonStates();

private:
    void updateResults();

private:
    QListWidget *mResultsList;
    ValueSlider *mTimeSlider;
    ValueSlider *mDistanceSlider;
    ValueSlider *mNoiseSlider;
    QPushButton *mShowOnMapButton;
    QPushButton *mMergeStopsButton;
    FolderSelectWidget *mFolderSelect;

    QTimer *mIdleTimer;

    QTimeZone mTimeZone;

    QVector<const TrackDataAbstractPoint *> mInputPoints;
    QList<const TrackDataWaypoint *> mResultPoints;
};

#endif							// STOPDETECTDIALOGUE_H
