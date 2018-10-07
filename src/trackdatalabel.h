
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
    virtual ~TrackDataLabel() = default;

    // TODO: maybe superfluous
    TrackDataLabel(double lat, double lon, QWidget *pnt = NULL);
    TrackDataLabel(double lat, double lon, bool blankIfUnknown, QWidget *pnt = NULL);

    void setDateTime(const QDateTime &dt);
    void setTimeZone(const QTimeZone *tz);

private:
    void init();
    void updateDateTime();

private:
    QDateTime mDateTime;
    const QTimeZone *mTimeZone;
};

#endif							// TRACKDATALABEL_H
