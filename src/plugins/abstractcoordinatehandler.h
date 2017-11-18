// -*-mode:c++ -*-

#ifndef ABSTRACTCOORDINATEHANDLER_H
#define ABSTRACTCOORDINATEHANDLER_H


#include <qobject.h>


#ifndef PLUGIN_EXPORT
#define PLUGIN_EXPORT
#endif


class PLUGIN_EXPORT AbstractCoordinateHandler : public QObject
{
    Q_OBJECT

public:
    virtual QWidget *createWidget(QWidget *pnt = nullptr) = 0;

    virtual double getLatitude() const			{ return (mLatitude); }
    virtual double getLongitude() const			{ return (mLongitude); }

    virtual void setLatLong(double lat, double lon);
    virtual bool hasAcceptableInput() const = 0;
    virtual QString tabName() const = 0;

signals:
    void valueChanged();

protected:
    AbstractCoordinateHandler(QObject *pnt = nullptr);
    virtual ~AbstractCoordinateHandler() = default;

    virtual void updateGUI(double lat, double lon) = 0;
    void updateValues(double lat, double lon);

private:
    double mLatitude;
    double mLongitude;
};

#endif							// ABSTRACTCOORDINATEHANDLER_H
