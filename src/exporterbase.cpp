
#include "exporterbase.h"

#include <string.h>
#include <errno.h>

#include <kdebug.h>
#include <klocale.h>
#include <kurl.h>

#include "trackdata.h"



ExporterBase::ExporterBase()
    : ImporterExporterBase()
{
    kDebug();
}


ExporterBase::~ExporterBase()
{
    kDebug() << "done";
}


bool ExporterBase::prepareSaveFile(const KUrl &file)
{
    kDebug() << "to" << file;

    // verify/open file
    if (!file.isLocalFile())
    {
        setError(i18n("Can only save to local files"));
        return (false);
    }

    mSaveFile.setFileName(file.path());
    if (!mSaveFile.open(QIODevice::WriteOnly))
    {
        setError(i18n("Cannot open file, %1", strerror(errno)));
        return (false);
    }

    return (true);
}
