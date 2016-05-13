
#include "importerexporterbase.h"

#include <qdebug.h>

#include <klocalizedstring.h>

#include "errorreporter.h"


ImporterExporterBase::ImporterExporterBase()
{
    qDebug();
    mReporter = new ErrorReporter;
}


ImporterExporterBase::~ImporterExporterBase()
{
    delete mReporter;
    qDebug() << "done";
}
