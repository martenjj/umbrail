// -*-mode:c++ -*-

#ifndef ABSTRACTCOORDINATEHANDLER_H
#define ABSTRACTCOORDINATEHANDLER_H


#include <qobject.h>

// For plugin implementation details and example, see
// the QPluginLoader API documentation and
// http://doc.qt.io/qt-5/qtwidgets-tools-echoplugin-example.html

#ifndef PLUGIN_EXPORT
#define PLUGIN_EXPORT
#endif

class AbstractCoordinateHandler;
Q_DECLARE_INTERFACE(AbstractCoordinateHandler, "org.kde.navtracks.AbstractCoordinateHandler")

class PLUGIN_EXPORT AbstractCoordinateHandler : public QObject
{
    Q_OBJECT
    Q_INTERFACES(AbstractCoordinateHandler)

public:
    virtual ~AbstractCoordinateHandler() = default;

    virtual QWidget *createWidget(QWidget *pnt = nullptr) = 0;
    virtual bool hasAcceptableInput() const = 0;
    virtual QString tabName() const = 0;

    void setLatLong(double lat, double lon);
    double getLatitude() const				{ return (mLatitude); }
    double getLongitude() const				{ return (mLongitude); }

signals:
    void valueChanged();
    void statusMessage(const QString &msg);

protected:
    AbstractCoordinateHandler(QObject *pnt = nullptr);

    virtual void updateGUI(double lat, double lon) = 0;
    void updateValues(double lat, double lon);
    virtual void checkError();
    void setError(const QString &msg);

private:
    double mLatitude;
    double mLongitude;
};

#endif							// ABSTRACTCOORDINATEHANDLER_H
