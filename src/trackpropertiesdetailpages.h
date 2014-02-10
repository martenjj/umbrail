
#ifndef TRACKPROPERTIESDETAILPAGES_H
#define TRACKPROPERTIESDETAILPAGES_H

#include <qwidget.h>

#include <kurl.h>

class QLabel;
class QFormLayout;

class TrackDataItem;








class TrackItemDetailPage : public QWidget
{
    Q_OBJECT

public:
    virtual ~TrackItemDetailPage()				{}

    QString newItemName() const;

protected:
    TrackItemDetailPage(const QList<TrackDataItem *> items, QWidget *pnt);

    virtual bool isDataValid() const;

    void addTimeDistanceSpeedFields(const QList<TrackDataItem *> &items, bool bothTimes = true);
    void addBoundingAreaField(const QList<TrackDataItem *> &items);
    void addChildCountField(const QList<TrackDataItem *> &items, const QString &labelText);
    void addSpacerField();

protected:
    QFormLayout *mFormLayout;

protected slots:
    void slotDataChanged();

signals:
    void enableButtonOk(bool);



};




class TrackFileDetailPage : public TrackItemDetailPage
{
    Q_OBJECT

public:
    TrackFileDetailPage(const QList<TrackDataItem *> items, QWidget *pnt);
    virtual ~TrackFileDetailPage()				{}

};



class TrackTrackDetailPage : public TrackItemDetailPage
{
    Q_OBJECT

public:
    TrackTrackDetailPage(const QList<TrackDataItem *> items, QWidget *pnt);
    virtual ~TrackTrackDetailPage()				{}


};



class TrackSegmentDetailPage : public TrackItemDetailPage
{
    Q_OBJECT

public:
    TrackSegmentDetailPage(const QList<TrackDataItem *> items, QWidget *pnt);
    virtual ~TrackSegmentDetailPage()				{}


};



class TrackPointDetailPage : public TrackItemDetailPage
{
    Q_OBJECT

public:
    TrackPointDetailPage(const QList<TrackDataItem *> items, QWidget *pnt);
    virtual ~TrackPointDetailPage()				{}


};




#endif							// TRACKPROPERTIESDETAILPAGES_H
