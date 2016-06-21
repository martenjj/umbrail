
#ifndef TRACKPROPERTIESSTYLEPAGES_H
#define TRACKPROPERTIESSTYLEPAGES_H

#include "trackpropertiespage.h"


class QFormLayout;
class QCheckBox;

class Style;
class TrackDataItem;

class KColorButton;






class TrackItemStylePage : public TrackPropertiesPage
{
    Q_OBJECT

public:
    virtual ~TrackItemStylePage()				{}

    QString newItemName() const;
    const Style newStyle() const;

protected:
    TrackItemStylePage(const QList<TrackDataItem *> *items, QWidget *pnt);

protected:
    KColorButton *mLineColourButton;
    QCheckBox *mLineInheritCheck;
    KColorButton *mPointColourButton;
    QCheckBox *mPointInheritCheck;

protected slots:
    void slotColourChanged(const QColor &col);

};




class TrackFileStylePage : public TrackItemStylePage
{
    Q_OBJECT

public:
    TrackFileStylePage(const QList<TrackDataItem *> *items, QWidget *pnt);
    virtual ~TrackFileStylePage()				{}

};



class TrackTrackStylePage : public TrackItemStylePage
{
    Q_OBJECT

public:
    TrackTrackStylePage(const QList<TrackDataItem *> *items, QWidget *pnt);
    virtual ~TrackTrackStylePage()				{}


};



class TrackRouteStylePage : public TrackItemStylePage
{
    Q_OBJECT

public:
    TrackRouteStylePage(const QList<TrackDataItem *> *items, QWidget *pnt);
    virtual ~TrackRouteStylePage()				{}


};



class TrackSegmentStylePage : public TrackItemStylePage
{
    Q_OBJECT

public:
    TrackSegmentStylePage(const QList<TrackDataItem *> *items, QWidget *pnt);
    virtual ~TrackSegmentStylePage()				{}


};



class TrackTrackpointStylePage : public TrackItemStylePage
{
    Q_OBJECT

public:
    TrackTrackpointStylePage(const QList<TrackDataItem *> *items, QWidget *pnt);
    virtual ~TrackTrackpointStylePage()				{}


};



class TrackFolderStylePage : public TrackItemStylePage
{
    Q_OBJECT

public:
    TrackFolderStylePage(const QList<TrackDataItem *> *items, QWidget *pnt);
    virtual ~TrackFolderStylePage()				{}


};



class TrackWaypointStylePage : public TrackItemStylePage
{
    Q_OBJECT

public:
    TrackWaypointStylePage(const QList<TrackDataItem *> *items, QWidget *pnt);
    virtual ~TrackWaypointStylePage()				{}


};



class TrackRoutepointStylePage : public TrackItemStylePage
{
    Q_OBJECT

public:
    TrackRoutepointStylePage(const QList<TrackDataItem *> *items, QWidget *pnt);
    virtual ~TrackRoutepointStylePage()				{}


};



#endif							// TRACKPROPERTIESSTYLEPAGES_H
