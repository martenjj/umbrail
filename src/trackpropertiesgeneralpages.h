
#ifndef TRACKPROPERTIESGENERALPAGES_H
#define TRACKPROPERTIESGENERALPAGES_H

#include <qwidget.h>

#include <kurl.h>

class QLabel;
class QDateTime;
class QFormLayout;
class KLineEdit;
class KUrlRequester;
class TrackDataItem;








class TrackItemGeneralPage : public QWidget
{
    Q_OBJECT

public:
    virtual ~TrackItemGeneralPage()				{}

    QString newItemName() const;

    virtual QString typeText(int count) const = 0;

protected:
    TrackItemGeneralPage(const QList<TrackDataItem *> items, QWidget *pnt);

    virtual bool isDataValid() const;

    void addTimeFields(const QList<TrackDataItem *> &items);
    void addSpacerField();

protected:
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

    QString typeText(int count) const;

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

    QString typeText(int count) const;

};



class TrackSegmentGeneralPage : public TrackItemGeneralPage
{
    Q_OBJECT

public:
    TrackSegmentGeneralPage(const QList<TrackDataItem *> items, QWidget *pnt);
    virtual ~TrackSegmentGeneralPage()				{}

    QString typeText(int count) const;

};



class TrackPointGeneralPage : public TrackItemGeneralPage
{
    Q_OBJECT

public:
    TrackPointGeneralPage(const QList<TrackDataItem *> items, QWidget *pnt);
    virtual ~TrackPointGeneralPage()				{}

    QString typeText(int count) const;

};




#endif							// TRACKPROPERTIESGENERALPAGES_H
