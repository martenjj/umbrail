
#ifndef TRACKPROPERTIESPAGE_H
#define TRACKPROPERTIESPAGE_H

#include <qwidget.h>

#include "mainwindowinterface.h"

class QFormLayout;

class TrackDataItem;
class MetadataModel;


#define CREATE_PROPERTIES_PAGE(ITEMTYPE, PAGETYPE)					\
    TrackPropertiesPage *								\
    TrackData ## ITEMTYPE::createProperties ## PAGETYPE ## Page(       			\
            const QList<TrackDataItem *> *items,                         		\
            QWidget *pnt) const                                         		\
    {											\
        return (new Track ## ITEMTYPE ## PAGETYPE ## Page(items, pnt));			\
    }

#define NULL_PROPERTIES_PAGE(ITEMTYPE, PAGETYPE)					\
    TrackPropertiesPage *								\
        TrackData ## ITEMTYPE::createProperties ## PAGETYPE ## Page(			\
            const QList<TrackDataItem *> *items,                         		\
            QWidget *pnt) const                                         		\
    {											\
        Q_UNUSED(items);								\
        Q_UNUSED(pnt);									\
        return (nullptr);								\
    }


class TrackPropertiesPage : public QWidget, public MainWindowInterface
{
    Q_OBJECT

public:
    virtual ~TrackPropertiesPage() = default;

    virtual bool isDataValid() const				{ return (true); }
    void setDataModel(MetadataModel *dataModel)			{ mDataModel = dataModel; }

    virtual void refreshData() = 0;

    bool isEmpty() const					{ return (mIsEmpty); }

protected:
    TrackPropertiesPage(const QList<TrackDataItem *> *items, QWidget *pnt);

    MetadataModel *dataModel() const				{ return (mDataModel); }

    void addSeparatorField(const QString &title = QString());
    void disableIfEmpty(QWidget *field, bool always = false);

protected:
    QFormLayout *mFormLayout;

private:
    bool mIsEmpty;
    MetadataModel *mDataModel;
};

#endif							// TRACKPROPERTIESPAGE_H
