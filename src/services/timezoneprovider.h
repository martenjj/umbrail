
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
    bool isValid() const			{ return (mJobReady); }

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
    bool mJobReady;
};

#endif							// TIMEZONEPROVIDER_H
