// -*-mode:c++ -*-

#ifndef COMMANDS_H
#define COMMANDS_H
 

#include <QUndoCommand>

#include "trackdata.h"
#include "filescontroller.h"
#include "style.h"





// abstract
class CommandBase : public QUndoCommand
{
public:
    virtual ~CommandBase()				{}
    virtual void undo() = 0;
    virtual void redo() = 0;

    static QString senderText(const QObject *sdr);
    void setSenderText(const QObject *sdr);

protected:
    CommandBase(QUndoCommand *parent = NULL) : QUndoCommand(parent)	{};
};





// abstract
class FilesCommandBase : public CommandBase
{
public:
    virtual ~FilesCommandBase()				{}

protected:
    FilesCommandBase(FilesController *fc, QUndoCommand *parent = NULL)
        : CommandBase(parent),
          mController(fc)				{}

    FilesController *controller() const			{ return (mController); }
    FilesModel *model() const				{ return (mController->model()); }

    void updateMap() const;

private:
    FilesController *mController;
};




class ImportFileCommand : public FilesCommandBase
{
public:
    ImportFileCommand(FilesController *fc, QUndoCommand *parent = NULL);
    virtual ~ImportFileCommand();

    void setData(TrackDataFile *tdf)			{ mTrackData = tdf; }

    void redo();
    void undo();

private:
    TrackDataFile *mTrackData;
    int mSavedCount;
};





class ChangeItemCommand : public FilesCommandBase
{
public:
    virtual ~ChangeItemCommand()			{}

    void setDataItem(TrackDataItem *item)		{ mDataItem = item; }

protected:
    ChangeItemCommand(FilesController *fc, QUndoCommand *parent = NULL);

protected:
    TrackDataItem *mDataItem;
    bool mFileWasModified;
};







class ChangeItemNameCommand : public ChangeItemCommand
{
public:
    ChangeItemNameCommand(FilesController *fc, QUndoCommand *parent = NULL)
        : ChangeItemCommand(fc, parent)			{}
    virtual ~ChangeItemNameCommand()			{}

    void setData(const QString &name)			{ mNewName = name; }

    void redo();
    void undo();

private:
    QString mNewName;
    QString mSavedName;
};




class ChangeItemStyleCommand : public ChangeItemCommand
{
public:
    ChangeItemStyleCommand(FilesController *fc, QUndoCommand *parent = NULL)
        : ChangeItemCommand(fc, parent)			{}
    virtual ~ChangeItemStyleCommand()			{}

    void setData(const Style &style)			{ mNewStyle = style; }

    void redo();
    void undo();

private:
    Style mNewStyle;
    Style mSavedStyle;
};




class ChangeItemDataCommand : public ChangeItemCommand
{
public:
    ChangeItemDataCommand(FilesController *fc, QUndoCommand *parent = NULL)
        : ChangeItemCommand(fc, parent)			{}
    virtual ~ChangeItemDataCommand()			{}

    void setData(const QString &key,
                 const QString &value)			{ mKey = key; mNewValue = value; }

    void redo();
    void undo();

private:
    QString mKey;
    QString mNewValue;
    QString mSavedValue;
};





class SplitSegmentCommand : public FilesCommandBase
{
public:
    SplitSegmentCommand(FilesController *fc, QUndoCommand *parent = NULL);
    virtual ~SplitSegmentCommand();

    void setData(TrackDataSegment *pnt, int idx);

    void redo();
    void undo();

private:
    TrackDataSegment *mParentSegment;
    int mSplitIndex;
    TrackDataSegment *mNewSegment;
};



class MergeSegmentsCommand : public FilesCommandBase
{
public:
    MergeSegmentsCommand(FilesController *fc, QUndoCommand *parent = NULL);
    virtual ~MergeSegmentsCommand();

    void setData(TrackDataSegment *master, const QList<TrackDataItem *> &others);

    void redo();
    void undo();

private:
    TrackDataSegment *mMasterSegment;
    QList<TrackDataItem *> mOtherSegments;
    QVector<TrackDataItem *> mOtherParents;
    QVector<int> mOtherCounts;
    QVector<int> mOtherIndexes;
};



class AddTrackCommand : public FilesCommandBase
{
public:
    AddTrackCommand(FilesController *fc, QUndoCommand *parent = NULL);
    virtual ~AddTrackCommand();

    void redo();
    void undo();

private:
    TrackDataTrack *mNewTrack;
};



class MoveSegmentCommand : public FilesCommandBase
{
public:
    MoveSegmentCommand(FilesController *fc, QUndoCommand *parent = NULL);
    virtual ~MoveSegmentCommand();

    void setData(TrackDataSegment *tds, TrackDataTrack *destTrack);

    void redo();
    void undo();

private:
    TrackDataSegment *mMoveSegment;
    TrackDataItem *mOrigTrack;
    TrackDataTrack *mDestTrack;
    int mOrigIndex;
};



class DeleteItemsCommand : public FilesCommandBase
{
public:
    DeleteItemsCommand(FilesController *fc, QUndoCommand *parent = NULL);
    virtual ~DeleteItemsCommand();

    void setData(const QList<TrackDataItem *> &items);

    void redo();
    void undo();

private:
    QList<TrackDataItem *> mItems;
    QVector<TrackDataItem *> mParentItems;
    QVector<int> mParentIndexes;
};




class MovePointsCommand : public FilesCommandBase
{
public:
    MovePointsCommand(FilesController *fc, QUndoCommand *parent = NULL)
        : FilesCommandBase(fc, parent)			{}
    virtual ~MovePointsCommand()			{}

    void setDataItems(const QList<TrackDataItem *> &items);
    void setData(qreal latOff, qreal lonOff)		{ mLatOff = latOff; mLonOff = lonOff; }

    void redo();
    void undo();

private:
    QList<TrackDataItem *> mItems;
    qreal mLatOff;
    qreal mLonOff;
};









#endif							// COMMANDS_H
