
#include "trackdatalabel.h"

#include <qdebug.h>

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

    TrackPropertiesPage *page = qobject_cast<TrackPropertiesPage *>(pnt);
    if (page!=NULL)					// part of a parent page
    {
        connect(page, SIGNAL(updateTimeZones(const QTimeZone *)), SLOT(slotTimeZoneChanged(const QTimeZone *)));
        slotTimeZoneChanged(page->timeZone());
    }
    else slotTimeZoneChanged(nullptr);
    init();
}


void TrackDataLabel::slotTimeZoneChanged(const QTimeZone *tz)
{
    if (!mDateTime.isValid()) return;			// not a date/time label
    setText(TrackData::formattedTime(mDateTime, tz));	// date/time with zone
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
    setTextInteractionFlags(Qt::TextSelectableByMouse|Qt::TextSelectableByKeyboard);
}
