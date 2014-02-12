
#include "trackdatalabel.h"

#include <kdebug.h>

#include "trackdata.h"



TrackDataLabel::TrackDataLabel(const QString &str, QWidget *pnt)
    : QLabel(str, pnt)
{
    init();
}



TrackDataLabel::TrackDataLabel(const QDateTime &dt, QWidget *pnt)
    : QLabel(TrackData::formattedTime(dt), pnt)
{
    init();
}



TrackDataLabel::TrackDataLabel(double lat, double lon, QWidget *pnt)
    : QLabel(TrackData::formattedLatLong(lat, lon), pnt)
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
