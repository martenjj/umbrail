//////////////////////////////////////////////////////////////////////////
//									//
//  Project:	Umbrail - GPX track viewer and editor			//
//									//
//////////////////////////////////////////////////////////////////////////
//									//
//  Copyright (c) 2014-2021 Jonathan Marten <jjm@keelhaul.me.uk>	//
//  Home and download page: <http://github.com/martenjj/umbrail>	//
//									//
//  This program is free software; you can redistribute it and/or	//
//  modify it under the terms of the GNU General Public License as	//
//  published by the Free Software Foundation, either version 3 of	//
//  the License or (at your option) any later version.			//
//									//
//  It is distributed in the hope that it will be useful, but		//
//  WITHOUT ANY WARRANTY;  without even the implied warranty of		//
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the	//
//  GNU General Public License for more details.			//
//									//
//  You should have received a copy of the GNU General Public License	//
//  along with this program;  see the file COPYING for further		//
//  details.  If not, see <http://gnu.org/licenses/gpl>.      		//
//									//
//////////////////////////////////////////////////////////////////////////

#ifndef TRACKPROPERTIESPAGE_H
#define TRACKPROPERTIESPAGE_H

#include <qwidget.h>

#include "applicationdatainterface.h"

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


class TrackPropertiesPage : public QWidget, public ApplicationDataInterface
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
