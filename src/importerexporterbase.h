// -*-mode:c++ -*-

#ifndef IMPORTEREXPORTERBASE_H
#define IMPORTEREXPORTERBASE_H
 

#include <qstring.h>

class ErrorReporter;


class ImporterExporterBase
{
public:

    ErrorReporter *reporter() const		{ return (mReporter); }

protected:
    ImporterExporterBase();
    virtual ~ImporterExporterBase();

private:
    ErrorReporter *mReporter;
};

 
#endif							// IMPORTEREXPORTERBASE_H
