
#include "timezoneprovider.h"

#include <qdebug.h>
#include <qurl.h>
#include <qurlquery.h>
#include <qregexp.h>

#include <klocalizedstring.h>
#include <kmessagebox.h>
#include <kio/transferjob.h>

#include "settings.h"


TimeZoneProvider::TimeZoneProvider(double lat, double lon, QObject *pnt)
    : QObject(pnt)
{
    mJobReady = false;					// not yet, anyway

    // TODO for release: be able to configure geonames.org username
    // or may later be able to use the KI18N regional API, see
    // https://www.volkerkrause.eu/2021/07/24/kf5-country-timezone-location-lookup.html

    // http://api.geonames.org/timezone?lat=51.46&lng=-0.30&username=xxxxxx
    mUrl = QUrl("http://api.geonames.org/timezone");

    const QString user = Settings::geonamesUser();
    if (user.isEmpty())					// no point doing the query
    {
         reportError(QString());			// just tell the user why
         return;
    }

    QUrlQuery query;
    query.addQueryItem("lat", QString::number(lat, 'f', 2));
    query.addQueryItem("lng", QString::number(lon, 'f', 2));
    query.addQueryItem("username", user);
    mUrl.setQuery(query);

    qDebug() << "getting" << mUrl;
    KIO::TransferJob *job = KIO::get(mUrl);
    connect(job, &KIO::TransferJob::data, this, &TimeZoneProvider::slotDataReceived);
    connect(job, &KJob::result, this, &TimeZoneProvider::slotDataResult);
    mJobReady = true;
    job->start();
}


void TimeZoneProvider::slotDataReceived(KIO::Job *job, const QByteArray &data)
{
    qDebug() << "received" << data;
    if (!data.isEmpty()) mReceivedData += data;
}


void TimeZoneProvider::slotDataResult(KJob *job)
{
    qDebug() << "error?" << job->error();
    if (job->error())
    {
        reportError(job->errorText());
        return;
    }

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
            emit result(mTimeZone);
            deleteLater();
            return;
        }

        if (line.contains(rx2))				// look for error message
        {
            reportError(rx2.cap(1));
            return;
        }
    }

    reportError(i18n("No time zone found in result"));
}


void TimeZoneProvider::reportError(const QString &msg)
{
    QString m;
    if (msg.isEmpty())					// called for missing user
    {
        m = xi18nc("@info", "The user name for <link url=\"http://geonames.org\">geonames.org</link> is not configured. Register there if necessary, then set it in <interface>Settings->Services</interface>.");
    }
    else						// some other remote error
    {
        m = xi18nc("@info", "Error fetching <link>%1</link><nl/><message>%2</message>", mUrl.toDisplayString(), msg);
    }

    QWidget *pnt = qobject_cast<QWidget *>(parent());	// NULL if none or not a widget
    KMessageBox::error(pnt, m, i18n("Error getting time zone"), KMessageBox::Notify|KMessageBox::AllowLink);
    deleteLater();
}
