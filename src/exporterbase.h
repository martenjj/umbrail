// -*-mode:c++ -*-

#ifndef EXPORTERBASE_H
#define EXPORTERBASE_H
 

#include "importerexporterbase.h"

#include <ksavefile.h>


class TrackDataFile;


class ExporterBase : public ImporterExporterBase
{
public:
    ExporterBase();
    virtual ~ExporterBase();

    virtual bool save(const QUrl &file, const TrackDataFile *item) = 0;

protected:
    bool prepareSaveFile(const QUrl &file);

protected:
    KSaveFile mSaveFile;
};

 
#endif							// EXPORTERBASE_H
