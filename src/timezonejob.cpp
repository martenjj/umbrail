
#include "timezonejob.h"

#include <qdebug.h>
#include <qurl.h>
#include <qurlquery.h>
#include <qregexp.h>

#include <kio/transferjob.h>


TimeZoneJob::TimeZoneJob(double lat, double lon, QObject *pnt)
    : KCompositeJob(pnt)
{
    // http://api.geonames.org/timezone?lat=51.46&lng=-0.30&username=jmarten
    mUrl = QUrl("http://api.geonames.org/timezone");

    QUrlQuery query;
    query.addQueryItem("lat", QString::number(lat, 'f', 2));
    query.addQueryItem("lng", QString::number(lon, 'f', 2));
    query.addQueryItem("username", "jmarten");
    mUrl.setQuery(query);

    qDebug() << "getting" << mUrl;
    KJob *job = KIO::get(mUrl);
    connect(job, SIGNAL(data(KIO::Job *, const QByteArray &)), SLOT(slotDataReceived(KIO::Job *, const QByteArray &)));
    connect(job, SIGNAL(result(KJob *)), SLOT(slotDataResult(KJob *)));
    addSubjob(job);
}


void TimeZoneJob::slotDataReceived(KIO::Job *job, const QByteArray &data)
{
    qDebug() << "got" << data;
    if (!data.isEmpty()) mReceivedData += data;
}


void TimeZoneJob::slotDataResult(KJob *job)
{
    qDebug() << "error?" << job->error();
    if (job->error()) return;

    QRegExp rx1("<timezoneId>(\\S+)</timezoneId>");
    QRegExp rx2("<status message=\"([^\"]+)\"");

    QList<QByteArray> lines = mReceivedData.split('\n');
    foreach (const QByteArray &l, lines)
    {
        QString line = QString::fromUtf8(l);
        if (line.contains(rx1))				// look for time zone ID
        {
            mTimeZone = rx1.cap(1);
            qDebug() << "from" << line << "got" << mTimeZone;
            break;
        }

        if (line.contains(rx2))				// look for error message
        {
            setError(KJob::UserDefinedError);
            setErrorText(rx2.cap(1));
            break;
        }
    }

    emitResult();
}
