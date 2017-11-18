
#include "abstractcoordinatehandler.h"

#include <qdebug.h>


AbstractCoordinateHandler::AbstractCoordinateHandler(QObject *pnt)
    : QObject(pnt)
{
    qDebug();
}


void AbstractCoordinateHandler::setLatLong(double lat, double lon)
{
    QSignalBlocker blocker(this);

    qDebug() << lat << lon;

    mLatitude = lat;
    mLongitude = lon;
    updateGUI(lat, lon);
}


void AbstractCoordinateHandler::updateValues(double lat, double lon)
{
    qDebug() << lat << lon;

    mLatitude = lat;
    mLongitude = lon;
    emit valueChanged();
}
