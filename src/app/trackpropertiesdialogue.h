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

#ifndef TRACKPROPERTIESDIALOGUE_H
#define TRACKPROPERTIESDIALOGUE_H


#include <qlist.h>

#include <kfdialog/dialogbase.h>
#include <kfdialog/dialogstatesaver.h>

#include "trackdata.h"


class QTabWidget;

class TrackDataItem;
class MetadataModel;


class TrackPropertiesDialogue : public DialogBase, public DialogStateSaver
{
    Q_OBJECT

public:
    TrackPropertiesDialogue(const QList<TrackDataItem *> *items, QWidget *pnt = nullptr);
    virtual ~TrackPropertiesDialogue() = default;

    MetadataModel *dataModel() const			{ return (mDataModel); }

    static void setNextPageIndex(int page);

protected:
    void saveConfig(QDialog *dialog, KConfigGroup &grp) const override;
    void restoreConfig(QDialog *dialog, const KConfigGroup &grp) override;

    void showEvent(QShowEvent *ev) override;

private:
    void addPage(TrackPropertiesPage *page, const QString &title, bool enabled = true);

protected slots:
    void slotModelDataChanged(int idx);
    void slotTabChanged(int idx);

private:
    QTabWidget *mTabWidget;
    TrackData::Type mItemType;
    MetadataModel *mDataModel;
    QVector<bool> mPageDataChanged;
    bool mCloseButtonShown;
};

#endif							// TRACKPROPERTIESDIALOGUE_H
