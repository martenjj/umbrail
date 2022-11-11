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

#ifndef METADATAMODEL_H
#define METADATAMODEL_H
 
#include <QAbstractTableModel>


class QTimeZone;
class TrackDataItem;


class MetadataModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit MetadataModel(const TrackDataItem *item, QObject *pnt = nullptr);
    virtual ~MetadataModel();

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &idx, int role) const override;
    bool setData(const QModelIndex &idx, const QVariant &value, int role) override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    const QVariant data(int idx) const;
    const QVariant data(const QString &nm) const;
    void setData(int idx, const QVariant &value);
    bool isChanged(int idx) const;

    const QTimeZone *timeZone() const			{ return (mTimeZone); }
    double latitude() const;
    double longitude() const;

signals:
    void metadataChanged(int idx);

private:
    void resolveTimeZone();

private:
    QMap<int,QVariant> mItemData;
    QMap<int,bool> mItemChanged;

    QString mParentTimeZone;
    bool mUseParentTimeZone;
    QTimeZone *mTimeZone;
};
 
#endif							// METADATAMODEL_H
