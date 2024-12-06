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

#include "osmandiconprovider.h"

#include <qstandardpaths.h>
#include <qbitmap.h>
#include <qpixmap.h>
#include <qfile.h>
#include <qdir.h>
#include <qpainter.h>
#include <qdebug.h>
#include <qelapsedtimer.h>

#include <klocalizedstring.h>
#include <kiconloader.h>

//////////////////////////////////////////////////////////////////////////
//									//
//  Debugging switches							//
//									//
//////////////////////////////////////////////////////////////////////////

#undef DEBUG_OSMAND

//////////////////////////////////////////////////////////////////////////
//									//
//  Static data								//
//									//
//////////////////////////////////////////////////////////////////////////

static bool sIsOsmandSetup = false;
static QHash<QString, QByteArray> sOsmandPaths;

//////////////////////////////////////////////////////////////////////////
//									//
//  Constants for icon generation					//
//									//
//////////////////////////////////////////////////////////////////////////

// TODO: config default and GUI setting for this path
static const char *OSMAND_RESBASE = "/ws/osmand/OsmAnd-resources/icons";
static const char *OSMAND_ALIASFILE = "tools/sortfiles.sh";
static const char *OSMAND_ICONSDIR = "svg";

static const int OSMAND_MAXSIZE = 64;			// maximum rendered pixmap size
static const int OSMAND_EXTRA = 6;			// extra size for border

//////////////////////////////////////////////////////////////////////////
//									//
//  findOsmandPaths - Locate all known OsmAnd Icons			//
//									//
//////////////////////////////////////////////////////////////////////////

