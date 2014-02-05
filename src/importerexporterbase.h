// -*-mode:c++ -*-

#ifndef IMPORTEREXPORTERBASE_H
#define IMPORTEREXPORTERBASE_H
 

#include <qstring.h>


class ImporterExporterBase
{
protected:
    ImporterExporterBase();
    virtual ~ImporterExporterBase();

    void setError(const QString &err);

public:
    const QString &lastError();

protected:
    QString mErrorString;
};

 
#endif							// IMPORTEREXPORTERBASE_H
