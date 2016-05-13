
#ifndef GPXEXPORTER_H
#define GPXEXPORTER_H


#include "exporterbase.h"


class TrackDataFile;


class GpxExporter : public ExporterBase
{
public:
    GpxExporter();
    virtual ~GpxExporter();

    bool save(const QUrl &file, const TrackDataFile *item);

    static QString filter();

private:

};

#endif							// GPXEXPORTER_H
