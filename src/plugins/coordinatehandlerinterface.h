// -*-mode:c++ -*-

#ifndef COORDINATEHANDLERINTERFACE_H
#define COORDINATEHANDLERINTERFACE_H

// For plugin implementation details and example, see
// the QPluginLoader API documentation and
// http://doc.qt.io/qt-5/qtwidgets-tools-echoplugin-example.html

class CoordinateHandlerInterface
{

public:
    virtual ~CoordinateHandlerInterface() = default;

    virtual QWidget *createWidget(QWidget *pnt = nullptr) = 0;
    virtual bool hasAcceptableInput() const = 0;
    virtual QString tabName() const = 0;

protected:
    virtual void updateGUI(double lat, double lon) = 0;
};

Q_DECLARE_INTERFACE(CoordinateHandlerInterface, "org.kde.navtracks.CoordinateHandlerInterface")

#endif							// COORDINATEHANDLERINTERFACE_H
