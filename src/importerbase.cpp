
#include "importerbase.h"

#include <string.h>
#include <errno.h>

//#include <qfile.h>

#include <kdebug.h>
#include <klocale.h>
#include <kurl.h>

#include "trackdata.h"



ImporterBase::ImporterBase()
    : ImporterExporterBase()
{
    kDebug();
    mReadingFile = NULL;
}


ImporterBase::~ImporterBase()
{
    kDebug() << "done";
}



bool ImporterBase::prepareLoadFile(const KUrl &file)
{
    kDebug() << "from" << file;

    mReadingFile = NULL;				// initially reset

//    // verify/open file
//    if (!file.isLocalFile())
//    {
//        setError(i18n("Can only read local files"));
//        return (false);
//    }
//
//    QFile f(file.path());
//    if (!f.open(QIODevice::ReadOnly))
//    {
//        setError(i18n("Cannot open file, %1", strerror(errno)));
//        return (false);
//    }

    mReadingFile = new TrackDataFile(file.fileName());
    mReadingFile->setFileName(file);
    return (true);
}
