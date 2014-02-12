
#ifndef TRACKDATALABEL_H
#define TRACKDATALABEL_H

#include <qlabel.h>

class QDateTime;
class TrackDataItem;






class TrackDataLabel : public QLabel
{
    Q_OBJECT

public:
    TrackDataLabel(const QString &str, QWidget *pnt = NULL);
    TrackDataLabel(int i, QWidget *pnt = NULL);
    TrackDataLabel(const QDateTime &dt, QWidget *pnt = NULL);
    TrackDataLabel(double lat, double lon, QWidget *pnt = NULL);

    virtual ~TrackDataLabel()					{}

private:
    void init();
};



#endif							// TRACKDATALABEL_H
