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

#ifndef FILESVIEW_H
#define FILESVIEW_H
 
#include <qtreeview.h>
#include "applicationdatainterface.h"

#include "trackdata.h"


class FilesView : public QTreeView, public ApplicationDataInterface
{
    Q_OBJECT

public:
    FilesView(QWidget *pnt = nullptr);
    virtual ~FilesView();

    void readProperties();
    void saveProperties();

    int selectedCount() const			{ return (mSelectedCount); }
    TrackData::Type selectedType() const	{ return (mSelectedType); }
    const TrackDataItem *selectedItem() const	{ return (mSelectedItem); }
    QList<TrackDataItem *> selectedItems() const;
    QVector<const TrackDataAbstractPoint *> selectedPoints() const;

    void selectItem(const TrackDataItem *item, bool combine = false);
    void setMovePointsMode(bool on);

    unsigned long selectionId() const		{ return (mSelectionId); }

public slots:
    void slotSelectAllSiblings();
    void slotClickedItem(const QModelIndex &index, unsigned int flags);

    void slotCollapseAll();
    void slotExpandAll();

protected:
    void selectionChanged(const QItemSelection &sel, const QItemSelection &desel) override;
    void contextMenuEvent(QContextMenuEvent *ev) override;

signals:
    void updateActionState();

private:
    void expandItem(const QModelIndex &idx);

private:
    int mSelectedCount;
    TrackData::Type mSelectedType;
    const TrackDataItem *mSelectedItem;
    unsigned long mSelectionId;
};
 
#endif							// FILESVIEW_H
