
#ifndef TRACKPROPERTIESDIALOGUE_H
#define TRACKPROPERTIESDIALOGUE_H


#include <qlist.h>

#include <kdialog.h>

class KTabWidget;

class TrackDataItem;
class TrackItemGeneralPage;
class TrackItemDetailPage;
class TrackItemStylePage;

class Style;




class TrackPropertiesDialogue : public KDialog
{
    Q_OBJECT

public:
    TrackPropertiesDialogue(const QList<TrackDataItem *> &items, QWidget *pnt = NULL);
    virtual ~TrackPropertiesDialogue();

    QString newItemName() const;
    QString newItemDesc() const;
    Style newStyle() const;
    QString newTrackType() const;

protected slots:
    void slotDataChanged();

private:
    KTabWidget *mTabWidget;

    TrackItemGeneralPage *mGeneralPage;
    TrackItemDetailPage *mDetailPage;
    TrackItemStylePage *mStylePage;



};

#endif							// TRACKPROPERTIESDIALOGUE_H
