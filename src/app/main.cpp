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


//////////////////////////////////////////////////////////////////////////
//									//
//  Include files							//
//									//
//////////////////////////////////////////////////////////////////////////

#include <qapplication.h>
#include <qcommandlineparser.h>
#include <qdir.h>
#include <qurl.h>
#include <qdebug.h>

#include <kaboutdata.h>
#include <klocalizedstring.h>
#include <kcrash.h>

#include "mainwindow.h"
#include "filescontroller.h"

#include "marble/MarbleGlobal.h"

#include "vcsversion.h"
#ifdef HAVE_QCUSTOMPLOT
#include "qcustomplot.h"
#endif // HAVE_QCUSTOMPLOT

//////////////////////////////////////////////////////////////////////////
//									//
//  Main							 	//
//									//
//////////////////////////////////////////////////////////////////////////
 
int main(int argc,char *argv[])
{
    KLocalizedString::setApplicationDomain(PROJECT_NAME);

    KAboutData aboutData(PROJECT_NAME,			// componentName
                         i18n("Umbrail"),		// displayName
#if VCS_AVAILABLE
                         ( VERSION " (" VCS_TYPE " " VCS_REVISION ")" ),
#else
                         VERSION,			// version
#endif
                         i18n("GPS log viewer and editor"),
                         KAboutLicense::GPL_V3,
                         // The QString(...) is needed to avoid the "string literal as
                         // second argument to i18n()" build time error.
                         i18n("Copyright (c) 2014-%1 Jonathan Marten", QString(YEAR)),
                         "",				// otherText
                         "https://github.com/martenjj/umbrail",		// homePageAddress
                         "https://github.com/martenjj/umbrail/issues");	// bugsEmailAddress
    aboutData.addAuthor(i18n("Jonathan Marten"),
                        "",
                        "jjm@keelhaul.me.uk",
                        "http://www.keelhaul.me.uk");

    aboutData.addComponent(i18n("LibKFDialog"),
                           i18n("Dialogue utility library"),
                           "",
                           "https://github.com/martenjj/libkfdialog");
    aboutData.addComponent(i18n("Marble"),
                           i18n("Map display library"),
                           Marble::MARBLE_VERSION_STRING,
                           "https://marble.kde.org");
#ifdef HAVE_QCUSTOMPLOT
    aboutData.addComponent(i18n("QCustomPlot"),
                           i18n("Qt plotting and data visualization"),
                           QCUSTOMPLOT_VERSION_STR,
                           "https://www.qcustomplot.com");
#endif // HAVE_QCUSTOMPLOT

    QApplication app(argc, argv);
    KAboutData::setApplicationData(aboutData);
    KCrash::setDrKonqiEnabled(true);

    QCommandLineParser parser;
    parser.setApplicationDescription(aboutData.shortDescription());

    parser.addPositionalArgument("file", i18n("File to load"), i18n("[file...]"));
    parser.addOption(QCommandLineOption((QStringList() << "r" << "readonly"), i18n("Open files as read-only.")));

    aboutData.setupCommandLine(&parser);
    parser.process(app);
    aboutData.processCommandLine(&parser);

    MainWindow *w = nullptr;
    QStringList args = parser.positionalArguments();
    for (int i = 0; i<args.count(); ++i)		// load project or data files
    {
        // Parsing file arguments as URLs, as recommended at
        // http://marc.info/?l=kde-core-devel&m=141359279227385&w=2
        const QUrl u = QUrl::fromUserInput(args[i], QDir::currentPath(), QUrl::AssumeLocalFile);
        if (!u.isValid())
        {
            qWarning() << "Invalid URL" << u;
            continue;
        }

        w = new MainWindow(nullptr);
        const bool ok = w->loadProject(u, parser.isSet("readonly"));
        if (!ok) w->deleteLater();
        else w->show();
    }

    if (w==nullptr)					// no project or file loaded
    {
        w = new MainWindow(nullptr);
        w->filesController()->initNew();
        w->show();
    }

    return (app.exec());
}
