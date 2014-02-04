
#ifndef TRACKPROPERTIESDIALOGUE_H
#define TRACKPROPERTIESDIALOGUE_H


#include <qlist.h>

#include <kpagedialog.h>

class TrackDataItem;









class TrackPropertiesDialogue : public KPageDialog
{
    Q_OBJECT

public:
    TrackPropertiesDialogue(const QList<TrackDataItem *> &items, QWidget *pnt = NULL);
    virtual ~TrackPropertiesDialogue();

private:

private:




};

#endif							// TRACKPROPERTIESDIALOGUE_H
