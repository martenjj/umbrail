//////////////////////////////////////////////////////////////////////////
//									//
//  Project:	Umbrail - GPX track viewer and editor			//
//									//
//////////////////////////////////////////////////////////////////////////
//									//
//  Copyright (c) 2014-2023 Jonathan Marten <jjm@keelhaul.me.uk>	//
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

#ifndef FILESLISTMODEL_H
#define FILESLISTMODEL_H
 
#include <kdescendantsproxymodel.h>
#include "itemindexinterface.h"


/**
 * @short A model to flatten the tree of track data into a linear list.
 *
 * This model does no processing on the data over and above that done
 * by KDescendantsProxyModel, but it needs to be a derived class so that
 * the index <-> item mapping can be passed down the model tree.
 */
class FilesListModel : public KDescendantsProxyModel, public ItemIndexInterface
{
    Q_OBJECT

public:
    explicit FilesListModel(QObject *pnt = nullptr);
    virtual ~FilesListModel() = default;
};
 
#endif							// FILESLISTMODEL_H
