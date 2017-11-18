// -*-mode:c++ -*-

#ifndef DMSCOORDINATE_H
#define DMSCOORDINATE_H


#include "abstractcoordinatehandler.h"


class QLineEdit;
class QComboBox;


class DMSCoordinateHandler : public AbstractCoordinateHandler
{
    Q_OBJECT

public:
    PLUGIN_EXPORT DMSCoordinateHandler(QObject *pnt = nullptr);
    virtual PLUGIN_EXPORT ~DMSCoordinateHandler() = default;

    PLUGIN_EXPORT QWidget *createWidget(QWidget *pnt = nullptr) override;
    PLUGIN_EXPORT bool hasAcceptableInput() const override;
    PLUGIN_EXPORT QString tabName() const override;

protected:
    void updateGUI(double lat, double lon) override;

private slots:
    void slotTextChanged();

private:
    void setDMS(double d, QLineEdit *deg, QLineEdit *min, QLineEdit *sec, QComboBox *sign);
    double getDMS(QLineEdit *deg, QLineEdit *min, QLineEdit *sec, QComboBox *sign) const;

private:
    QLineEdit *mLatitudeDeg;
    QLineEdit *mLatitudeMin;
    QLineEdit *mLatitudeSec;
    QComboBox *mLatitudeCombo;

    QLineEdit *mLongitudeDeg;
    QLineEdit *mLongitudeMin;
    QLineEdit *mLongitudeSec;
    QComboBox *mLongitudeCombo;
};

#endif							// DMSCOORDINATE_H
