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

#include "applicationdatainterface.h"

#include <qobject.h>
#include <qwidget.h>

#include "applicationdata.h"


ApplicationDataInterface::ApplicationDataInterface(QObject *pnt)
{
    QObject *obj = pnt;
    mApplicationData = nullptr;
    while (obj!=nullptr)
    {
        mApplicationData = dynamic_cast<ApplicationData *>(obj);
        if (mApplicationData!=nullptr) break;
        obj = obj->parent();
    }

    if (mApplicationData==nullptr)
    {
        qFatal("ApplicationDataInterface: Parent '%s' must be a descendent of ApplicationData", qPrintable(pnt->objectName()));
    }
}


FilesController *ApplicationDataInterface::filesController() const
{
    return (mApplicationData->filesController());
}


FilesView *ApplicationDataInterface::filesView() const
{
    return (mApplicationData->filesView());
}


MapController *ApplicationDataInterface::mapController() const
{
    return (mApplicationData->mapController());
}


QWidget *ApplicationDataInterface::mainWidget() const
{
    return (mApplicationData->mainWidget());
}


bool ApplicationDataInterface::isReadOnly() const
{
    return (mApplicationData->isReadOnly());
}


void ApplicationDataInterface::executeCommand(QUndoCommand *cmd)
{
    QMetaObject::invokeMethod(mApplicationData->mainWidget(),
                              "slotExecuteCommand",
                              Q_ARG(QUndoCommand *, cmd));
}
