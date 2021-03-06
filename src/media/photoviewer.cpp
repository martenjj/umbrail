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

#include "photoviewer.h"

#include <qevent.h>
#include <qdebug.h>
#include <qmimetype.h>
#include <qmimedatabase.h>
#include <qmenubar.h>

#include <kactioncollection.h>
#include <kstandardaction.h>
#include <klocalizedstring.h>
#include <kpluginfactory.h>
#ifdef USE_KSERVICE
#include <kservice.h>
#endif
#include <kparts/partloader.h>

#include "settings.h"


PhotoViewer::PhotoViewer(const QUrl &url, QWidget *pnt)
    : KParts::MainWindow(pnt, Qt::Window)
{
    qDebug() << url;

    setObjectName("PhotoViewer");
    setWindowTitle(i18nc("@title:window", "Photo Viewer"));
    setAttribute(Qt::WA_DeleteOnClose);
    setXMLFile("viewerui.rc");

    mPart = nullptr;
    QString errorString;

    QString viewMode = Settings::photoViewMode();	// selected view mode from settings
    qDebug() << "view mode from settings" << viewMode;
    if (!viewMode.isEmpty())				// if there is one,
    {							// get the service from that
        if (viewMode.endsWith(".desktop")) viewMode.chop(8);
#ifdef USE_KSERVICE
        KService::Ptr service = KService::serviceByDesktopName(viewMode);
        if (service==nullptr)
        {
            qWarning() << "Viewer part" << viewMode << "not available";
            return;
        }

        qDebug() << "  service" << service->name() << "id" << service->storageId();
        // from https://techbase.kde.org/Development/Tutorials/Using_KParts
        mPart = service->createInstance<KParts::ReadOnlyPart>(this, nullptr, QVariantList(), &errorString);
#else
        const KPluginMetaData pluginData("kf5/parts/"+viewMode);
        auto result = KPluginFactory::instantiatePlugin<KParts::ReadOnlyPart>(pluginData);
        mPart = result.plugin;
        if (mPart==nullptr) errorString = result.errorString;
#endif
    }
    else
    {
        QMimeDatabase db;
        QMimeType mimeType = db.mimeTypeForUrl(url);
        qDebug() << "mime type" << mimeType.name();	// create part for MIME type

        mPart = KParts::PartLoader::createPartInstanceForMimeType<KParts::ReadOnlyPart>(mimeType.name(),
                                                                                        this, this, &errorString);
    }

    if (mPart==nullptr)
    {
        qWarning() << "Unable to create viewer part," << errorString;
        return;
    }

    // Set up actions
    actionCollection()->addAction(KStandardAction::Close, "file_close", this, &QWidget::close);

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

    QAction *sepAct = nullptr;
    QAction *helpAct = nullptr;
    QAction *settAct = nullptr;

    QList<QAction *> acts = bar->actions();
    for (int i = 0; i<acts.count(); ++i)
    {
        QAction *act = acts[i];
        //qDebug() << "act" << i << act->text() << "menu?" << (act->menu()!=nullptr);
        if (sepAct==nullptr && act->text().isEmpty())			// the separator?
        {
            qDebug() << "separator found at" << i;
            sepAct = act;
            if (i>0) settAct = acts[i-1];
            continue;
        }

        if (sepAct!=nullptr && helpAct==nullptr)
        {
            qDebug() << "help" << act->text() << "found at" << i;
            helpAct = act;
            break;
        }
    }

    if (settAct!=nullptr)
    {
        qDebug() << "moving settings to end";
        bar->removeAction(settAct);
        bar->addAction(settAct);
    }

    if (sepAct!=nullptr)
    {
        qDebug() << "moving separator to end";
        bar->removeAction(sepAct);
        bar->addAction(sepAct);
    }

    if (helpAct!=nullptr)
    {
        qDebug() << "moving help to end";
        bar->removeAction(helpAct);
        bar->addAction(helpAct);
    }
}
