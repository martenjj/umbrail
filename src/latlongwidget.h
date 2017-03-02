// -*-mode:c++ -*-

#ifndef LATLONGWIDGET_H
#define LATLONGWIDGET_H


#include <qframe.h>


class QLineEdit;
class QTabWidget;
class QComboBox;


class LatLongWidget : public QFrame
{
    Q_OBJECT

public:
    explicit LatLongWidget(QWidget *pnt = NULL);
    virtual ~LatLongWidget();

    void setLatLong(double lat, double lon);
    double latitude() const			{ return (mLatitude); }
    double longitude() const			{ return (mLongitude); }

    bool hasAcceptableInput() const;

signals:
    void positionChanged(double lat, double lon);
    void positionValid(bool valid);

private slots:
    void slotDecimalTextChanged();
    void slotDmsTextChanged();
    void slotSwissTextChanged();

private:
    void setDMS(double d, QLineEdit *deg, QLineEdit *min, QLineEdit *sec, QComboBox *sign);
    double getDMS(QLineEdit *deg, QLineEdit *min, QLineEdit *sec, QComboBox *sign) const;

    void setSwiss(double lat, double lon, QLineEdit *east, QLineEdit *north);
    void getSwiss(QLineEdit *east, QLineEdit *north, double *latp, double *lonp) const;

    void textChanged();

private:
    QTabWidget *mTabs;

    QLineEdit *mLatitudeEdit;
    QLineEdit *mLongitudeEdit;

    QLineEdit *mLatitudeDeg;
    QLineEdit *mLatitudeMin;
    QLineEdit *mLatitudeSec;
    QComboBox *mLatitudeCombo;

    QLineEdit *mLongitudeDeg;
    QLineEdit *mLongitudeMin;
    QLineEdit *mLongitudeSec;
    QComboBox *mLongitudeCombo;

    QLineEdit *mSwissNorthEdit;
    QLineEdit *mSwissEastEdit;

    double mLatitude;
    double mLongitude;
};


#endif							// LATLONGWIDGET_H
