
#include "trackdatalabel.h"

#include <kdebug.h>

#include "trackdata.h"



TrackDataLabel::TrackDataLabel(const QString &str, QWidget *parent)
    : QLabel(str, parent)
{
    init();
}



TrackDataLabel::TrackDataLabel(const QDateTime &dt, QWidget *parent)
    : QLabel(TrackData::formattedTime(dt), parent)
{
    init();
}



TrackDataLabel::TrackDataLabel(double lat, double lon, QWidget *parent)
    : QLabel(TrackData::formattedLatLong(lat, lon), parent)
{
    init();
}



TrackDataLabel::TrackDataLabel(int i, QWidget *parent)
    : QLabel(QString::number(i), parent)
{
    init();
}



void TrackDataLabel::init()
{
    setTextInteractionFlags(Qt::TextSelectableByMouse|Qt::TextSelectableByKeyboard);
}
