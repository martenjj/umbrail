
#ifndef MOVESEGMENTDIALOGUE_H
#define MOVESEGMENTDIALOGUE_H


#include <kdialog.h>


class QTreeView;
class QItemSelection;
class TrackDataTrack;
class TrackDataSegment;
class FilesController;



class MoveSegmentDialogue : public KDialog
{
    Q_OBJECT

public:
    explicit MoveSegmentDialogue(FilesController *fc, QWidget *pnt = NULL);
    virtual ~MoveSegmentDialogue();

    void setSegment(const TrackDataSegment *seg);
    TrackDataTrack *selectedTrack() const;

protected slots:
    void slotSelectionChanged(const QItemSelection &sel, const QItemSelection &desel);

private:
    QTreeView *mTrackList;
    const TrackDataSegment *mSegment;
};

#endif							// MOVESEGMENTDIALOGUE_H
