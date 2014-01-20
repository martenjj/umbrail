// -*-mode:c++ -*-

#ifndef IMPORTERBASE_H
#define IMPORTERBASE_H
 

#include "importerexporterbase.h"


class KUrl;
class TrackDataFile;


class ImporterBase : public ImporterExporterBase
{
public:
    ImporterBase();
    virtual ~ImporterBase();

    virtual TrackDataFile *load(const KUrl &file) = 0;

protected:
    bool prepareLoadFile(const KUrl &file);

protected:
    TrackDataFile *mReadingFile;
};

 
#endif							// IMPORTERBASE_H
