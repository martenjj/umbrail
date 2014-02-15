
#ifndef TRACKPROPERTIESGENERALPAGES_H
#define TRACKPROPERTIESGENERALPAGES_H

#include <kurl.h>

#include "trackpropertiespage.h"

class QLabel;
class QDateTime;
class QFormLayout;
class KLineEdit;
class KUrlRequester;
class TrackDataItem;








class TrackItemGeneralPage : public TrackPropertiesPage
{
    Q_OBJECT

public:
    virtual ~TrackItemGeneralPage()				{}

    QString newItemName() const;

    virtual QString typeText(int count) const = 0;
    virtual bool isDataValid() const;

protected:
    TrackItemGeneralPage(const QList<TrackDataItem *> items, QWidget *pnt);

    void addTimeFields(const QList<TrackDataItem *> &items);

protected:
    KLineEdit *mNameEdit;
};




class TrackFileGeneralPage : public TrackItemGeneralPage
{
    Q_OBJECT

public:
    TrackFileGeneralPage(const QList<TrackDataItem *> items, QWidget *pnt);
    virtual ~TrackFileGeneralPage()				{}

    KUrl newFileUrl() const;
    virtual bool isDataValid() const;

    QString typeText(int count) const;

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
