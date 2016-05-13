
#ifndef CREATEWAYPOINTDIALOGUE_H
#define CREATEWAYPOINTDIALOGUE_H


#include <dialogbase.h>


class QTreeView;
class QLineEdit;
class TrackDataAbstractPoint;
class TrackDataFolder;
class FilesController;
class LatLongWidget;


class CreateWaypointDialogue : public DialogBase
{
    Q_OBJECT

public:
    explicit CreateWaypointDialogue(FilesController *fc, QWidget *pnt = nullptr);
    virtual ~CreateWaypointDialogue() = default;

    void setSourcePoint(const TrackDataAbstractPoint *point);
    void setSourceLatLong(double lat, double lon);
    void setDestinationFolder(const TrackDataFolder *folder);

    TrackDataFolder *selectedFolder() const;
    QString waypointName() const;
    void waypointPosition(qreal *latp, qreal *lonp);

private slots:
    void slotSetButtonStates();

private:
    QLineEdit *mNameEdit;
    LatLongWidget *mLatLongEdit;
    QTreeView *mFolderList;
};

#endif							// CREATEWAYPOINTDIALOGUE_H
