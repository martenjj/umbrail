//////////////////////////////////////////////////////////////////////////
//									//
//  Project:	Umbrail - GPX track viewer and editor			//
//									//
//////////////////////////////////////////////////////////////////////////
//									//
//  Copyright (c) 2014-2021 Jonathan Marten <jjm@keelhaul.me.uk>	//
//  Home and download page: <http://github.com/martenjj/umbrail>	//
//									//
//  This program is free software; you can redistribute it and/or	//
//  modify it under the terms of the GNU General Public License as	//
//  published by the Free Software Foundation, either version 3 of	//
//  the License or (at your option) any later version.			//
//									//
//  It is distributed in the hope that it will be useful, but		//
//  WITHOUT ANY WARRANTY;  without even the implied warranty of		//
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the	//
//  GNU General Public License for more details.			//
//									//
//  You should have received a copy of the GNU General Public License	//
//  along with this program;  see the file COPYING for further		//
//  details.  If not, see <http://gnu.org/licenses/gpl>.      		//
//									//
//////////////////////////////////////////////////////////////////////////

#ifndef COMMANDS_H
#define COMMANDS_H
 

#include <QUndoCommand>

#include "trackdata.h"
#include "filescontroller.h"


class ItemContainer;


// abstract
class CommandBase : public QUndoCommand
{
public:
    virtual ~CommandBase()				{}
    virtual void undo() override = 0;
    virtual void redo() override = 0;

    static QString senderText(const QObject *sdr);
    void setSenderText(const QObject *sdr);

protected:
    CommandBase(QUndoCommand *parent = nullptr) : QUndoCommand(parent)	{};
};





// abstract
class FilesCommandBase : public CommandBase
{
public:
    virtual ~FilesCommandBase()				{}

protected:
    FilesCommandBase(FilesController *fc, QUndoCommand *parent = nullptr)
        : CommandBase(parent),
          mController(fc)				{}

    FilesController *controller() const			{ return (mController); }
    FilesModel *model() const				{ return (mController->model()); }

    void startLayoutChange() const;
    void endLayoutChange() const;

private:
    FilesController *mController;
};




class ImportFileCommand : public FilesCommandBase
{
public:
    ImportFileCommand(FilesController *fc, QUndoCommand *parent = nullptr);
    virtual ~ImportFileCommand();

    void setData(TrackDataFile *tdf)			{ mImportData = tdf; }

    void redo() override;
    void undo() override;

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
    ChangeItemCommand(FilesController *fc, QUndoCommand *parent = nullptr);

protected:
    QList<TrackDataItem *> mDataItems;
    bool mFileWasModified;
};







class ChangeItemNameCommand : public ChangeItemCommand
{
public:
    ChangeItemNameCommand(FilesController *fc, QUndoCommand *parent = nullptr)
        : ChangeItemCommand(fc, parent)			{}
    virtual ~ChangeItemNameCommand()			{}

    void setData(const QString &name)			{ mNewName = name; }

    void redo() override;
    void undo() override;

private:
    QString mNewName;
    QString mSavedName;
    bool mSavedExplicit;
};




class ChangeItemDataCommand : public ChangeItemCommand
{
public:
    ChangeItemDataCommand(FilesController *fc, QUndoCommand *parent = nullptr)
        : ChangeItemCommand(fc, parent)			{}
    virtual ~ChangeItemDataCommand()			{}

    void setData(const QByteArray &key,
                 const QVariant &value)			{ mKey = key; mNewValue = value; }

    void redo() override;
    void undo() override;

private:
    QByteArray mKey;
    QVariant mNewValue;
    QVariantList mSavedValues;
};





class SplitSegmentCommand : public FilesCommandBase
{
public:
    SplitSegmentCommand(FilesController *fc, QUndoCommand *parent = nullptr);
    virtual ~SplitSegmentCommand();

    void setData(TrackDataItem *pnt, int idx);

    void redo() override;
    void undo() override;

private:
    TrackDataItem *mParentSegment;
    int mSplitIndex;
    ItemContainer *mNewSegmentContainer;
};



class MergeSegmentsCommand : public FilesCommandBase
{
public:
    MergeSegmentsCommand(FilesController *fc, QUndoCommand *parent = nullptr);
    virtual ~MergeSegmentsCommand();

    void setData(TrackDataItem *master, const QList<TrackDataItem *> &others);

