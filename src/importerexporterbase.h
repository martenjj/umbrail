// -*-mode:c++ -*-

#ifndef IMPORTEREXPORTERBASE_H
#define IMPORTEREXPORTERBASE_H
 

#include <qflags.h>

class ErrorReporter;


class ImporterExporterBase
{
public:
    ErrorReporter *reporter() const		{ return (mReporter); }

    enum Option
    {
        NoOption = 0x00,
        ToClipboard = 0x01,
        SelectionOnly = 0x02
    };
    Q_DECLARE_FLAGS(Options, Option)

protected:
    ImporterExporterBase();
    virtual ~ImporterExporterBase();

private:
    ErrorReporter *mReporter;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(ImporterExporterBase::Options)
 
#endif							// IMPORTEREXPORTERBASE_H
