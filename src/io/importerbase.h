// -*-mode:c++ -*-

#ifndef IMPORTERBASE_H
#define IMPORTERBASE_H
 

#include "importerexporterbase.h"


class QFile;
class QUrl;
class TrackDataFile;


class ImporterBase : public ImporterExporterBase
{
public:
    ImporterBase();
    virtual ~ImporterBase();

    virtual TrackDataFile *load(const QUrl &file) = 0;
    virtual bool needsResave() const			{ return (false); }

protected:
    bool prepareLoadFile(const QUrl &file);
    bool finaliseLoadFile(const QUrl &file);

protected:
    QFile *mFile;
    TrackDataFile *mDataRoot;
};

 
#endif							// IMPORTERBASE_H
