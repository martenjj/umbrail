
#ifndef TRACKDATALABEL_H
#define TRACKDATALABEL_H

#include <qlabel.h>

class QDateTime;
class TrackDataItem;






class TrackDataLabel : public QLabel
{
    Q_OBJECT

public:
    TrackDataLabel(const QString &str, QWidget *parent = NULL);
    TrackDataLabel(int i, QWidget *parent = NULL);
    TrackDataLabel(const QDateTime &dt, QWidget *parent = NULL);
    TrackDataLabel(double lat, double lon, QWidget *parent = NULL);

    virtual ~TrackDataLabel()					{}

private:
    void init();
};



#endif							// TRACKDATALABEL_H
