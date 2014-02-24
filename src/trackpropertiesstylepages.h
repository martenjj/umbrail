
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
    TrackItemStylePage(const QList<TrackDataItem *> items, QWidget *pnt);

protected:
    KColorButton *mLineColourButton;
    QCheckBox *mLineInheritCheck;

protected slots:
    void slotColourChanged(const QColor &col);

};




class TrackFileStylePage : public TrackItemStylePage
{
    Q_OBJECT

public:
    TrackFileStylePage(const QList<TrackDataItem *> items, QWidget *pnt);
    virtual ~TrackFileStylePage()				{}

};



class TrackTrackStylePage : public TrackItemStylePage
{
    Q_OBJECT

public:
    TrackTrackStylePage(const QList<TrackDataItem *> items, QWidget *pnt);
    virtual ~TrackTrackStylePage()				{}


};



class TrackSegmentStylePage : public TrackItemStylePage
{
    Q_OBJECT

public:
    TrackSegmentStylePage(const QList<TrackDataItem *> items, QWidget *pnt);
    virtual ~TrackSegmentStylePage()				{}


};



class TrackPointStylePage : public TrackItemStylePage
{
    Q_OBJECT

public:
    TrackPointStylePage(const QList<TrackDataItem *> items, QWidget *pnt);
    virtual ~TrackPointStylePage()				{}


};




#endif							// TRACKPROPERTIESSTYLEPAGES_H