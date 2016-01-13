// -*-mode:c++ -*-

#ifndef IMPORTERBASE_H
#define IMPORTERBASE_H
 

#include "importerexporterbase.h"


class QFile;
class KUrl;
class TrackDataFile;


class ImporterBase : public ImporterExporterBase
{
public:
    ImporterBase();
    virtual ~ImporterBase();

    virtual TrackDataFile *load(const KUrl &file) = 0;
    virtual bool needsResave() const			{ return (false); }

protected:
    bool prepareLoadFile(const KUrl &file);
    bool finaliseLoadFile(const KUrl &file);

protected:
    QFile *mFile;
    TrackDataFile *mDataRoot;
};

 
#endif							// IMPORTERBASE_H
