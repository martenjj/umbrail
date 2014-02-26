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

    void setTrackData(TrackDataFile *tdf)		{ mTrackData = tdf; }

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

    void setDataItem(TrackDataDisplayable *item)	{ mDataItem = item; }

protected:
    ChangeItemCommand(FilesController *fc, QUndoCommand *parent = NULL);

protected:
    TrackDataDisplayable *mDataItem;
    bool mFileWasModified;
};







class ChangeItemNameCommand : public ChangeItemCommand
{
public:
    ChangeItemNameCommand(FilesController *fc, QUndoCommand *parent = NULL)
        : ChangeItemCommand(fc, parent)		{}
    virtual ~ChangeItemNameCommand()		{}

    void setNewName(const QString &name)	{ mNewName = name; }

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
        : ChangeItemCommand(fc, parent)		{}
    virtual ~ChangeItemStyleCommand()		{}

    void setNewStyle(const Style &style)	{ mNewStyle = style; }

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
        : ChangeItemCommand(fc, parent)		{}
    virtual ~ChangeItemDataCommand()		{}

    void setNewData(const QString &key,
                    const QString &value)	{ mKey = key; mNewValue = value; }

    void redo();
    void undo();

private:
    QString mKey;
    QString mNewValue;
    QString mSavedValue;
};







//class DeletePointsCommand : public PointsCommandBase
//{
//public:
//    DeletePointsCommand(PointsController *pc, QUndoCommand *parent = NULL)
//        : PointsCommandBase(pc, parent)
//    {};
//
//    void setRowList(QList<int> rows)		{ mRowList = rows; }
//
//    void redo();
//    void undo();
//
//private:
//    QList<int> mRowList;
//    QList<PointData> mSavedPoints;
//};













#endif							// COMMANDS_H
