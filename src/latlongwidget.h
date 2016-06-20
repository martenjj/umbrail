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

protected slots:
    void slotPasteCoordinates();

signals:
    void positionChanged(double lat, double lon);
    void positionValid(bool valid);

private slots:
    void slotDecimalTextChanged();
    void slotDmsTextChanged();

private:
    void setDMS(double d, QLineEdit *deg, QLineEdit *min,
                QLineEdit *sec, QComboBox *sign);
    double getDMS(QLineEdit *deg, QLineEdit *min,
                  QLineEdit *sec, QComboBox *sign) const;
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

    double mLatitude;
    double mLongitude;
};


#endif							// LATLONGWIDGET_H
