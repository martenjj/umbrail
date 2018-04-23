
#include "abstractcoordinatehandler.h"

#include <math.h>

#include <qdebug.h>


AbstractCoordinateHandler::AbstractCoordinateHandler(QObject *pnt)
    : QObject(pnt)
{
}


void AbstractCoordinateHandler::setLatLong(double lat, double lon)
{
    qDebug() << lat << lon;

    blockSignals(true);					// block signals to other handlers

    mLatitude = lat;
    mLongitude = lon;
    updateGUI(lat, lon);

    blockSignals(false);				// so that the below works
    checkError();
}


void AbstractCoordinateHandler::updateValues(double lat, double lon)
{
    qDebug() << lat << lon;

    mLatitude = lat;
    mLongitude = lon;
    emit valueChanged();
}


void AbstractCoordinateHandler::checkError()
{
    setError(QString());
}


void AbstractCoordinateHandler::setError(const QString &msg)
{
    qDebug() << msg;
    emit statusMessage(msg);
}
