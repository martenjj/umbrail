
#include "importerexporterbase.h"

#include <kdebug.h>
#include <klocale.h>



ImporterExporterBase::ImporterExporterBase()
{
    kDebug();

    mErrorString = QString::null;
}


ImporterExporterBase::~ImporterExporterBase()
{
    kDebug() << "done";
}


void ImporterExporterBase::setError(const QString &err)
{
    kDebug() << err;
    mErrorString = err;
}


const QString &ImporterExporterBase::lastError()
{
    return (mErrorString);
}
