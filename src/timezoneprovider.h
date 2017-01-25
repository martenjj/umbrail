
#ifndef TIMEZONEPROVIDER_H
#define TIMEZONEPROVIDER_H

#include <qobject.h>
#include <kio/job.h>


class TimeZoneProvider : public QObject
{
    Q_OBJECT

public:
    TimeZoneProvider(double lat, double lon, QObject *pnt = nullptr);
    virtual ~TimeZoneProvider() = default;

    QString timeZone() const			{ return (mTimeZone); }

signals:
    void result(const QString &timeZone);

protected slots:
    void slotDataReceived(KIO::Job *job, const QByteArray &data);
    void slotDataResult(KJob *job);

private:
    void reportError(const QString &msg);

private:
    QByteArray mReceivedData;
    QString mTimeZone;
    QUrl mUrl;
};

#endif							// TIMEZONEPROVIDER_H
