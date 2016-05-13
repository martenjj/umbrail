
#include "importerexporterbase.h"

#include <kdebug.h>
#include <klocalizedstring.h>

#include "errorreporter.h"


ImporterExporterBase::ImporterExporterBase()
{
    kDebug();
    mReporter = new ErrorReporter;
}


ImporterExporterBase::~ImporterExporterBase()
{
    delete mReporter;
    kDebug() << "done";
}
