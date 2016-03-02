
#ifndef STOPDETECTDIALOGUE_H
#define STOPDETECTDIALOGUE_H


#include <kdialog.h>

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


class StopDetectDialogue : public KDialog
{
    Q_OBJECT

public:
    explicit StopDetectDialogue(QWidget *pnt = NULL);
    virtual ~StopDetectDialogue();

protected:
    void showEvent(QShowEvent *ev);

    // TODO: subclass MainWindowInterface
    // that has mainWindow(), looks up through Qt object tree
    // cache for later access
    MainWindow *mainWindow() const			{ return (mMainWindow); }

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

    MainWindow *mMainWindow;

    QList<const TrackDataWaypoint *> mResultPoints;
};

#endif							// STOPDETECTDIALOGUE_H
