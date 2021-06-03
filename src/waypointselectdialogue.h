
#ifndef WAYPOINTSELECTDIALOGUE_H
#define WAYPOINTSELECTDIALOGUE_H


#include <kfdialog/dialogbase.h>


class QButtonGroup;


class WaypointSelectDialogue : public DialogBase
{
    Q_OBJECT

public:
    enum Selection
    {
        SelectWaypoints = 0x01,
        SelectRoutepoints = 0x02,
        SelectStops = 0x04,
        SelectAudioNotes = 0x08,
        SelectVideoNotes = 0x10,
        SelectPhotos = 0x20
    };
    Q_DECLARE_FLAGS(SelectionSet, Selection)

    WaypointSelectDialogue(QWidget *pnt = nullptr);
    virtual ~WaypointSelectDialogue() = default;

    void setSelection(WaypointSelectDialogue::SelectionSet sel);
    WaypointSelectDialogue::SelectionSet selection() const;

private:
    QButtonGroup *mGroup;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(WaypointSelectDialogue::SelectionSet)

#endif							// WAYPOINTSELECTDIALOGUE_H
