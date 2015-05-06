
#ifndef CREATEWAYPOINTDIALOGUE_H
#define CREATEWAYPOINTDIALOGUE_H


#include <kdialog.h>


class QTreeView;
class QLineEdit;
class TrackDataAbstractPoint;
class TrackDataFolder;
class FilesController;
class LatLongWidget;


class CreateWaypointDialogue : public KDialog
{
    Q_OBJECT

public:
    explicit CreateWaypointDialogue(FilesController *fc, QWidget *pnt = NULL);
    virtual ~CreateWaypointDialogue();

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
