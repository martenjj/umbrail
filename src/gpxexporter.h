
#ifndef GPXEXPORTER_H
#define GPXEXPORTER_H


#include "exporterbase.h"


class TrackDataFile;
class QXmlStreamWriter;


class GpxExporter : public ExporterBase
{
public:
    GpxExporter();
    virtual ~GpxExporter() = default;

    static QString filter();

protected:
    bool saveTo(QIODevice *devconst, const TrackDataFile *item) override;

private:
    bool writeItem(const TrackDataItem *item, QXmlStreamWriter &str) const;
    bool writeChildren(const TrackDataItem *item, QXmlStreamWriter &str) const;

};

#endif							// GPXEXPORTER_H
