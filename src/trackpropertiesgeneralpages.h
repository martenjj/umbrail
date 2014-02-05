
#ifndef TRACKPROPERTIESGENERALPAGES_H
#define TRACKPROPERTIESGENERALPAGES_H

#include <qwidget.h>
#include <qlabel.h>

#include <kurl.h>

class QDateTime;
class QFormLayout;
class KLineEdit;
class KUrlRequester;
class TrackDataItem;






class TrackDataLabel : public QLabel
{
    Q_OBJECT

public:
    TrackDataLabel(const QString &str, QWidget *parent = NULL);
    TrackDataLabel(int i, QWidget *parent = NULL);
    TrackDataLabel(const QDateTime &dt, QWidget *parent = NULL);
    TrackDataLabel(double lat, double lon, QWidget *parent = NULL);

    virtual ~TrackDataLabel()					{}

private:
    void init();
};





class TrackItemGeneralPage : public QWidget
{
    Q_OBJECT

public:
    virtual ~TrackItemGeneralPage()				{}

    QString newItemName() const;

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

    KUrl newFileUrl() const;

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




#endif							// TRACKPROPERTIESGENERALPAGES_H
