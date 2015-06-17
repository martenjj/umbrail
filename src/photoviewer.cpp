//////////////////////////////////////////////////////////////////////////
//									//
//  Project:	NavTracks						//
//  Edit:	17-Jun-15						//
//									//
//////////////////////////////////////////////////////////////////////////
//									//
//  Copyright (c) 2012-2015 Jonathan Marten <jjm@keelhaul.me.uk>	//
//  Home and download page:  http://www.keelhaul.me.uk/TBD/		//
//									//
//  This program is free software; you can redistribute it and/or	//
//  modify it under the terms of the GNU General Public License as	//
//  published by the Free Software Foundation; either version 2 of	//
//  the License, or (at your option) any later version.			//
//									//
//  It is distributed in the hope that it will be useful, but		//
//  WITHOUT ANY WARRANTY; without even the implied warranty of		//
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the	//
//  GNU General Public License for more details.			//
//									//
//  You should have received a copy of the GNU General Public		//
//  License along with this program; see the file COPYING for further	//
//  details.  If not, write to the Free Software Foundation, Inc.,	//
//  59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.		//
//									//
//////////////////////////////////////////////////////////////////////////

#include "photoviewer.h"

#include <qgridlayout.h>
#include <qapplication.h>
#include <qdesktopwidget.h>
#include <qlabel.h>

#include <kdebug.h>
#include <klocale.h>
#include <kurl.h>
#include <kglobal.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kpushbutton.h>


PhotoViewer::PhotoViewer(const KUrl &url, QWidget *pnt)
    : QWidget(pnt)
{
    kDebug() << url;

    setObjectName("PhotoViewer");
    setWindowTitle(i18n("Photo Viewer"));
    setAttribute(Qt::WA_DeleteOnClose);
 
    QGridLayout *gl = new QGridLayout(this);

    QPixmap pix(url.path());
    if (pix.width()>pix.height())
    {
        pix = pix.scaled(800, 600, Qt::KeepAspectRatio);
        mAspect = "L";
    }
    else
    {
        pix = pix.scaled(600, 800, Qt::KeepAspectRatio);
        mAspect = "P";
    }

    QLabel *l = new QLabel(this);
    l->setPixmap(pix);
    l->setScaledContents(true);
    gl->addWidget(l, 0, 0, 1, -1);
    gl->setRowStretch(0, 1);

    QPushButton *but = new KPushButton(KStandardGuiItem::close(), this);
    connect(but, SIGNAL(clicked()), SLOT(close()));
    gl->addWidget(but, 1, 1, Qt::AlignRight);

    // from KDialog::restoreDialogSize()
    int scnum = QApplication::desktop()->screenNumber(parentWidget());
    QRect desk = QApplication::desktop()->screenGeometry(scnum);

    const KConfigGroup grp = KGlobal::config()->group(objectName());
    int w = grp.readEntry( QString::fromLatin1("Width %1 %2").arg(desk.width()).arg(mAspect), 400);
    int h = grp.readEntry(QString::fromLatin1("Height %1 %2").arg(desk.height()).arg(mAspect), 300);
    resize(w, h);
}


PhotoViewer::~PhotoViewer()
{
    // from KDialog::saveDialogSize()
    int scnum = QApplication::desktop()->screenNumber(parentWidget());
    QRect desk = QApplication::desktop()->screenGeometry(scnum);

    KConfigGroup grp = KGlobal::config()->group(objectName());
    const QSize sizeToSave = size();
    grp.writeEntry(QString::fromLatin1("Width %1 %2").arg(desk.width()).arg(mAspect), sizeToSave.width());
    grp.writeEntry(QString::fromLatin1("Height %1 %2").arg(desk.height()).arg(mAspect), sizeToSave.height());
    grp.sync();

    kDebug() << "done";
}
