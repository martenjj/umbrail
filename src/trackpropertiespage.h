
#ifndef TRACKPROPERTIESPAGE_H
#define TRACKPROPERTIESPAGE_H

#include <qwidget.h>

class QFormLayout;
class KTimeZone;
class TrackDataItem;
class QLabel;


#define CREATE_PROPERTIES_PAGE(ITEMTYPE, PAGETYPE)					\
    TrackPropertiesPage *								\
        TrackData ## ITEMTYPE::createProperties ## PAGETYPE ## Page(			\
            const QList<TrackDataItem *> *items,                         		\
            QWidget *pnt) const                                         		\
    {											\
        return (new Track ## ITEMTYPE ## PAGETYPE ## Page(items, pnt));			\
    }


class TrackPropertiesPage : public QWidget
{
    Q_OBJECT

public:
    virtual ~TrackPropertiesPage();

    virtual bool isDataValid() const				{ return (true); }

    KTimeZone *timeZone() const					{ return (mTimeZone); }
    bool isEmpty() const					{ return (mIsEmpty); }

public slots:
    void setTimeZone(const QString &name);
    void slotPointPositionChanged(double newLat, double newLon);

protected:
    TrackPropertiesPage(const QList<TrackDataItem *> *items, QWidget *pnt);

    void addSeparatorField(const QString &title = QString::null);
    void disableIfEmpty(QWidget *field, bool always = false);

protected:
    QFormLayout *mFormLayout;
    QLabel *mPositionLabel;

protected slots:
    virtual void slotDataChanged();

signals:
    void dataChanged();
    void updateTimeZones(const KTimeZone *tz);

private:
    bool mIsEmpty;
    KTimeZone *mTimeZone;
};

#endif							// TRACKPROPERTIESPAGE_H
