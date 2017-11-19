// -*-mode:c++ -*-

#ifndef SWISSCOORDINATE_H
#define SWISSCOORDINATE_H


#include "abstractcoordinatehandler.h"


class QLineEdit;


class SwissCoordinateHandler : public AbstractCoordinateHandler
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kde.navtracks.CoordinateHandlerInterface")
    Q_INTERFACES(CoordinateHandlerInterface)

public:
    PLUGIN_EXPORT SwissCoordinateHandler(QObject *pnt = nullptr);
    virtual PLUGIN_EXPORT ~SwissCoordinateHandler() = default;

    PLUGIN_EXPORT QWidget *createWidget(QWidget *pnt = nullptr) override;
    PLUGIN_EXPORT bool hasAcceptableInput() const override;
    PLUGIN_EXPORT QString tabName() const override;

protected:
    void updateGUI(double lat, double lon) override;

private slots:
    void slotTextChanged();

private:
    void setSwiss(double lat, double lon, QLineEdit *east, QLineEdit *north);
    void getSwiss(QLineEdit *east, QLineEdit *north, double *latp, double *lonp) const;

private:
    QLineEdit *mSwissNorthEdit;
    QLineEdit *mSwissEastEdit;
};

#endif							// SWISSCOORDINATE_H
