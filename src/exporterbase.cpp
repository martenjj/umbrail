
#include "exporterbase.h"

#include <string.h>
#include <errno.h>

#include <kdebug.h>
#include <klocalizedstring.h>
#include <kurl.h>

#include "trackdata.h"
#include "errorreporter.h"



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

    reporter()->setFile(file);

    // verify/open file
    if (!file.isLocalFile())
    {
        reporter()->setError(ErrorReporter::Fatal, i18n("Can only save to local files"));
        return (false);
    }

    mSaveFile.setFileName(file.path());
    if (!mSaveFile.open(QIODevice::WriteOnly))
    {
        reporter()->setError(ErrorReporter::Fatal, i18n("Cannot open file, %1", strerror(errno)));
        return (false);
    }

    return (true);
}
