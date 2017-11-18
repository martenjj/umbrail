// -*-mode:c++ -*-

#ifndef LATLONGWIDGET_H
#define LATLONGWIDGET_H


#include <qframe.h>
#include <qvector.h>


class QLineEdit;
class QTabWidget;
class QComboBox;

// -----------------------------------------------------------------------

class AbstractCoordinateHandler : public QObject
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

// -----------------------------------------------------------------------

class DecimalCoordinateHandler : public AbstractCoordinateHandler
{
    Q_OBJECT

public:
    DecimalCoordinateHandler(QObject *pnt = nullptr);
    virtual ~DecimalCoordinateHandler() = default;

    QWidget *createWidget(QWidget *pnt = nullptr) override;
    bool hasAcceptableInput() const override;
    QString tabName() const override;

protected:
    void updateGUI(double lat, double lon) override;

private slots:
    void slotTextChanged(const QString &text);

private:
    QLineEdit *mLatitudeEdit;
    QLineEdit *mLongitudeEdit;
};

// -----------------------------------------------------------------------

class DMSCoordinateHandler : public AbstractCoordinateHandler
{
    Q_OBJECT

public:
    DMSCoordinateHandler(QObject *pnt = nullptr);
    virtual ~DMSCoordinateHandler() = default;

    QWidget *createWidget(QWidget *pnt = nullptr) override;
    bool hasAcceptableInput() const override;
    QString tabName() const override;

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

// -----------------------------------------------------------------------

class SwissCoordinateHandler : public AbstractCoordinateHandler
{
    Q_OBJECT

public:
    SwissCoordinateHandler(QObject *pnt = nullptr);
    virtual ~SwissCoordinateHandler() = default;

    QWidget *createWidget(QWidget *pnt = nullptr) override;
    bool hasAcceptableInput() const override;
    QString tabName() const override;

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

// -----------------------------------------------------------------------

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
    void slotValueChanged();

private:
    void textChanged();

private:
    QTabWidget *mTabs;

    double mLatitude;
    double mLongitude;

    QVector<AbstractCoordinateHandler *> mHandlers;
};


#endif							// LATLONGWIDGET_H
