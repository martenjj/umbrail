// -*-mode:c++ -*-

#ifndef ABSTRACTCOORDINATEHANDLER_H
#define ABSTRACTCOORDINATEHANDLER_H


#include <qobject.h>
#include "coordinatehandlerinterface.h"


#ifndef PLUGIN_EXPORT
#define PLUGIN_EXPORT
#endif


class PLUGIN_EXPORT AbstractCoordinateHandler : public QObject, public CoordinateHandlerInterface
{
    Q_OBJECT
    Q_INTERFACES(CoordinateHandlerInterface);

public:
    virtual double getLatitude() const			{ return (mLatitude); }
    virtual double getLongitude() const			{ return (mLongitude); }

    virtual void setLatLong(double lat, double lon);

signals:
    void valueChanged();

protected:
    AbstractCoordinateHandler(QObject *pnt = nullptr);
    virtual ~AbstractCoordinateHandler() = default;

    void updateValues(double lat, double lon);

private:
    double mLatitude;
    double mLongitude;
};

#endif							// ABSTRACTCOORDINATEHANDLER_H
