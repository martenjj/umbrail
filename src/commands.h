// -*-mode:c++ -*-

#ifndef COMMANDS_H
#define COMMANDS_H
 

#include <QUndoCommand>

#include "trackdata.h"
#include "filescontroller.h"





// abstract
class CommandBase : public QUndoCommand
{
public:
    CommandBase(QUndoCommand *parent = NULL) : QUndoCommand(parent) {};
    virtual ~CommandBase() {};
    virtual void undo() = 0;
    virtual void redo() = 0;
};





// abstract
class FilesCommandBase : public CommandBase
{
public:
    FilesCommandBase(FilesController *fc, QUndoCommand *parent = NULL)
        : CommandBase(parent),
          mController(fc)		{};
    virtual ~FilesCommandBase()		{};

protected:
    FilesController *controller() const		{ return (mController); }
    FilesModel *model() const			{ return (mController->model()); }

private:
    FilesController *mController;
};




class ImportFileCommand : public FilesCommandBase
{
public:
    ImportFileCommand(FilesController *fc, QUndoCommand *parent = NULL);
    virtual ~ImportFileCommand();

    void setTrackData(TrackDataFile *tdf)	{ mTrackData = tdf; }

    void redo();
    void undo();

private:
    TrackDataFile *mTrackData;
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
