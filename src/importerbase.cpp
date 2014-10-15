
#include "importerbase.h"

#include <string.h>
#include <errno.h>

#include <qfile.h>

#include <kdebug.h>
#include <klocale.h>
#include <kurl.h>

#include "trackdata.h"
#include "dataindexer.h"


#define DEBUG_IMPORT



ImporterBase::ImporterBase()
    : ImporterExporterBase()
{
    kDebug();
    mDataRoot = NULL;
    mFile = NULL;
}



ImporterBase::~ImporterBase()
{
    delete mFile;
    // Do not delete mDataRoot, it is owned by caller
    kDebug() << "done";
}



bool ImporterBase::prepareLoadFile(const KUrl &file)
{
    kDebug() << "from" << file;

    // verify/open file
    if (!file.isLocalFile())
    {
        setError(i18n("Can only read local files"));
        return (false);
    }

    mFile = new QFile(file.path());
    if (!mFile->open(QIODevice::ReadOnly))
    {
        setError(i18n("Cannot open file, %1", strerror(errno)));
        return (false);
    }

    mDataRoot = new TrackDataFile(file.fileName());
    mDataRoot->setFileName(file);
    return (true);
}



#ifdef DEBUG_IMPORT
static void dumpMetadata(const TrackDataItem *tdd, const QString &source)
{
    kDebug() << source.toLatin1().constData();
    for (int i = 0; i<DataIndexer::self()->count(); ++i)
    {
        QString s =  tdd->metadata(i);
        if (s.isEmpty()) continue;
        kDebug() << "  " << i << DataIndexer::self()->name(i) << "=" << s;
    }
}
#endif



bool ImporterBase::finaliseLoadFile(const KUrl &file)
{
    mFile->close();					// finished with reading file
    if (mDataRoot==NULL) return (false);		// problem reading, no data

    // The metadata as read from the file is currently stored in mDataRoot.
    // This data is merged with the metadata of each child track, as a record
    // of the original source.  The metadata stored in mDataRoot is ignored
    // when the tree is added to the files model, it is updated to reflect
    // the current state when the file is saved.

#ifdef DEBUG_IMPORT
    dumpMetadata(mDataRoot, "metadata from file:");
#endif
    for (int i = 0; i<mDataRoot->childCount(); ++i)
    {
        TrackDataTrack *tdt = dynamic_cast<TrackDataTrack *>(mDataRoot->childAt(i));
        if (tdt==NULL) continue;
#ifdef DEBUG_IMPORT
        dumpMetadata(tdt, QString("original metadata of track %1:").arg(i));
#endif

        if (tdt->metadata("creator").isEmpty())		// only if blank already
        {
            tdt->copyMetadata(mDataRoot, false);
#ifdef DEBUG_IMPORT
            dumpMetadata(tdt, QString("merged metadata of track %1:").arg(i));
#endif
        }
    }

    return (true);
}