static void findOsmandPaths()
{
    qDebug();
    sIsOsmandSetup = true;				// note now done (or failed) setup

    QElapsedTimer timer;
    timer.start();

    // TODO: if the parsed list has been saved from a previous run, then use it

    // TODO: maybe filter "seamark" names - we don't use them
    // and they account for about 1/4 of the total

    // The alias file which lists all known icon names and their file paths.
    QFile aliasFile(QString(OSMAND_RESBASE)+'/'+OSMAND_ALIASFILE);
    if (!aliasFile.exists())
    {
        qWarning() << "OsmAnd alias file" << aliasFile.fileName() << "does not exist";
        return;
    }
    if (!aliasFile.open(QIODevice::ReadOnly|QIODeviceBase::Text))
    {
        qWarning() << "Cannot read OsmAnd alias file" << aliasFile.fileName();
        return;
    }

    // The directory which contains the corresponding SVG images.
    QDir iconsDir(QString(OSMAND_RESBASE)+'/'+OSMAND_ICONSDIR);
    if (!iconsDir.exists())
    {
        qWarning() << "OsmAnd icons directory" << iconsDir.path() << "does not exist";
        return;
    }

    qDebug() << "Reading alias file" << aliasFile.fileName();
    qDebug() << "SVG icons directory" << iconsDir.path();

    // Read in the OsmAnd icon alias file and extract the icon names and
    // path aliases from it.

    int numRead = 0;					// total of lines read
    int numDefs = 0;					// number of definitions parsed
    int numFound = 0;					// number with SVG files found

    while (!aliasFile.atEnd())
    {
        ++numRead;					// count this line read
        const QByteArray line = aliasFile.readLine().simplified();
        if (line.isEmpty()) continue;			// simplify whitespace, ignore empty lines

        // Split the line up into space-separated fields.
        const QList<QByteArray> fields = line.split(' ');
        const QByteArray cmd = fields.first();
        QByteArray name;
        QByteArray alias;

        // Look at the first field, which indicates what sort of line this is.
        if (cmd=="icon_alias")
        {
            //            +--------------- GPX icon name
            //            |          +---- SVG path alias
            //            |          |
            //            v          v
            // icon_alias industrial landuse_industrial

            if (fields.count()<3) continue;
            name = fields[1];
            if (name.startsWith('$')) continue;		// in a shell function body
            alias = fields[2];
#ifdef DEBUG_OSMAND
            qDebug() << cmd << "--" << name << "->" << alias;
#endif // DEBUG_OSMAND
        }
        else if (cmd=="icon")
        {
            //      +---- GPX icon name, SVG path alias is the same
            //      |
            //      v
            // icon special_information

            if (fields.count()<2) continue;
            name = fields[1];
            if (name.startsWith('$')) continue;		// in a shell function body
            alias = name;
#ifdef DEBUG_OSMAND
            qDebug() << cmd << "--" << name;
#endif // DEBUG_OSMAND
        }
        else continue;					// ignore any other line
        ++numDefs;					// count this definition found

        // The icon alias gives the relative path to the SVG file, but not
        // in an obvious way.  The files are in a subdirectory which is the
        // first part of the pathname, but it is unpredictable whether the
        // separation happens at the first '_' or a subsequent one.  So
        // generate a potential pathname by replacing the first '_' with a
        // slash and see whether such a SVG file exists.  If it does not,
        // then undo that replacement and try again with the next '_',
        // repeating until a valid SVG file is found.  In practice the file
        // always seems to be found after either the first substitution or
        // the second, or not found at all.

        int prevIndex = -1;
        QByteArray tryPath;
        bool triedRewrite = false;

        while (true)
        {
            int idx = alias.indexOf('_', prevIndex+1);
            if (idx==-1)
            {
                // All of the '_'s in the alias path have been tried, but no
                // SVG file has been found.  If the alias has already been
                // rewritten once, then there is no more that can be done and
                // the SVG file cannot be found.
                if (triedRewrite)
                {
#ifdef DEBUG_OSMAND
                    qDebug() << "  not found, and already tried rewrite";
#endif // DEBUG_OSMAND
                    break;
                }

#ifdef DEBUG_OSMAND
                qDebug() << "  not found";
#endif // DEBUG_OSMAND

                // No SVG file has been found.  Try rewriting the alias using
                // these rules, which in their applicable cases gives the true
                // location of the SVG file.  If the alias is rewritten, set
                // the 'prevIndex' so that the check starts again immediately
                // at the expected location.
                if (alias.endsWith("_small") || alias.endsWith("_small_disused"))
                {
                    // ferry_terminal_small = map-small/ferry_terminal_small.svg
                    // railway_station_small_disused = map-small/railway_station_small_disused.svg
                    alias.prepend("map-small_");
                    prevIndex = 8;
                }
                else if (alias.startsWith("topo_topo_"))
                {
                    // topo_topo_alpine_hut = topo_accomodation/topo_alpine_hut.svg
                    alias = "topo_accomodation_"+alias.mid(5);
                    prevIndex = 16;
                }
                else if (alias.startsWith("seamark_int1_"))
                {
                    // seamark_int1_seamark_j132_weedkelp_shield_night = seamark_int1_shields/seamark_j132_weedkelp_shield_night.svg
                    alias = "seamark_int1_shields_"+alias.mid(13);
                    prevIndex = 19;
                }
                else
                {
                    qDebug() << "  no rewrite known";
                    break;
                }

                // If the alias has been rewritten, then note that so that if
                // the search fails again it will not be retried.
                triedRewrite = true;
#ifdef DEBUG_OSMAND
                qDebug() << "  rewritten to" << alias;
#endif // DEBUG_OSMAND
                continue;
            }

            // Insert a directory separator at the appropriate place in the
            // path, and add the file extsneion.
            tryPath = alias;
            tryPath[idx] = '/';
            tryPath += ".svg";
#ifdef DEBUG_OSMAND
            qDebug() << "  checking for" << iconsDir.absoluteFilePath(tryPath);
#endif // DEBUG_OSMAND

            if (iconsDir.exists(tryPath))		// relative to base directory
            {
#ifdef DEBUG_OSMAND
                qDebug() << "  found";
#endif // DEBUG_OSMAND

                // The SVG file has been found.  Note it as the icon path
                // for the name.
                sOsmandPaths[name] = tryPath;
                ++numFound;				// count this icon found
                break;
            }

            prevIndex = idx;				// the last split point tried
        }
    }

    qDebug() << "read" << numRead << "lines," << numDefs << "icon definitions," << numFound << "SVG icon files";

    // TODO: save the file for subsequent runs

    qDebug() << "listing took" << (timer.nsecsElapsed()/1000000) << "ms";
}

//////////////////////////////////////////////////////////////////////////
//									//
//  Constructor and plugin information					//
//									//
//////////////////////////////////////////////////////////////////////////

OsmandIconProvider::OsmandIconProvider()
    : AbstractIconProvider()
{
    qDebug() << "allocated nsp" << namespaceId();
}


QString OsmandIconProvider::displayName() const
{
    return (i18n("OsmAnd"));
}

//////////////////////////////////////////////////////////////////////////
//									//
//  Icon image generation						//
//									//
//////////////////////////////////////////////////////////////////////////

