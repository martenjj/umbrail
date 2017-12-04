// -*-mode:c++ -*-

#ifndef OSGBCOORDINATE_H
#define OSGBCOORDINATE_H


#include "abstractcoordinatehandler.h"


class QComboBox;
class QLineEdit;

struct OSGBRef;


class OSGBCoordinateHandler : public AbstractCoordinateHandler
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kde.navtracks.AbstractCoordinateHandler")
    Q_INTERFACES(AbstractCoordinateHandler)

public:
    PLUGIN_EXPORT OSGBCoordinateHandler(QObject *pnt = nullptr);
    virtual PLUGIN_EXPORT ~OSGBCoordinateHandler() = default;

    PLUGIN_EXPORT QWidget *createWidget(QWidget *pnt = nullptr) override;
    PLUGIN_EXPORT bool hasAcceptableInput() const override;
    PLUGIN_EXPORT QString tabName() const override;

protected:
    void updateGUI(double lat, double lon) override;

private slots:
    void slotReferenceChanged();
    void slotCoordinateChanged();

private:
    void setOSGB(double lat, double lon) const;
    void setOSGBToEN(const OSGBRef &ref) const;
    void setOSGBToRef(const OSGBRef &ref) const;

private:
    QComboBox *mLetterCombo;
    QLineEdit *mReferenceEdit;
    QLineEdit *mNorthEdit;
    QLineEdit *mEastEdit;
};

#endif							// OSGBCOORDINATE_H
