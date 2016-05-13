
#include "exporterbase.h"

#include <string.h>
#include <errno.h>

#include <qdebug.h>

#include <klocalizedstring.h>

#include "trackdata.h"
#include "errorreporter.h"



ExporterBase::ExporterBase()
    : ImporterExporterBase()
{
    qDebug();
}


ExporterBase::~ExporterBase()
{
    qDebug() << "done";
}


bool ExporterBase::prepareSaveFile(const QUrl &file)
{
    qDebug() << "to" << file;

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
