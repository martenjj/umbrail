//////////////////////////////////////////////////////////////////////////
//									//
//  Project:	NavTracks						//
//  Edit:	13-May-16						//
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

#include <qevent.h>
#include <qdebug.h>

//#include <kurl.h>
#include <kservice.h>
#include <kmimetype.h>
#include <kmimetypetrader.h>
#include <kactioncollection.h>
#include <kstandardaction.h>
#include <kmenubar.h>
#include <klocalizedstring.h>

#include "settings.h"


PhotoViewer::PhotoViewer(const QUrl &url, QWidget *pnt)
    : KParts::MainWindow(pnt, Qt::Window)
{
    qDebug() << url;

    setObjectName("PhotoViewer");
    setWindowTitle(i18nc("@title:window", "Photo Viewer"));
    setAttribute(Qt::WA_DeleteOnClose);
    setXMLFile("viewerui.rc");

    KService::Ptr service;

    QString viewMode = Settings::photoViewMode();	// selected view mode from settings
    qDebug() << "view mode from settings" << viewMode;
    if (!viewMode.isEmpty())				// if there is one,
    {							// get the service from that
        if (viewMode.endsWith(".desktop")) viewMode.chop(8);
        service = KService::serviceByDesktopName(viewMode);
        if (service==nullptr)
        {
            qWarning() << "Viewer part" << viewMode << "not available";
            return;
        }
    }
    else
    {
        KMimeType::Ptr mimeType = KMimeType::findByUrl(url);
        qDebug() << "mime type" << mimeType->name();	// get services for MIME type
        KService::List services = KMimeTypeTrader::self()->query(mimeType->name(), "KParts/ReadOnlyPart");
        if (services.isEmpty())
        {
            qWarning() << "No viewer parts available for" << mimeType->name();
            return;
        }

        service = services.first();			// take the first preference
    }

    Q_ASSERT(service!=nullptr);
    qDebug() << "  service" << service->name() << "id" << service->storageId();

    // from https://techbase.kde.org/Development/Tutorials/Using_KParts
    mPart = service->createInstance<KParts::ReadOnlyPart>(NULL);
    if (mPart==NULL)
    {
        qWarning() << "Unable to create viewer part";
        return;
    }

    // Set up actions
    actionCollection()->addAction(KStandardAction::Close, "file_close", this, SLOT(close()));

    // Create/merge the GUI
    setCentralWidget(mPart->widget());
    setupGUI(KXmlGuiWindow::ToolBar|KXmlGuiWindow::Keys);
    createGUI(mPart);
    setFocusPolicy(Qt::StrongFocus);
    fixupMenuBar(menuBar());

    setAutoSaveSettings(objectName(), true);
    mPart->openUrl(url);
}


void PhotoViewer::keyPressEvent(QKeyEvent *ev)
{
    if (ev->key()==Qt::Key_Escape) close();
    else QWidget::keyPressEvent(ev);
}


PhotoViewer::~PhotoViewer()
{
    qDebug() << "done";
}


//  The viewer part may provide its own menu bar entries, which are supposed
//  to be merged with the existing ones (since we actually define no menus of
//  our own, all from the standard XMLGUI definition) at the appropriate place.
//  However, what actually seems to happen is that an new menu bar entries which
//  the part provides appear at the end of the menu bar!
//
//  We cannot specify those menus at the appropriate place in our own XMLGUI
//  file, since this would mean having to anticipiate all of the entries that a
//  part may provide, and also the possibility of ending up with blank menu
//  entries if the part doesn't provide them.  So what we do is to look along
//  the menu bar, noting the actions for the "Settings", right alignment
//  separator and "Help", assuming that they all come in that order with nothing
//  intervening.  Then we remove those actions from the menu bar, and add them
//  at the end in the appropriate order.

void PhotoViewer::fixupMenuBar(QMenuBar *bar)
{
    qDebug();

    QAction *sepAct = NULL;
    QAction *helpAct = NULL;
    QAction *settAct = NULL;

    QList<QAction *> acts = bar->actions();
    for (int i = 0; i<acts.count(); ++i)
    {
        QAction *act = acts[i];
        //qDebug() << "act" << i << act->text() << "menu?" << (act->menu()!=NULL);
        if (sepAct==NULL && act->text().isEmpty())			// the separator?
        {
            qDebug() << "separator found at" << i;
            sepAct = act;
            if (i>0) settAct = acts[i-1];
            continue;
        }

        if (sepAct!=NULL && helpAct==NULL)
        {
            qDebug() << "help" << act->text() << "found at" << i;
            helpAct = act;
            break;
        }
    }

    if (settAct!=NULL)
    {
        qDebug() << "moving settings to end";
        bar->removeAction(settAct);
        bar->addAction(settAct);
    }

    if (sepAct!=NULL)
    {
        qDebug() << "moving separator to end";
        bar->removeAction(sepAct);
        bar->addAction(sepAct);
    }

    if (helpAct!=NULL)
    {
        qDebug() << "moving help to end";
        bar->removeAction(helpAct);
        bar->addAction(helpAct);
    }
}
