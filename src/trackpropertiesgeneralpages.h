
#ifndef TRACKPROPERTIESGENERALPAGES_H
#define TRACKPROPERTIESGENERALPAGES_H

#include "trackpropertiespage.h"

class QLabel;
class QDateTime;
class QFormLayout;
class KLineEdit;
class KTextEdit;
class ItemTypeCombo;
class TrackDataItem;
class TrackDataAbstractPoint;
class TimeZoneSelector;





class TrackItemGeneralPage : public TrackPropertiesPage
{
    Q_OBJECT

public:
    virtual ~TrackItemGeneralPage()				{}

    QString newItemName() const;
    QString newItemDesc() const;
    QString newTrackType() const;
    QString newTimeZone() const;

    virtual QString typeText(int count) const = 0;
    virtual bool isDataValid() const;

    bool newPointPosition(double *newLat, double *newLon);

protected:
    TrackItemGeneralPage(const QList<TrackDataItem *> *items, QWidget *pnt);

    void addTimeSpanFields(const QList<TrackDataItem *> *items);
    void addTypeField(const QList<TrackDataItem *> *items);
    void addDescField(const QList<TrackDataItem *> *items);
    void addPositionTimeFields(const QList<TrackDataItem *> *items);

protected slots:
    void slotChangePosition();

signals:
    void timeZoneChanged(const QString &zone);
    void pointPositionChanged(double newLat, double newLon);

protected:
    KLineEdit *mNameEdit;
    ItemTypeCombo *mTypeCombo;
    KTextEdit *mDescEdit;
    TimeZoneSelector *mTimeZoneSel;

    const TrackDataAbstractPoint *mPositionPoint;
    bool mPositionChanged;
    double mPositionLatitude;
    double mPositionLongitude;
};




class TrackFileGeneralPage : public TrackItemGeneralPage
{
    Q_OBJECT

public:
    TrackFileGeneralPage(const QList<TrackDataItem *> *items, QWidget *pnt);
    virtual ~TrackFileGeneralPage()				{}

    virtual bool isDataValid() const;

    QString typeText(int count) const;

private:
    KLineEdit *mUrlRequester;
};



class TrackTrackGeneralPage : public TrackItemGeneralPage
{
    Q_OBJECT

public:
    TrackTrackGeneralPage(const QList<TrackDataItem *> *items, QWidget *pnt);
    virtual ~TrackTrackGeneralPage()				{}

    QString typeText(int count) const;
};



class TrackSegmentGeneralPage : public TrackItemGeneralPage
{
    Q_OBJECT

public:
    TrackSegmentGeneralPage(const QList<TrackDataItem *> *items, QWidget *pnt);
    virtual ~TrackSegmentGeneralPage()				{}

    QString typeText(int count) const;
};



class TrackTrackpointGeneralPage : public TrackItemGeneralPage
{
    Q_OBJECT

public:
    TrackTrackpointGeneralPage(const QList<TrackDataItem *> *items, QWidget *pnt);
    virtual ~TrackTrackpointGeneralPage()				{}

    QString typeText(int count) const;


private:
};


class TrackFolderGeneralPage : public TrackItemGeneralPage
{
    Q_OBJECT

public:
    TrackFolderGeneralPage(const QList<TrackDataItem *> *items, QWidget *pnt);
    virtual ~TrackFolderGeneralPage()				{}

    QString typeText(int count) const;

private:
};


class TrackWaypointGeneralPage : public TrackItemGeneralPage
{
    Q_OBJECT

public:
    TrackWaypointGeneralPage(const QList<TrackDataItem *> *items, QWidget *pnt);
    virtual ~TrackWaypointGeneralPage()				{}

    QString typeText(int count) const;
};


#endif							// TRACKPROPERTIESGENERALPAGES_H
