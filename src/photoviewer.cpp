//////////////////////////////////////////////////////////////////////////
//									//
//  Project:	NavTracks						//
//  Edit:	27-Sep-15						//
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

#include <kdebug.h>
#include <kurl.h>
#include <kservice.h>
#include <kmimetype.h>
#include <kmimetypetrader.h>
#include <kactioncollection.h>
#include <kstandardaction.h>
#include <kmenubar.h>

#include "settings.h"


PhotoViewer::PhotoViewer(const KUrl &url, QWidget *pnt)
    : KParts::MainWindow(pnt, Qt::Window)
{
    kDebug() << url;

    setObjectName("PhotoViewer");
    setWindowTitle(i18n("Photo Viewer"));
    setAttribute(Qt::WA_DeleteOnClose);
    setXMLFile("viewerui.rc");

    KService::Ptr service;

    QString viewMode = Settings::photoViewMode();	// selected view mode from settings
    kDebug() << "view mode from settings" << viewMode;
    if (!viewMode.isEmpty())				// if there is one,
    {							// get the service from that
        if (viewMode.endsWith(".desktop")) viewMode.chop(8);
        service = KService::serviceByDesktopName(viewMode);
        if (service.isNull())
        {
            kWarning() << "Viewer part" << viewMode << "not available";
            return;
        }
    }
    else
    {
        KMimeType::Ptr mimeType = KMimeType::findByUrl(url);
        kDebug() << "mime type" << mimeType->name();	// get services for MIME type
        KService::List services = KMimeTypeTrader::self()->query(mimeType->name(), "KParts/ReadOnlyPart");
        if (services.isEmpty())
        {
            kWarning() << "No viewer parts available for" << mimeType->name();
            return;
        }

        service = services.first();			// take the first preference
    }

    Q_ASSERT(!service.isNull());
    kDebug() << "  service" << service->name() << "id" << service->storageId();

    // from https://techbase.kde.org/Development/Tutorials/Using_KParts
    mPart = service->createInstance<KParts::ReadOnlyPart>(NULL);
    if (mPart==NULL)
    {
        kWarning() << "Unable to create viewer part";
        return;
    }

    // Set up actions
    actionCollection()->addAction(KStandardAction::Close, "file_close", this, SLOT(close()));

    // Create/merge the GUI
    setCentralWidget(mPart->widget());
    setupGUI(KXmlGuiWindow::ToolBar|KXmlGuiWindow::Keys);
    createGUI(mPart);
    setFocusPolicy(Qt::StrongFocus);

    // Our own XMLGUI file for this main window has to contain a menu bar
    // item for any menu that the part may provide;  otherwise, any such
    // new menu bar items will appear at the end of our menu bar!  However,
    // this results in these menu bar items appearing (as empty) even if the
    // part doesn't provide any actions for them.  So, after the part's XMLGUI
    // has been merged with ours, we hide any empty menu bar items.
    QList<QAction *> acts = menuBar()->actions();
    for (int i = 0; i<acts.count(); ++i)
    {
        QAction *act = acts[i];
        QMenu *menu = act->menu();
        if (menu!=NULL && menu->isEmpty())
        {
            kDebug() << "hiding empty menu" << act->text();
            act->setVisible(false);
        }
    }

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
    kDebug() << "done";
}