    void redo() override;
    void undo() override;

private:
    TrackDataItem *mMasterSegment;
    QList<TrackDataItem *> mSourceSegments;
    QVector<TrackDataItem *> mSourceParents;
    QVector<int> mSourceCounts;
    QVector<int> mSourceIndexes;
    ItemContainer *mSavedSegmentContainer;
};



class AddContainerCommand : public FilesCommandBase
{
public:
    AddContainerCommand(FilesController *fc, QUndoCommand *parent = nullptr);
    virtual ~AddContainerCommand();

    void redo() override;
    void undo() override;

    void setData(TrackData::Type type, TrackDataItem *pnt = nullptr);
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
    AddPointCommand(FilesController *fc, QUndoCommand *parent = nullptr);
    virtual ~AddPointCommand();

    void redo() override;
    void undo() override;

    void setData(TrackDataItem *item);

private:
    ItemContainer *mNewPointContainer;
    TrackDataTrackpoint *mAtPoint;
};



class MoveItemCommand : public FilesCommandBase
{
public:
    MoveItemCommand(FilesController *fc, QUndoCommand *parent = nullptr);
    virtual ~MoveItemCommand();

    void setData(const QList<TrackDataItem *> &items, TrackDataItem *dest, int row = -1);

    void redo() override;
    void undo() override;

private:
    QList<TrackDataItem *> mItems;
    QVector<TrackDataItem *> mParentItems;
    QVector<int> mParentIndexes;
    TrackDataItem *mDestinationParent;
    int mDestinationRow;
};



class DeleteItemsCommand : public FilesCommandBase
{
public:
    DeleteItemsCommand(FilesController *fc, QUndoCommand *parent = nullptr);
    virtual ~DeleteItemsCommand();

    void setData(const QList<TrackDataItem *> &items);

    void redo() override;
    void undo() override;

private:
    QList<TrackDataItem *> mItems;
    QVector<TrackDataItem *> mParentItems;
    QVector<int> mParentIndexes;
    ItemContainer *mDeletedItemsContainer;
};




class MovePointsCommand : public FilesCommandBase
{
public:
    MovePointsCommand(FilesController *fc, QUndoCommand *parent = nullptr)
        : FilesCommandBase(fc, parent)			{}
    virtual ~MovePointsCommand()			{}

    void setDataItems(const QList<TrackDataItem *> &items);
    void setData(qreal latOff, qreal lonOff)		{ mLatOff = latOff; mLonOff = lonOff; }

    void redo() override;
    void undo() override;

private:
    QList<TrackDataItem *> mItems;
    qreal mLatOff;
    qreal mLonOff;
};



class AddWaypointCommand : public FilesCommandBase
{
public:
    AddWaypointCommand(FilesController *fc, QUndoCommand *parent = nullptr);
    virtual ~AddWaypointCommand();

    void setData(const QString &name, qreal lat, qreal lon,
                 TrackDataFolder *folder, const TrackDataAbstractPoint *sourcePoint = nullptr);

    void redo() override;
    void undo() override;

protected:
    TrackDataFolder *mWaypointFolder;

private:
    QString mWaypointName;
    qreal mLatitude;
    qreal mLongitude;
    const TrackDataAbstractPoint *mSourcePoint;
    ItemContainer *mNewWaypointContainer;
};



class AddRoutepointCommand : public FilesCommandBase
{
public:
    AddRoutepointCommand(FilesController *fc, QUndoCommand *parent = nullptr);
    virtual ~AddRoutepointCommand();

    void setData(const QString &name, qreal lat, qreal lon,
                 TrackDataRoute *route, const TrackDataAbstractPoint *sourcePoint = nullptr);

    void redo() override;
    void undo() override;

protected:
    TrackDataRoute *mRoutepointRoute;

private:
    QString mRoutepointName;
    qreal mLatitude;
    qreal mLongitude;
    const TrackDataAbstractPoint *mSourcePoint;
    ItemContainer *mNewRoutepointContainer;
};



class AddPhotoCommand : public AddWaypointCommand
{
public:
    AddPhotoCommand(FilesController *fc, QUndoCommand *parent = nullptr)
        : AddWaypointCommand(fc, parent)		{}
    virtual ~AddPhotoCommand()				{}

    void setLink(const QUrl &link)			{ mLinkUrl = link; }
    void setTime(const QDateTime &dt)			{ mDateTime = dt; }

    void redo() override;
    void undo() override;

private:
    QUrl mLinkUrl;
    QDateTime mDateTime;
};


#endif							// COMMANDS_H
