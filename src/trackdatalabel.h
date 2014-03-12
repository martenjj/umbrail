
#ifndef TRACKDATALABEL_H
#define TRACKDATALABEL_H

#include <qlabel.h>
#include <qdatetime.h>

class KTimeZone;
class TrackDataItem;





class TrackDataLabel : public QLabel
{
    Q_OBJECT

public:
    TrackDataLabel(const QString &str, QWidget *pnt = NULL);
    TrackDataLabel(int i, QWidget *pnt = NULL);
    TrackDataLabel(const QDateTime &dt, QWidget *pnt = NULL);
    TrackDataLabel(double lat, double lon, QWidget *pnt = NULL);
    TrackDataLabel(double lat, double lon, bool blankIfUnknown, QWidget *pnt = NULL);

    virtual ~TrackDataLabel()					{}

private slots:
    void slotTimeZoneChanged(const KTimeZone *tz);

private:
    void init();

private:
    QDateTime mDateTime;
};



#endif							// TRACKDATALABEL_H
