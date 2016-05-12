//////////////////////////////////////////////////////////////////////////
//									//
//  Project:	Track Editor						//
//  Edit:	12-May-16						//
//									//
//////////////////////////////////////////////////////////////////////////
//									//
//  Copyright (c) 2012 Jonathan Marten <jjm@keelhaul.me.uk>		//
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


//////////////////////////////////////////////////////////////////////////
//									//
//  Include files							//
//									//
//////////////////////////////////////////////////////////////////////////

#include <qapplication.h>
#include <qcommandlineparser.h>

#include <kaboutdata.h>
#include <klocalizedstring.h>

#include "mainwindow.h"
#include "filescontroller.h"

#include "version.h"

//////////////////////////////////////////////////////////////////////////
//									//
//  Main							 	//
//									//
//////////////////////////////////////////////////////////////////////////
 
int main(int argc,char *argv[])
{
    KAboutData aboutData("navtracks",			// componentName
                         i18n("NavTracks"),		// displayName
#ifdef VCS_HAVE_VERSION
                         ( VERSION " (" VCS_TYPE_STRING " " VCS_REVISION_STRING ")" ),
#else
                         VERSION,			// version
#endif
                         i18n("GPS track viewer and editor"),
                         KAboutLicense::GPL_V3,
                         i18n("Copyright (c) 2014-2016 Jonathan Marten"),
                         QString::null,			// otherText
                         "http://www.keelhaul.me.uk",	// homePageAddress
                        "jjm@keelhaul.me.uk");		// bugsEmailAddress
    aboutData.addAuthor(i18n("Jonathan Marten"),
                        QString::null,
                        "jjm@keelhaul.me.uk",
                        "http://www.keelhaul.me.uk");

    QApplication app(argc, argv);
    KAboutData::setApplicationData(aboutData);

    QCommandLineParser parser;
    parser.setApplicationDescription(aboutData.shortDescription());

    QCommandLineOption opt((QStringList() << "f" << "file"), i18n("Load a data file."), "file");
    parser.addOption(opt);

    aboutData.setupCommandLine(&parser);
    parser.process(app);

    MainWindow *w = new MainWindow(NULL);

    if (parser.isSet("file"))
    {
        const QUrl u = QUrl::fromUserInput(parser.value("file"));
        if (u.isValid())
        {
            const bool ok = w->loadProject(u);
            if (!ok) w->deleteLater();
        }
    }

    w->show();
    return (app.exec());
}
