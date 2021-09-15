
#include "importerbase.h"

#include <string.h>
#include <errno.h>

#include <qfile.h>
#include <qdebug.h>

#include <klocalizedstring.h>
//#include <kurl.h>

#include "trackdata.h"
#include "dataindexer.h"
#include "errorreporter.h"


#define DEBUG_IMPORT



ImporterBase::ImporterBase()
    : ImporterExporterBase()
{
    qDebug();
    mDataRoot = nullptr;
    mFile = nullptr;
}



ImporterBase::~ImporterBase()
{
    delete mFile;
    // Do not delete mDataRoot, it is owned by caller
    qDebug() << "done";
}



bool ImporterBase::prepareLoadFile(const QUrl &file)
{
    qDebug() << "from" << file;

    reporter()->setFile(file);

    // verify/open file
    if (!file.isLocalFile())
    {
        reporter()->setError(ErrorReporter::Fatal, i18n("Can only read local files"));
        return (false);
    }

    mFile = new QFile(file.path());
    if (!mFile->open(QIODevice::ReadOnly))
    {
        reporter()->setError(ErrorReporter::Fatal, i18n("Cannot open file, %1", strerror(errno)));
        return (false);
    }

    mDataRoot = new TrackDataFile;			// no need to set name here,
    mDataRoot->setFileName(file);			// this does from file's basename
    return (true);
}


#ifdef DEBUG_IMPORT
static void dumpMetadata(const TrackDataItem *tdd, const QString &source)
{
    qDebug() << source.toLatin1().constData();
    for (int i = 0; i<DataIndexer::count(); ++i)
    {
        QVariant s =  tdd->metadata(i);
        if (s.isNull()) continue;
        qDebug() << "  " << i << DataIndexer::name(i) << "=" << s.toString();
    }
}
#endif


bool ImporterBase::finaliseLoadFile(const QUrl &file)
{
    mFile->close();					// finished with reading file
    if (mDataRoot==nullptr) return (false);		// problem reading, no data

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
        if (tdt==nullptr) continue;
#ifdef DEBUG_IMPORT
        dumpMetadata(tdt, QString("original metadata of track %1:").arg(i));
#endif

        if (tdt->metadata("creator").isNull())		// only if blank already
        {
            tdt->copyMetadata(mDataRoot, false);
#ifdef DEBUG_IMPORT
            dumpMetadata(tdt, QString("merged metadata of track %1:").arg(i));
#endif
        }
    }

    return (true);
}
