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

#ifndef FILESMODEL_H
#define FILESMODEL_H
 
#include <qabstractitemmodel.h>
#include <qitemselectionmodel.h>

#include "itemindexinterface.h"


class FilesController;
class TrackDataItem;
class TrackDataFile;
class TrackDataAbstractPoint;


// This is deliberately outside the class definition so that it
// does not need to be qualified everywhere.
enum Column
{
    ColumnName,					// name
    ColumnSym,					// symbol
    ColumnOrigin,				// data source ID
    ColumnCoords,				// lat/long coordinates
    ColumnAddress,				// street address
    ColumnCats,					// catgeories
    ColumnCount					// how many - must be last
};


class FilesModel : public QAbstractItemModel, public ItemIndexInterface
{
    Q_OBJECT

public:
    FilesModel(QObject *pnt = nullptr);
    virtual ~FilesModel();

    virtual QModelIndex index(int row, int col, const QModelIndex &parent = QModelIndex()) const override;
    virtual QModelIndex parent(const QModelIndex &idx) const override;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual QVariant data(const QModelIndex &idx, int role) const override;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    virtual Qt::ItemFlags flags(const QModelIndex &idx) const override;
    virtual Qt::DropActions supportedDropActions() const override;
    virtual bool dropMimeData(const QMimeData *data, Qt::DropAction act, int row, int col, const QModelIndex &pnt) override;
    virtual bool canDropMimeData(const QMimeData *data, Qt::DropAction act, int row, int col, const QModelIndex &pnt) const override;
    virtual QStringList mimeTypes() const override;
    virtual QMimeData *mimeData(const QModelIndexList &idxs) const override;

    Q_DECL_DEPRECATED TrackDataFile *rootFileItem() const;
    bool isEmpty() const					{ return (mRootItem==nullptr); }
    TrackDataFile *takeRootFileItem();
    void setRootFileItem(TrackDataFile *root);

    static void sortByIndexRow(QList<TrackDataItem *> *list);

    // Tell the model to signal changes
    void changedItem(const TrackDataItem *item);
    void startLayoutChange();
    void endLayoutChange();

    // ItemIndexInterface
    TrackDataItem *itemForIndex(const QModelIndex &idx) const override;
    QModelIndex indexForItem(const TrackDataItem *tdi) const override;

    TrackDataItem *rootItem() const override			{ return (mRootItem); }

signals:
    void dragDropItems(const QList<TrackDataItem *> &sourceItems, TrackDataItem *ontoParent, int row);

private:
    bool dropMimeDataInternal(bool doit, const QMimeData *data, int row, const QModelIndex &pnt);

private:
    TrackDataItem *mRootItem;
};
 
#endif							// FILESMODEL_H
