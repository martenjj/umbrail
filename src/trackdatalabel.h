
#ifndef TRACKDATALABEL_H
#define TRACKDATALABEL_H

#include <qlabel.h>
#include <qdatetime.h>

class QTimeZone;
class TrackDataItem;


class TrackDataLabel : public QLabel
{
    Q_OBJECT

public:
    explicit TrackDataLabel(const QString &str, QWidget *pnt = NULL);
    explicit TrackDataLabel(int i, QWidget *pnt = NULL);
    explicit TrackDataLabel(const QDateTime &dt, QWidget *pnt = NULL);
    TrackDataLabel(double lat, double lon, QWidget *pnt = NULL);
    TrackDataLabel(double lat, double lon, bool blankIfUnknown, QWidget *pnt = NULL);

    virtual ~TrackDataLabel() = default;

private slots:
    void slotTimeZoneChanged(const QTimeZone *tz);

private:
    void init();

private:
    QDateTime mDateTime;
};

#endif							// TRACKDATALABEL_H
