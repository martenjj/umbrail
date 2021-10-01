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

#ifndef FOLDERSELECTWIDGET_H
#define FOLDERSELECTWIDGET_H


#include <qframe.h>
#include "applicationdatainterface.h"

class QLineEdit;


class FolderSelectWidget : public QFrame, public ApplicationDataInterface
{
    Q_OBJECT

public:
    explicit FolderSelectWidget(QWidget *pnt = nullptr);
    virtual ~FolderSelectWidget() = default;

    QString folderPath() const;
    void setFolderPath(const QString &path, bool asDefault);

signals:
    void folderChanged(const QString &path);

protected slots:
    void slotSelectFolder();

private:
    QLineEdit *mDestFolder;
    QString mDefaultFolder;
};

#endif							// FOLDERSELECTWIDGET_H
