// -*-mode:c++ -*-

#ifndef COMMANDS_H
#define COMMANDS_H
 

#include <QUndoCommand>

#include "trackdata.h"
#include "filescontroller.h"
#include "style.h"


class ItemContainer;


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
    void startLayoutChange() const;
    void endLayoutChange() const;

private:
    FilesController *mController;
};




class ImportFileCommand : public FilesCommandBase
{
public:
    ImportFileCommand(FilesController *fc, QUndoCommand *parent = NULL);
    virtual ~ImportFileCommand();

    void setData(TrackDataFile *tdf)			{ mImportData = tdf; }

    void redo();
    void undo();

private:
    TrackDataFile *mImportData;
    int mSavedCount;
};





class ChangeItemCommand : public FilesCommandBase
{
public:
    virtual ~ChangeItemCommand()			{}

    void setDataItem(TrackDataItem *item)			{ mDataItems.clear(); mDataItems.append(item); }
    void setDataItems(const QList<TrackDataItem *> &items)	{ mDataItems = items; }

protected:
    ChangeItemCommand(FilesController *fc, QUndoCommand *parent = NULL);

protected:
    QList<TrackDataItem *> mDataItems;
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
    QStringList mSavedValues;
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
    ItemContainer *mNewSegmentContainer;
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
    QList<TrackDataItem *> mSourceSegments;
    QVector<TrackDataItem *> mSourceParents;
    QVector<int> mSourceCounts;
    QVector<int> mSourceIndexes;
    ItemContainer *mSavedSegmentContainer;
};



class AddContainerCommand : public FilesCommandBase
{
public:
    AddContainerCommand(FilesController *fc, QUndoCommand *parent = NULL);
    virtual ~AddContainerCommand();

    void redo();
    void undo();

    void setData(TrackData::Type type, TrackDataItem *pnt = NULL);
    void setName(const QString &name)			{ mAddName = name; }

private:
    TrackData::Type mType;
    TrackDataItem *mParent;
    ItemContainer *mNewItemContainer;
    QString mAddName;
};



class AddPointCommand : public FilesCommandBase
{
public:
    AddPointCommand(FilesController *fc, QUndoCommand *parent = NULL);
    virtual ~AddPointCommand();

    void redo();
    void undo();

    void setData(TrackDataItem *item);

private:
    ItemContainer *mNewPointContainer;
    TrackDataTrackpoint *mAtPoint;
};



class MoveItemCommand : public FilesCommandBase
{
public:
    MoveItemCommand(FilesController *fc, QUndoCommand *parent = NULL);
    virtual ~MoveItemCommand();

    void setData(const QList<TrackDataItem *> &items, TrackDataItem *dest);

    void redo();
    void undo();

private:
    QList<TrackDataItem *> mItems;
    QVector<TrackDataItem *> mParentItems;
    QVector<int> mParentIndexes;
    TrackDataItem *mDestination;
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
    ItemContainer *mDeletedItemsContainer;
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



class AddWaypointCommand : public FilesCommandBase
{
public:
    AddWaypointCommand(FilesController *fc, QUndoCommand *parent = NULL);
    virtual ~AddWaypointCommand();

    void setData(const QString &name, qreal lat, qreal lon,
                 TrackDataFolder *folder, const TrackDataAbstractPoint *sourcePoint = NULL);

    void redo();
    void undo();

protected:
    TrackDataFolder *mWaypointFolder;

private:
    QString mWaypointName;
    qreal mLatitude;
    qreal mLongitude;
    const TrackDataAbstractPoint *mSourcePoint;
    ItemContainer *mNewWaypointContainer;
};



class AddPhotoCommand : public AddWaypointCommand
{
public:
    AddPhotoCommand(FilesController *fc, QUndoCommand *parent = NULL)
        : AddWaypointCommand(fc, parent)		{}
    virtual ~AddPhotoCommand()				{}

    void setLink(const KUrl &link)			{ mLinkUrl = link; }
    void setTime(const QDateTime &dt)			{ mDateTime = dt; }

    void redo();
    void undo();

private:
    KUrl mLinkUrl;
    QDateTime mDateTime;
};


#endif							// COMMANDS_H
