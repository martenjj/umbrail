
#ifndef STOPDETECTDIALOGUE_H
#define STOPDETECTDIALOGUE_H


#include <dialogbase.h>
#include "mainwindowinterface.h"


class QTimer;
class QListWidget;
class QLineEdit;
class QPushButton;
class QShowEvent;
class MainWindow;
class ValueSlider;
class TrackDataItem;
class TrackDataAbstractPoint;
class TrackDataWaypoint;
class FolderSelectWidget;


class StopDetectDialogue : public DialogBase, public MainWindowInterface
{
    Q_OBJECT

public:
    explicit StopDetectDialogue(QWidget *pnt = nullptr);
    virtual ~StopDetectDialogue();

protected:
    void showEvent(QShowEvent *ev) override;

protected slots:
    void slotShowOnMap();
    void slotCommitResults();

private slots:
    void slotDetectStops();
    void slotSetButtonStates();

private:
    QListWidget *mResultsList;
    ValueSlider *mTimeSlider;
    ValueSlider *mDistanceSlider;
    ValueSlider *mNoiseSlider;
    QPushButton *mShowOnMapButton;
    FolderSelectWidget *mFolderSelect;

    QTimer *mIdleTimer;

    QVector<const TrackDataAbstractPoint *> mInputPoints;
    QList<const TrackDataWaypoint *> mResultPoints;
};

#endif							// STOPDETECTDIALOGUE_H
