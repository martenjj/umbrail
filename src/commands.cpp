
#include "commands.h"

#include <kdebug.h>

#include "filesmodel.h"
//#include "pointsview.h"



ImportFileCommand::ImportFileCommand(FilesController *fc, QUndoCommand *parent)
    : FilesCommandBase(fc, parent)
{
    mTrackData = NULL;					// nothing held at present
}


ImportFileCommand::~ImportFileCommand()
{
    if (mTrackData!=NULL) delete mTrackData;		// delete if we hold data
}


void ImportFileCommand::redo()
{
    TrackDataFile *tdf = mTrackData;
    Q_ASSERT(tdf!=NULL);
    kDebug() << "file" << tdf->name();

    model()->addFile(tdf);				// add data tree to model
    mTrackData = NULL;					// model owns it now
}


void ImportFileCommand::undo()
{
    mTrackData = model()->removeFile();			// take ownership of data tree
    kDebug() << "saved" << mTrackData->name();
}
