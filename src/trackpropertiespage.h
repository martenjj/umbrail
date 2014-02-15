
#ifndef TRACKPROPERTIESPAGE_H
#define TRACKPROPERTIESPAGE_H

#include <qwidget.h>

class QFormLayout;

class TrackDataItem;





class TrackPropertiesPage : public QWidget
{
    Q_OBJECT

public:
    virtual ~TrackPropertiesPage()				{}

    virtual bool isDataValid() const				{ return (true); }

protected:
    TrackPropertiesPage(const QList<TrackDataItem *> items, QWidget *pnt);

    void addSpacerField();

protected:
    QFormLayout *mFormLayout;

protected slots:
    virtual void slotDataChanged();

signals:
    void dataChanged();

};

#endif							// TRACKPROPERTIESPAGE_H
