
#ifndef TRACKPROPERTIESPAGE_H
#define TRACKPROPERTIESPAGE_H

#include <qwidget.h>

class QFormLayout;
class KTimeZone;
class TrackDataItem;





class TrackPropertiesPage : public QWidget
{
    Q_OBJECT

public:
    virtual ~TrackPropertiesPage();

    virtual bool isDataValid() const				{ return (true); }

    KTimeZone *timeZone() const					{ return (mTimeZone); }

public slots:
    void setTimeZone(const QString &name);

protected:
    TrackPropertiesPage(const QList<TrackDataItem *> items, QWidget *pnt);

    void addSeparatorField(const QString &title = QString::null);

protected:
    QFormLayout *mFormLayout;

protected slots:
    virtual void slotDataChanged();

signals:
    void dataChanged();
    void updateTimeZones(const KTimeZone *tz);

private:
    KTimeZone *mTimeZone;
};

#endif							// TRACKPROPERTIESPAGE_H
