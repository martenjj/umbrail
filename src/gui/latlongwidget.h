// -*-mode:c++ -*-

#ifndef LATLONGWIDGET_H
#define LATLONGWIDGET_H


#include <qframe.h>
#include <qvector.h>

class QTabWidget;
class AbstractCoordinateHandler;


class LatLongWidget : public QFrame
{
    Q_OBJECT

public:
    explicit LatLongWidget(QWidget *pnt = nullptr);
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
