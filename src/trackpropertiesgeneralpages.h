
#ifndef TRACKPROPERTIESGENERALPAGES_H
#define TRACKPROPERTIESGENERALPAGES_H

#include "trackpropertiespage.h"
#include "trackdata.h"

class QLabel;
class QDateTime;
class QFormLayout;
class QComboBox;
class QSpinBox;
class QLineEdit;
class KTextEdit;
class ItemTypeCombo;
class TrackDataItem;
class TrackDataAbstractPoint;
class TrackDataWaypoint;
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
    TrackData::WaypointStatus newWaypointStatus() const;
    QString newBearingLine() const;

    virtual QString typeText(int count) const = 0;
    virtual bool isDataValid() const override;

    bool newPointPosition(double *newLat, double *newLon);

protected:
    TrackItemGeneralPage(const QList<TrackDataItem *> *items, QWidget *pnt);

    void addTimeSpanFields(const QList<TrackDataItem *> *items);
    void addTypeField(const QList<TrackDataItem *> *items);
    void addStatusField(const QList<TrackDataItem *> *items);
    void addDescField(const QList<TrackDataItem *> *items);
    void addPositionTimeFields(const QList<TrackDataItem *> *items);

protected slots:
    void slotChangePosition();

signals:
    void timeZoneChanged(const QString &zone);
    void pointPositionChanged(double newLat, double newLon);

protected:
    QLineEdit *mNameEdit;
    ItemTypeCombo *mTypeCombo;
    KTextEdit *mDescEdit;
    TimeZoneSelector *mTimeZoneSel;
    QComboBox *mStatusCombo;
    QSpinBox *mBearingEntry;

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

    bool isDataValid() const override;
    QString typeText(int count) const override;

private:
    QLineEdit *mUrlRequester;
};



class TrackTrackGeneralPage : public TrackItemGeneralPage
{
    Q_OBJECT

public:
    TrackTrackGeneralPage(const QList<TrackDataItem *> *items, QWidget *pnt);
    virtual ~TrackTrackGeneralPage()				{}

    QString typeText(int count) const override;
};



class TrackSegmentGeneralPage : public TrackItemGeneralPage
{
    Q_OBJECT

public:
    TrackSegmentGeneralPage(const QList<TrackDataItem *> *items, QWidget *pnt);
    virtual ~TrackSegmentGeneralPage()				{}

    QString typeText(int count) const override;
};



class TrackTrackpointGeneralPage : public TrackItemGeneralPage
{
    Q_OBJECT

public:
    TrackTrackpointGeneralPage(const QList<TrackDataItem *> *items, QWidget *pnt);
    virtual ~TrackTrackpointGeneralPage()				{}

    QString typeText(int count) const override;


private:
};


class TrackRoutepointGeneralPage : public TrackItemGeneralPage
{
    Q_OBJECT

public:
    TrackRoutepointGeneralPage(const QList<TrackDataItem *> *items, QWidget *pnt);
    virtual ~TrackRoutepointGeneralPage()				{}

    QString typeText(int count) const override;

private:
};


class TrackFolderGeneralPage : public TrackItemGeneralPage
{
    Q_OBJECT

public:
    TrackFolderGeneralPage(const QList<TrackDataItem *> *items, QWidget *pnt);
    virtual ~TrackFolderGeneralPage()				{}

    QString typeText(int count) const override;

private:
};


class TrackWaypointGeneralPage : public TrackItemGeneralPage
{
    Q_OBJECT

public:
    TrackWaypointGeneralPage(const QList<TrackDataItem *> *items, QWidget *pnt);
    virtual ~TrackWaypointGeneralPage()				{}

    QString typeText(int count) const override;

protected slots:
    void slotPlayAudioNote();
    void slotPlayVideoNote();
    void slotViewPhotoNote();

private:
    const TrackDataWaypoint *mWaypoint;
};


class TrackRouteGeneralPage : public TrackItemGeneralPage
{
    Q_OBJECT

public:
    TrackRouteGeneralPage(const QList<TrackDataItem *> *items, QWidget *pnt);
    virtual ~TrackRouteGeneralPage()				{}

    QString typeText(int count) const override;
};


#endif							// TRACKPROPERTIESGENERALPAGES_H
