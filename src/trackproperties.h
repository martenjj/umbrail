
#ifndef TRACKPROPERTIES_H
#define TRACKPROPERTIES_H

//#include "trackdata.h"
#include <qwidget.h>

class QLabel;
class QFormLayout;
class KLineEdit;
class KUrlRequester;
class TrackDataItem;



class TrackItemGeneralPage : public QWidget
{
    Q_OBJECT

public:
    virtual ~TrackItemGeneralPage()				{}

protected:
    TrackItemGeneralPage(const QList<TrackDataItem *> items, QWidget *pnt);

    virtual bool isDataValid() const;

    void addTimeDistanceSpeedFields(const QList<TrackDataItem *> &items, bool bothTimes = true);
    void addBoundingAreaField(const QList<TrackDataItem *> &items);
    void addChildCountField(const QList<TrackDataItem *> &items, const QString &labelText);

protected:
    QLabel *mTypeLabel;
    QLabel *mIconLabel;
    QFormLayout *mFormLayout;
    KLineEdit *mNameEdit;

protected slots:
    void slotDataChanged();

signals:
    void enableButtonOk(bool);



};








class TrackFileGeneralPage : public TrackItemGeneralPage
{
    Q_OBJECT

public:
    TrackFileGeneralPage(const QList<TrackDataItem *> items, QWidget *pnt);
    virtual ~TrackFileGeneralPage()				{}

protected:
    virtual bool isDataValid() const;

private:
    KUrlRequester *mUrlRequester;

};



class TrackTrackGeneralPage : public TrackItemGeneralPage
{
    Q_OBJECT

public:
    TrackTrackGeneralPage(const QList<TrackDataItem *> items, QWidget *pnt);
    virtual ~TrackTrackGeneralPage()				{}


};



class TrackSegmentGeneralPage : public TrackItemGeneralPage
{
    Q_OBJECT

public:
    TrackSegmentGeneralPage(const QList<TrackDataItem *> items, QWidget *pnt);
    virtual ~TrackSegmentGeneralPage()				{}


};



class TrackPointGeneralPage : public TrackItemGeneralPage
{
    Q_OBJECT

public:
    TrackPointGeneralPage(const QList<TrackDataItem *> items, QWidget *pnt);
    virtual ~TrackPointGeneralPage()				{}


};
















#endif							// TRACKPROPERTIES_H
