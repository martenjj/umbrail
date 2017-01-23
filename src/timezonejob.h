
#ifndef TIMEZONEJOB_H
#define TIMEZONEJOB_H

#include <kcompositejob.h>
#include <kio/job.h>


class TimeZoneJob : public KCompositeJob
{
    Q_OBJECT

public:
    TimeZoneJob(double lat, double lon, QObject *pnt = nullptr);
    virtual ~TimeZoneJob() = default;

    void start()				{}

    QString timeZone() const			{ return (mTimeZone); }
    QUrl url() const				{ return (mUrl); }

protected slots:
    void slotDataReceived(KIO::Job *job, const QByteArray &data);
    void slotDataResult(KJob *job);

private:
    QByteArray mReceivedData;
    QString mTimeZone;
    QUrl mUrl;
};

#endif							// TIMEZONEJOB_H
