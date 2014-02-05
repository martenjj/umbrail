
#ifndef TRACKPROPERTIESDIALOGUE_H
#define TRACKPROPERTIESDIALOGUE_H


#include <qlist.h>

#include <kpagedialog.h>

class TrackDataItem;
class TrackItemGeneralPage;
class TrackItemStylePage;






class TrackPropertiesDialogue : public KPageDialog
{
    Q_OBJECT

public:
    TrackPropertiesDialogue(const QList<TrackDataItem *> &items, QWidget *pnt = NULL);
    virtual ~TrackPropertiesDialogue();

    QString newItemName() const;
    KUrl newFileUrl() const;

private:

private:
    TrackItemGeneralPage *mGeneralPage;
//    TrackItemStylePage *mStylePage;



};

#endif							// TRACKPROPERTIESDIALOGUE_H
