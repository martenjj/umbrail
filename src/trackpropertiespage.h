
#ifndef TRACKPROPERTIESPAGE_H
#define TRACKPROPERTIESPAGE_H

#include <qwidget.h>

class QFormLayout;
class KTimeZone;
class TrackDataItem;


#define CREATE_PROPERTIES_PAGE(ITEMTYPE, PAGETYPE)					\
    TrackPropertiesPage *								\
        TrackData ## ITEMTYPE::createProperties ## PAGETYPE ## Page(			\
            const QList<TrackDataItem *> items,                         		\
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

protected:
    TrackPropertiesPage(const QList<TrackDataItem *> items, QWidget *pnt);

    void addSeparatorField(const QString &title = QString::null);
    void disableIfEmpty(QWidget *field);

protected:
    QFormLayout *mFormLayout;

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
