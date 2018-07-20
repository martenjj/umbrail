// -*-mode:c++ -*-

#ifndef EXPORTERBASE_H
#define EXPORTERBASE_H
 

#include "importerexporterbase.h"

class QUrl;
class QIODevice;


class TrackDataItem;
class TrackDataFile;


class ExporterBase : public ImporterExporterBase
{
public:
    ExporterBase();
    virtual ~ExporterBase() = default;

    bool save(const QUrl &file, const TrackDataFile *item);
    void setSelectionId(unsigned long id);

protected:
    virtual bool saveTo(QIODevice *dev, const TrackDataFile *item) = 0;
    bool isSelected(const TrackDataItem *item) const;

protected:
    unsigned long mSelectionId;
};

 
#endif							// EXPORTERBASE_H
