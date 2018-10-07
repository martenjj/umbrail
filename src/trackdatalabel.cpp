
#include "trackdatalabel.h"

#include <qdebug.h>
#include <qtimezone.h>

#include <klocalizedstring.h>

#include "trackdata.h"
#include "trackpropertiespage.h"


TrackDataLabel::TrackDataLabel(const QString &str, QWidget *pnt)
    : QLabel(str, pnt)
{
    init();
}


TrackDataLabel::TrackDataLabel(const QDateTime &dt, QWidget *pnt)
    : QLabel(pnt)
{
    mDateTime = dt;
    init();
}


void TrackDataLabel::setDateTime(const QDateTime &dt)
{
    mDateTime = dt;
    updateDateTime();
}


void TrackDataLabel::setTimeZone(const QTimeZone *tz)
{
    mTimeZone = tz;
    updateDateTime();
}


void TrackDataLabel::updateDateTime()
{
    if (mDateTime.isNull()) setText(QString());
    else if (!mDateTime.isValid()) setText(i18nc("an invalid time", "(invalid)"));
    else if (mTimeZone!=nullptr && !mTimeZone->isValid()) setText(i18nc("an invalid time zone", "(invalid time zone)"));
    else setText(TrackData::formattedTime(mDateTime, mTimeZone));
}


TrackDataLabel::TrackDataLabel(double lat, double lon, QWidget *pnt)
    : QLabel(TrackData::formattedLatLong(lat, lon), pnt)
{
    init();
}


TrackDataLabel::TrackDataLabel(double lat, double lon, bool blankIfUnknown, QWidget *pnt)
    : QLabel(TrackData::formattedLatLong(lat, lon, blankIfUnknown), pnt)
{
    init();
}


TrackDataLabel::TrackDataLabel(int i, QWidget *pnt)
    : QLabel(QString::number(i), pnt)
{
    init();
}


void TrackDataLabel::init()
{
    mTimeZone = nullptr;
    setTextInteractionFlags(Qt::TextSelectableByMouse|Qt::TextSelectableByKeyboard);
}
