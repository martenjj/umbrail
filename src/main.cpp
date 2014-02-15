//////////////////////////////////////////////////////////////////////////
//									//
//  Project:	Track Editor						//
//  Edit:	13-Feb-14						//
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

#include <kdebug.h>
#include <kapplication.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>

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
    KAboutData aboutData("navtracks",			// appName
                         NULL,				// catalogName
                         ki18n("NavTracks"),		// programName
#ifdef VCS_HAVE_VERSION
                         ( VERSION " (" VCS_TYPE_STRING " " VCS_REVISION_STRING ")" ),
#else
                         VERSION,				// version
#endif
                         ki18n("GPS Tracks viewer and editor"),
                         KAboutData::License_GPL_V3,
                         ki18n("Copyright (c) 2014 Jonathan Marten"),
                         KLocalizedString(),		// text
                         "http://www.keelhaul.me.uk",	// homePageAddress
                        "jjm@keelhaul.me.uk");		// bugsEmailAddress
    aboutData.addAuthor(ki18n("Jonathan Marten"),
                         KLocalizedString(),
                        "jjm@keelhaul.me.uk",
                        "http://www.keelhaul.me.uk");

    KCmdLineOptions opts;
    opts.add("f <file>", ki18n("Load a data file"));
    KCmdLineArgs::addCmdLineOptions(opts);

    KCmdLineArgs::init(argc,argv,&aboutData);
    KApplication app;
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

    MainWindow *w = new MainWindow(NULL);

    KUrl u = args->getOption("f");			// load a project file?
    if (u.isValid()) w->loadProject(u);

    w->show();
    return (app.exec());
}
