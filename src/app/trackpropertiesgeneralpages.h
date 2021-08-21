
#ifndef TRACKPROPERTIESGENERALPAGES_H
#define TRACKPROPERTIESGENERALPAGES_H

#include "trackpropertiespage.h"
#include "trackdata.h"

class QLabel;
class QDateTime;
class QFormLayout;
class QComboBox;
class QLineEdit;

class KTextEdit;

class ItemTypeCombo;
class TrackDataItem;
class TrackDataAbstractPoint;
class TrackDataWaypoint;
class TimeZoneSelector;
class TrackDataLabel;


class TrackItemGeneralPage : public TrackPropertiesPage
{
    Q_OBJECT

public:
    virtual ~TrackItemGeneralPage() = default;

    virtual QString typeText(int count) const = 0;
    virtual bool isDataValid() const override;
    virtual void refreshData() override;

protected:
    TrackItemGeneralPage(const QList<TrackDataItem *> *items, QWidget *pnt);

    void addTimeSpanFields(const QList<TrackDataItem *> *items);
    void addTypeField(const QList<TrackDataItem *> *items);
    void addDescField(const QList<TrackDataItem *> *items);
    void addPositionFields(const QList<TrackDataItem *> *items);
    void addTimeField(const QList<TrackDataItem *> *items);

protected slots:
    void slotNameChanged(const QString &text);
    void slotTypeChanged(const QString &text);
    void slotDescChanged();

    void slotChangePosition();

signals:
    void timeZoneChanged(const QString &zone);
    void pointPositionChanged(double newLat, double newLon);

protected:
    QLineEdit *mNameEdit;
    ItemTypeCombo *mTypeCombo;
    KTextEdit *mDescEdit;
    QLabel *mPositionLabel;

    TrackDataLabel *mTimeLabel;
    TrackDataLabel *mTimeStartLabel;
    TrackDataLabel *mTimeEndLabel;

private:
    bool mHasExplicitName;
};


class TrackFileGeneralPage : public TrackItemGeneralPage
{
    Q_OBJECT

public:
    TrackFileGeneralPage(const QList<TrackDataItem *> *items, QWidget *pnt);
    virtual ~TrackFileGeneralPage() = default;

    bool isDataValid() const override;
    QString typeText(int count) const override;
    void refreshData() override;

private slots:
    void slotTimeZoneChanged(const QString &zoneName);

private:
    QLineEdit *mUrlRequester;
    TimeZoneSelector *mTimeZoneSel;
};


class TrackTrackGeneralPage : public TrackItemGeneralPage
{
    Q_OBJECT

public:
    TrackTrackGeneralPage(const QList<TrackDataItem *> *items, QWidget *pnt);
    virtual ~TrackTrackGeneralPage() = default;

    QString typeText(int count) const override;
};


class TrackSegmentGeneralPage : public TrackItemGeneralPage
{
    Q_OBJECT

public:
    TrackSegmentGeneralPage(const QList<TrackDataItem *> *items, QWidget *pnt);
    virtual ~TrackSegmentGeneralPage() = default;

    QString typeText(int count) const override;
};


class TrackTrackpointGeneralPage : public TrackItemGeneralPage
{
    Q_OBJECT

public:
    TrackTrackpointGeneralPage(const QList<TrackDataItem *> *items, QWidget *pnt);
    virtual ~TrackTrackpointGeneralPage() = default;

    QString typeText(int count) const override;
};


class TrackFolderGeneralPage : public TrackItemGeneralPage
{
    Q_OBJECT

public:
    TrackFolderGeneralPage(const QList<TrackDataItem *> *items, QWidget *pnt);
    virtual ~TrackFolderGeneralPage() = default;

    QString typeText(int count) const override;
};


class TrackWaypointGeneralPage : public TrackItemGeneralPage
{
    Q_OBJECT

public:
    TrackWaypointGeneralPage(const QList<TrackDataItem *> *items, QWidget *pnt);
    virtual ~TrackWaypointGeneralPage() = default;

    QString typeText(int count) const override;
    void refreshData() override;

protected slots:
    void slotPlayAudioNote();
    void slotPlayVideoNote();
    void slotViewPhotoNote();

private:
    void addStatusField(const QList<TrackDataItem *> *items);

private slots:
    void slotStatusChanged(int idx);

private:
    const TrackDataWaypoint *mWaypoint;
    QComboBox *mStatusCombo;
};


class TrackRouteGeneralPage : public TrackItemGeneralPage
{
    Q_OBJECT

public:
    TrackRouteGeneralPage(const QList<TrackDataItem *> *items, QWidget *pnt);
    virtual ~TrackRouteGeneralPage() = default;

    QString typeText(int count) const override;
};


class TrackRoutepointGeneralPage : public TrackItemGeneralPage
{
    Q_OBJECT

public:
    TrackRoutepointGeneralPage(const QList<TrackDataItem *> *items, QWidget *pnt);
    virtual ~TrackRoutepointGeneralPage() = default;

    QString typeText(int count) const override;
};

#endif							// TRACKPROPERTIESGENERALPAGES_H
