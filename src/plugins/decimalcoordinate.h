// -*-mode:c++ -*-

#ifndef DECIMALCOORDINATE_H
#define DECIMALCOORDINATE_H


#include "abstractcoordinatehandler.h"


class QLineEdit;


class DecimalCoordinateHandler : public AbstractCoordinateHandler
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kde.navtracks.CoordinateHandlerInterface")
    Q_INTERFACES(CoordinateHandlerInterface)

public:
    PLUGIN_EXPORT DecimalCoordinateHandler(QObject *pnt = nullptr);
    virtual PLUGIN_EXPORT ~DecimalCoordinateHandler() = default;

    PLUGIN_EXPORT QWidget *createWidget(QWidget *pnt = nullptr) override;
    PLUGIN_EXPORT bool hasAcceptableInput() const override;
    PLUGIN_EXPORT QString tabName() const override;

protected:
    void updateGUI(double lat, double lon) override;

private slots:
    void slotTextChanged(const QString &text);

private:
    QLineEdit *mLatitudeEdit;
    QLineEdit *mLongitudeEdit;
};

#endif							// DECIMALCOORDINATE_H
