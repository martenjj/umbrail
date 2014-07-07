// -*-mode:c++ -*-

#ifndef LATLONGDIALOGUE_H
#define LATLONGDIALOGUE_H


#include <kdialog.h>


class QLineEdit;
class QTabWidget;
class QComboBox;


class LatLongDialogue : public KDialog
{
    Q_OBJECT

public:
    explicit LatLongDialogue(QWidget *pnt = NULL);
    virtual ~LatLongDialogue();

    void setLatLong(double lat, double lon);
    double latitude() const			{ return (mLatitude); }
    double longitude() const			{ return (mLongitude); }

signals:
    void positionChanged(double lat, double lon);

private slots:
    void slotDecimalTextChanged();
    void slotDmsTextChanged();

private:
    bool updateButtonState();
    void setDMS(double d, QLineEdit *deg, QLineEdit *min,
                QLineEdit *sec, QComboBox *sign);
    double getDMS(QLineEdit *deg, QLineEdit *min,
                  QLineEdit *sec, QComboBox *sign) const;

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


#endif							// LATLONGDIALOGUE_H