bool OsmandIconProvider::createIcon(QIcon *icon, const QString &name, const QVariant &colour, const QVariant &shape)
{
    if (!sIsOsmandSetup) findOsmandPaths();
    if (!sOsmandPaths.contains(name)) return (false);

    const QByteArray svgPath = sOsmandPaths.value(name);
#ifdef DEBUG_OSMAND
    qDebug() << "name" << name << "-> svg" << svgPath;
#endif // DEBUG_OSMAND
    if (svgPath.isEmpty()) return (false);		// should never happen

    QPixmap pix(QString(OSMAND_RESBASE)+'/'+OSMAND_ICONSDIR+'/'+svgPath);
    if (pix.isNull()) return (false);			// SVG image load failed

    if (qMax(pix.size().width(), pix.size().height())>OSMAND_MAXSIZE)
    {
        // Most OsmAnd POI icons render at 48x48, but some, in particular
        // seamarks and those more applicable to landuse, come out much
        // larger.  We don't need such big images in this application, so
        // scale them down to limit the maximium size.
#ifdef DEBUG_OSMAND
        qDebug() << "  limiting SVG size" << pix.size() << "to" << OSMAND_MAXSIZE;
#endif // DEBUG_OSMAND
        pix = pix.scaled(OSMAND_MAXSIZE, OSMAND_MAXSIZE, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }
    else if (pix.size().width()!=pix.size().height())
    {
        // If the rendered image is not square, then trim it to the minimum.
        // This simplifies the processing below which can assume a square image.
        const int minSize = qMin(pix.size().width(), pix.size().height());
#ifdef DEBUG_OSMAND
        qDebug() << "  squaring SVG size" << pix.size() << "to" << minSize;
#endif // DEBUG_OSMAND
        const int ew = pix.size().width()-minSize;	// extra space in width and height,
        const int eh = pix.size().height()-minSize;	// one of these must be zero
        pix = pix.copy(ew/2, eh/2, minSize, minSize);	// copy from original imake
    }
#ifdef DEBUG_OSMAND
    else qDebug() << "  rendered SVG size" << pix.size();
#endif // DEBUG_OSMAND

    const int ps = pix.width()+OSMAND_EXTRA;		// final grown pixmap size

    // Get the icon colour from the item metadata, if it is present.
    // OsmAnd tags this as COLOR, which is saved as the "pointcolor"
    // metadata when the GPX is imported.
    const QColor bgCol(!colour.isNull() ? colour.value<QColor>() : Qt::black);

    // Fill the image background with the colour, then render the SVG
    // image on top of it.
    QPixmap bgPix(ps, ps);
    bgPix.fill(bgCol);
    QPainter p1(&bgPix);
    p1.drawPixmap(QPoint(OSMAND_EXTRA/2, OSMAND_EXTRA/2), pix);
    p1.end();
    pix = bgPix;

    if (!shape.isNull() && shape!="square")		// has a background shape set, but
    {							// nothing is needed for a square
        QBitmap mask(ps, ps);
        mask.fill(Qt::color0);
        QPainter p2(&mask);
        p2.setBrush(Qt::color1);

        if (shape=="circle") p2.drawEllipse(QRect(0, 0, ps, ps));
        else if (shape=="octagon")
        {
            const int s = ps/2;				// half the overall size

            // This value is half of the octagon side length.  The true
            // side length should be ps/(1+sqrt(2)) where the divisor is
            // 2.414 to 3 significant figures.  However, the divisor is set
            // slightly lower at 24/11 (making it 2.182), to improve the
            // appearance by making the straight sides of the octagon longer.
            // Half the value is used as a premature optimisation.
            const int a = 11*ps/(24*2);
            QPoint pnts[8];

            pnts[0] = QPoint(s-a, 0);			// top
            pnts[1] = QPoint(s+a, 0);
            pnts[2] = QPoint(ps, s-a);			// right
            pnts[3] = QPoint(ps, s+a);
            pnts[4] = QPoint(s+a, ps);			// bottom
            pnts[5] = QPoint(s-a, ps);
            pnts[6] = QPoint(0, s+a);			// left
            pnts[7] = QPoint(0, s-a);
            p2.drawPolygon(pnts, 8);
        }
        else qDebug() << "Unknown OsmAnd background shape" << shape.toString();

        p2.end();
        pix.setMask(mask);
    }

    // Finally store the pixmap at its current size for the icon.
    icon->addPixmap(pix);

    // While we have the pixmap available, scale it to the sizes that
    // will be used by the application - 32x32 and 16x16 - and store
    // them for the icon also.
    icon->addPixmap(pix.scaled(KIconLoader::SizeMedium, KIconLoader::SizeMedium, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
    icon->addPixmap(pix.scaled(KIconLoader::SizeSmall, KIconLoader::SizeSmall, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));

    return (true);					// icon created
}

//////////////////////////////////////////////////////////////////////////
//									//
//  Icon names listing							//
//									//
//////////////////////////////////////////////////////////////////////////

QStringList OsmandIconProvider::allIconNames()
{
    if (!sIsOsmandSetup) findOsmandPaths();
    return (sOsmandPaths.keys());
}
