
#ifndef STOPDETECTDIALOGUE_H
#define STOPDETECTDIALOGUE_H


#include <kdialog.h>
#include "mainwindowinterface.h"

class QTimer;
class QListWidget;
class QLineEdit;
class QPushButton;
class QShowEvent;
class MainWindow;
class ValueSlider;
class TrackDataItem;
class TrackDataTrackpoint;
class TrackDataWaypoint;
class FolderSelectWidget;


class StopDetectDialogue : public KDialog, public MainWindowInterface
{
    Q_OBJECT

public:
    explicit StopDetectDialogue(QWidget *pnt = NULL);
    virtual ~StopDetectDialogue();

protected:
    void showEvent(QShowEvent *ev);

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

    QList<const TrackDataWaypoint *> mResultPoints;
};

#endif							// STOPDETECTDIALOGUE_H
