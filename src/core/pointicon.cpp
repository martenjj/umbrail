//////////////////////////////////////////////////////////////////////////
//									//
//  Project:	Umbrail - GPX track viewer and editor			//
//									//
//////////////////////////////////////////////////////////////////////////
//									//
//  Copyright (c) 2014-2022 Jonathan Marten <jjm@keelhaul.me.uk>	//
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

#include "pointicon.h"

#include <qdebug.h>
#include <qstandardpaths.h>
#include <qhash.h>
#include <qimage.h>
#include <qbitmap.h>
#include <qfile.h>
#include <qdir.h>
#include <qpainter.h>

#include <kiconloader.h>
#include <klocalizedstring.h>

#include "trackdata.h"

//////////////////////////////////////////////////////////////////////////
//									//
//  Debugging switches							//
//									//
//////////////////////////////////////////////////////////////////////////

#undef DEBUG_GARMIN
#undef DEBUG_ICONS
#define DEBUG_OSMAND

#ifdef DEBUG_GARMIN
#include <iostream>
#endif

//////////////////////////////////////////////////////////////////////////
//									//
//  Colour key for provided colour - must agree with colour used in	//
//  master images.							//
//									//
//////////////////////////////////////////////////////////////////////////

#define COLOURKEY_FG		0xFF00FF		// magenta

//////////////////////////////////////////////////////////////////////////
//									//
//  OsmAnd								//
//									//
//////////////////////////////////////////////////////////////////////////

static bool sIsOsmandSetup = false;
static QHash<QString, QByteArray> sOsmandPaths;

// TODO: config default and GUI setting for this path
static const char *OSMAND_RESBASE = "/ws/osmand/OsmAnd-resources/icons";
static const char *OSMAND_ALIASFILE = "tools/sortfiles.sh";
static const char *OSMAND_ICONSDIR = "svg";

static const int OSMAND_MAXSIZE = 64;			// maximum rendered pixmap size
static const int OSMAND_EXTRA = 6;			// extra size for border


static void findOsmandPaths()
{
    qDebug();
    sIsOsmandSetup = true;				// note now done (or failed) setup

    // TODO: if the parsed list has been saved from a previous run, then use it

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
}


static void setOsmandPixmap(QIcon *icon, const QString &name, const TrackDataItem *item = nullptr)
{
    const QByteArray svgPath = sOsmandPaths[name];
#ifdef DEBUG_OSMAND
    qDebug() << "name" << name << "-> svg" << svgPath;
#endif // DEBUG_OSMAND
    if (svgPath.isEmpty()) return;			// should never happen

    QPixmap pix(QString(OSMAND_RESBASE)+'/'+OSMAND_ICONSDIR+'/'+svgPath);
    if (pix.isNull()) return;				// SVG image load failed

    // Most OsmAnd POI icons render at 48x48, but some, in particular
    // seamarks and those more applicable to landuse, come out much
    // larger.  We don't need such big images in this application, so
    // scale them down to limit the maximium size.
    if (qMax(pix.size().width(), pix.size().height())>OSMAND_MAXSIZE)
    {
#ifdef DEBUG_OSMAND
        qDebug() << "  limiting SVG size" << pix.size() << "to" << OSMAND_MAXSIZE;
#endif // DEBUG_OSMAND
        pix = pix.scaled(OSMAND_MAXSIZE, OSMAND_MAXSIZE, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }
#ifdef DEBUG_OSMAND
    else qDebug() << "  rendered SVG size" << pix.size();
#endif // DEBUG_OSMAND

    const int pw = pix.width()+OSMAND_EXTRA;
    const int ph = pix.height()+OSMAND_EXTRA;

    // Get the icon colour from the item metadata, if it is present.
    // OsmAnd tags this as COLOR, which is saved as the "pointcolor"
    // metadata when the GPX is imported.
    QColor bgCol(Qt::black);
    if (item!=nullptr)
    {
        QColor c = item->metadata("pointcolor").value<QColor>();
        if (c.isValid()) bgCol = c;
    }

    // Fill the image background with the colour, then render the SVG
    // image on top of it.
    QPixmap bgPix(pw, ph);
    bgPix.fill(bgCol);
    QPainter p1(&bgPix);
    p1.drawPixmap(QPoint(OSMAND_EXTRA/2, OSMAND_EXTRA/2), pix);
    //p1.setPen(Qt::black);
    //p1.drawRect(0, 0, pw-1, ph-1);
    p1.end();
    pix = bgPix;

    // TODO: implement the shape from item metadata

    QBitmap mask(pw, ph);
    mask.fill(Qt::color0);
    QPainter p2(&mask);
    p2.setBrush(Qt::color1);
    p2.drawEllipse(QRect(0, 0, pw, ph));
    p2.end();

    // Finally set the mask and store the pixmap at its current
    // size for the icon.
    pix.setMask(mask);
    icon->addPixmap(pix);

    // While we have the pixmap available, scale it to the sizes that
    // will be used by the application - 32x32 and 16x16 - and store
    // them for the icon also.
    icon->addPixmap(pix.scaled(KIconLoader::SizeMedium, KIconLoader::SizeMedium, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
    icon->addPixmap(pix.scaled(KIconLoader::SizeSmall, KIconLoader::SizeSmall, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
}

//////////////////////////////////////////////////////////////////////////
//									//
//  Names and constants for Garmin icons				//
//									//
// from https://forums.geocaching.com/GC/index.php?/topic/		//
//                          277519-garmin-roadtrip-waypoint-symbols	//
//									//
//////////////////////////////////////////////////////////////////////////

static const char *garminNames[] =
{
    "Anchor",						// 001
    "Bell",						// 002
    "Diamond, Green",					// 003
    "Diamond, Red",					// 004
    "Diver Down Flag 1",				// 005
    "Diver Down Flag 2",				// 006
    "Bank",						// 007
    "Fishing Area",					// 008
    "Gas Station",					// 009
    "Horn",						// 010
    "Residence",					// 011
    "Restaurant",					// 012
    "Light",						// 013
    "Bar",						// 014
    "Skull and Crossbones",				// 015
    "Square, Green",					// 016
    "Square, Red",					// 017
    "Buoy, White",					// 018
    "Waypoint",						// 019
    "Shipwreck",					// 020
    "Man Overboard",					// 021
    "Navaid, Amber",					// 022
    "Navaid, Black",					// 023
    "Navaid, Blue",					// 024
    "Navaid, Green",					// 025
    "Navaid, Green/Red",				// 026
    "Navaid, Green/White",				// 027
    "Navaid, Orange",					// 028
    "Navaid, Red",					// 029
    "Navaid, Red/Green",				// 030
    "Navaid, Red/White",				// 031
    "Navaid, Violet",					// 032
    "Navaid, White",					// 033
    "Navaid, White/Green",				// 034
    "Navaid, White/Red",				// 035
    "Dot, White",					// 036
    "Radio Beacon",					// 037
    "Boat Ramp",					// 038
    "Campground",					// 039
    "Restroom",						// 040

    "Shower",						// 041
    "Drinking Water",					// 042
    "Telephone",					// 043
    "Medical Facility",					// 044
    "Information",					// 045
    "Parking Area",					// 046
    "Park",						// 047
    "Picnic Area",					// 048
    "Scenic Area",					// 049
    "Skiing Area",					// 050
    "Swimming Area",					// 051
    "Dam",						// 052
    "Controlled Area",					// 053
    "Danger Area",					// 054
    "Restricted Area",					// 055
    "Ball Park",					// 056
    "Car",						// 057
    "Hunting Area",					// 058
    "Shopping Center",					// 059
    "Lodging",						// 060
    "Mine",						// 061
    "Trail Head",					// 062
    "Truck Stop",					// 063
    "Exit",						// 064
    "Flag",						// 065
    "Circle with X",					// 066
    "Mile Marker",					// 067
    "TracBack Point",					// 068
    "Golf Course",					// 069
    "City (Small)",					// 070
    "City (Medium)",					// 071
    "City (Large)",					// 072
    "City (Capitol)",					// 073
    "Amusement Park",					// 074
    "Bowling",						// 075
    "Car Rental",					// 076
    "Car Repair",					// 077
    "Fast Food",					// 078
    "Fitness Center",					// 079
    "Movie Theater",					// 080

    "Museum",						// 081
    "Pharmacy",						// 082
    "Pizza",						// 083
    "Post Office",					// 084
    "RV Park",						// 085
    "School",						// 086
    "Stadium",						// 087
    "Department Store",					// 088
    "Zoo",						// 089
    "Convenience Store",				// 090
    "Live Theater",					// 091
    "Scales",						// 092
    "Toll Booth",					// 093
    "Bridge",						// 094
    "Building",						// 095
    "Cemetery",						// 096
    "Church",						// 097
    "Civil",						// 098
    "Crossing",						// 099
    "Ghost Town",					// 100
    "Levee",						// 101
    "Military",						// 102
    "Oil Field",					// 103
    "Tunnel",						// 104
    "Beach",						// 105
    "Forest",						// 106
    "Summit",						// 107
    "Airport",						// 108
    "Heliport",						// 109
    "Private Field",					// 110
    "Soft Field",					// 111
    "Tall Tower",					// 112
    "Short Tower",					// 113
    "Glider Area",					// 114
    "Ultralight Area",					// 115
    "Parachute Area",					// 116
    "Seaplane Base",					// 117
    "Geocache",						// 118
    "Geocache Found",					// 119
    "Contact, Afro",					// 120

    "Contact, Alien",					// 121
    "Contact, Ball Cap",				// 122
    "Contact, Big Ears",				// 123
    "Contact, Biker",					// 124
    "Contact, Bug",					// 125
    "Contact, Cat",					// 126
    "Contact, Dog",					// 127
    "Contact, Dreadlocks",				// 128
    "Contact, Female1",					// 129
    "Contact, Female2",					// 130
    "Contact, Female3",					// 131
    "Contact, Goatee",					// 132
    "Contact, Kung-Fu",					// 133
    "Contact, Pig",					// 134
    "Contact, Pirate",					// 135
    "Contact, Ranger",					// 136
    "Contact, Smiley",					// 137
    "Contact, Spike",					// 138
    "Contact, Sumo",					// 139
    "Water Hydrant",					// 140
    "Flag, Red",					// 141
    "Flag, Blue",					// 142
    "Flag, Green",					// 143
    "Pin, Red",						// 144
    "Pin, Blue",					// 145
    "Pin, Green",					// 146
    "Block, Red",					// 147
    "Block, Blue",					// 148
    "Block, Green",					// 149
    "Bike Trail",					// 150
    "Fishing Hot Spot Facility",			// 151
    "Police Station",					// 152
    "Ski Resort",					// 153
    "Ice Skating",					// 154
    "Wrecker",						// 155
    "Anchor Prohibited",				// 156
    "Beacon",						// 157
    "Coast Guard",					// 158
    "Reef",						// 159
    "Weed Bed",						// 160

    "Dropoff",						// 161
    "Dock",						// 162
    "Marina",						// 163
    "Bait and Tackle",					// 164
    "Stump",						// 165
    "Circle, Red",					// 166
    "Circle, Green",					// 167
    "Circle, Blue",					// 168
    "Diamond, Blue",					// 169
    "Oval, Red",					// 170
    "Oval, Green",					// 171
    "Oval, Blue",					// 172
    "Rectangle, Red",					// 173
    "Rectangle, Green",					// 174
    "Rectangle, Blue",					// 175
    "Square, Blue",					// 176
    "Letter A, Red",					// 177
    "Letter A, Green",					// 178
    "Letter A, Blue",					// 179
    "Letter B, Red",					// 180
    "Letter B, Green",					// 181
    "Letter B, Blue",					// 182
    "Letter C, Red",					// 183
    "Letter C, Green",					// 184
    "Letter C, Blue",					// 185
    "Letter D, Red",					// 186
    "Letter D, Green",					// 187
    "Letter D, Blue",					// 188
    "Number 0, Red",					// 189
    "Number 0, Green",					// 190
    "Number 0, Blue",					// 191
    "Number 1, Red",					// 192
    "Number 1, Green",					// 193
    "Number 1, Blue",					// 194
    "Number 2, Red",					// 195
    "Number 2, Green",					// 196
    "Number 2, Blue",					// 197
    "Number 3, Red",					// 198
    "Number 3, Green",					// 199
    "Number 3, Blue",					// 200

    "Number 4, Red",					// 201
    "Number 4, Green",					// 202
    "Number 4, Blue",					// 203
    "Number 5, Red",					// 204
    "Number 5, Green",					// 205
    "Number 5, Blue",					// 206
    "Number 6, Red",					// 207
    "Number 6, Green",					// 208
    "Number 6, Blue",					// 209
    "Number 7, Red",					// 210
    "Number 7, Green",					// 211
    "Number 7, Blue",					// 212
    "Number 8, Red",					// 213
    "Number 8, Green",					// 214
    "Number 8, Blue",					// 215
    "Number 9, Red",					// 216
    "Number 9, Green",					// 217
    "Number 9, Blue",					// 218
    "Triangle, Blue",					// 219
    "Triangle, Green",					// 220
    "Triangle, Red",					// 221
    // Named as "Contact, Blond" in the forum post,
    // but the GPS device writes this value instead.
    "Contact, BlondWoman",				// 222
    "Contact, Clown",					// 223
    "Contact, Glasses",					// 224
    "Contact, Panda",					// 225
    "Multi-Cache",					// 226
    "Letterbox Cache",					// 227
    "Puzzle Cache",					// 228
    "Library",						// 229
    "Ground Transportation",				// 230
    "City Hall",					// 231
    "Winery",						// 232
    "ATV",						// 233
    "Big Game",						// 234
    "Blind",						// 235
    "Blood Trail",					// 236
    "Cover",						// 237
    "Covey",						// 238
    "Food Source",					// 239
    "Furbearer",					// 240

    "Lodge",						// 241
    "Small Game",					// 242
    "Animal Tracks",					// 243
    "Treed Quarry",					// 244
    "Tree Stand",					// 245
    "Truck",						// 246
    "Upland Game",					// 247
    "Waterfowl"						// 248
};

static const int numGarminNames = sizeof(garminNames)/sizeof(char *);

static const int garminNumPerFile = 40;			// images per file
static const int garminNumPerRow = 8;			// images per row in file

static const int garminBaseX = 6;			// offset to first image
static const int garminBaseY = 5;

static const int garminStepX = 35;			// offset between columns/rows
static const int garminStepY = 35;

static const int garminSizeX = 24;			// size of actual image
static const int garminSizeY = 24;

//////////////////////////////////////////////////////////////////////////
//									//
//  Static cache for loaded master images				//
//									//
//////////////////////////////////////////////////////////////////////////

static QHash<int,QImage> sMasterImages;

//////////////////////////////////////////////////////////////////////////
//									//
//  Finding the master "star" image and generating a pixmap for the	//
//  specified colour from that.						//
//									//
//  Originally from 'WaypointImageProviderPrivate'			//
//									//
//////////////////////////////////////////////////////////////////////////

static QImage &masterImage(int size)
{
    QImage img;
    if (!sMasterImages.contains(size))			// master not found already
    {
        QString picFile = "pics/waypoint-"+QString::number(size)+".png";
        QString imgFile = QStandardPaths::locate(QStandardPaths::AppDataLocation, picFile);
        if (!imgFile.isEmpty())				// look for master image file
        {
            QImage loadImg(imgFile);			// load master source image
            if (!loadImg.isNull()) img = loadImg;	// use the loaded image
            else qWarning() << "loading image failed" << imgFile;
        }
        else qWarning() << "cannot find image file" << picFile;

        qDebug() << "loaded" << imgFile << "size" << img.size();
        sMasterImages.insert(size, img);		// or null if a problem
    }

    Q_ASSERT(sMasterImages.contains(size));
    return (sMasterImages[size]);
}


static void setIconPixmap(QIcon *icon, const QColor &col, int size)
{
    QImage img = masterImage(size);			// want a deep copy
    if (img.isNull()) return;				// no image to use

    for (int x = 0; x<img.width(); ++x)
    {
        for (int y = 0; y<img.height(); ++y)
        {
            QRgb pix = img.pixel(x, y);
            int pval = pix & 0x00FFFFFF;
            if (pval==COLOURKEY_FG) img.setPixel(x, y, col.rgb());
        }
    }

    icon->addPixmap(QPixmap::fromImage(img));
}

//////////////////////////////////////////////////////////////////////////
//									//
//  Finding the master Garmin image and generating a pixmap for the	//
//  specified symbol from that.						//
//									//
//////////////////////////////////////////////////////////////////////////

static QImage &masterGarminImage(int fileNo)
{
    QImage img;
    if (!sMasterImages.contains(fileNo))		// master not found already
    {
        const int i1 = fileNo*garminNumPerFile+1;	// first icon in image set
        int i2 = (fileNo+1)*garminNumPerFile;		// last icon in image set
        if (i2==280) i2 = 248;				// last file is smaller

        QString picFile = QString("icons/garmin/Waypoints_%1-%2.png")
                                  .arg(i1, 3, 10, QLatin1Char('0'))
                                  .arg(i2, 3, 10, QLatin1Char('0'));
        QString imgFile = QStandardPaths::locate(QStandardPaths::AppDataLocation, picFile);
        if (!imgFile.isEmpty())				// look for master image file
        {
            QImage loadImg(imgFile);			// load master source image
            if (!loadImg.isNull()) img = loadImg;	// use the loaded image
            else qWarning() << "loading image failed" << imgFile;
        }
        else qWarning() << "cannot find image file" << picFile;

        qDebug() << "loaded" << imgFile << "size" << img.size() << "fmt" << img.format();
        sMasterImages.insert(fileNo, img);		// or null if a problem
    }

    Q_ASSERT(sMasterImages.contains(fileNo));
    return (sMasterImages[fileNo]);
}


static void setGarminPixmap(QIcon *icon, int idx)
{
    const int fileNo = (idx-1)/garminNumPerFile;	// image file number
    const int i = (idx-1) % garminNumPerFile;		// index within that file
    const int xi = i % garminNumPerRow;			// position within that file
    const int yi = i / garminNumPerRow;

    const QImage &img = masterGarminImage(fileNo);	// just as a reference
    if (img.isNull()) return;				// no image to use

    // Copy the required symbol out of the combined source image.
    QImage sym = img.copy(garminBaseX+xi*garminStepX,
                          garminBaseY+yi*garminStepY,
                          garminSizeX, garminSizeY);

    const int xs = sym.width()-1;
    const int ys = sym.height()-1;

#ifdef DEBUG_GARMIN
    qDebug() << "extracted symbol size" << sym.size();
    for (int y = 0; y<=ys; ++y)
    {
        QString l;
        for (int x = 0; x<=ys; ++x)
        {
            const QColor col = sym.pixelColor(x, y);
            l += " "+QString("%1").arg(col.rgb() & 0x00FFFFFF, 6, 16, QLatin1Char('0'));
        }
        std::cout << qPrintable(l) << std::endl;
    }
#endif // DEBUG_GARMIN

    // Unfortunately the source images are not clean enough to do
    // a QImage::createHeuristicMask() directly on them - the
    // background is not solid and Qt applies no pixel value
    // tolerance for that operation.  So examine each pixel of
    // the symbol image in turn, and if it appears to be a light
    // enough grey (but not completely white, because that is more
    // likely to be a border around the symbol) then set the
    // corresponding pixel in a separate 'greyMask' image.  A
    // black/white output will be sufficient for this, in which
    // case it may appear wasteful to create an RGB32 image but
    // createHeuristicMask() converts the image to this format
    // anyway if it is not so already.
    //
    // Although advised in the Qt API documentation, it is not
    // necessary to clear or fill the image because every pixel
    // of it will be set.
    QImage greyMask(sym.size(), QImage::Format_RGB32);

#ifdef DEBUG_GARMIN
    qDebug() << "grey mask";
#endif // DEBUG_GARMIN
    for (int y = 0; y<=ys; ++y)
    {
        QString l;
        for (int x = 0; x<=xs; ++x)
        {
            const QColor col = sym.pixelColor(x, y);
            const QRgb rgb = col.rgb() & 0x00FFFFFF;

            bool isg = (rgb!=0xFFFFFF) &&		// not pure white
                       (rgb>0xE00000) &&		// grey bright enough
                       (col.red()==col.green()) &&	// red same as green
                       (col.red()==col.blue());		// and also same as blue

            // Ensure that the corners of the image count as mask
            // pixels, because createHeuristicMask() starts to
            // detect from there.
            if ((x==0 && y==0) || (x==xs && y==ys)) isg = true;

#ifdef DEBUG_GARMIN
            l += " "+QString("%1").arg(isg, 1, 16, QLatin1Char('0'));
#endif // DEBUG_GARMIN
            greyMask.setPixel(x, y, (isg ? Qt::black : Qt::white));
        }
#ifdef DEBUG_GARMIN
        std::cout << qPrintable(l) << std::endl;
#endif // DEBUG_GARMIN
    }

    // Now the black/white image generated above is clean enough to
    // generate a mask.  There is no point converting the image to
    // a QPixmap first, as QPixmap::createHeuristicMask() immediately
    // converts the QPixmap back to a QImage.
    QBitmap mask = QBitmap::fromImage(greyMask.createHeuristicMask());
#ifdef DEBUG_GARMIN
    const QImage im = mask.toImage();
    qDebug() << "heuristic mask format" << im.format();
    for (int y = 0; y<=ys; ++y)
    {
        QString l;
        for (int x = 0; x<=xs; ++x)
        {
            l += " "+QString("%1").arg(im.pixelColor(x, y).value()>0x80, 1, 16, QLatin1Char('0'));
        }
        std::cout << qPrintable(l) << std::endl;
    }
#endif // DEBUG_GARMIN

    // Finally generate a QPixmap from the scaled image, add the mask
    // and store it at that size for the icon.
    QPixmap pix = QPixmap::fromImage(sym);
    pix.setMask(mask);
    icon->addPixmap(pix);

    // While we have the masked pixmap available, scale it to the sizes
    // that will be used by the application - 32x32 and 16x16 - and store
    // them for the icon also.
    icon->addPixmap(pix.scaled(KIconLoader::SizeMedium, KIconLoader::SizeMedium, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
    icon->addPixmap(pix.scaled(KIconLoader::SizeSmall, KIconLoader::SizeSmall, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
}

//////////////////////////////////////////////////////////////////////////
//									//
//  Constructors							//
//									//
//////////////////////////////////////////////////////////////////////////

PointIcon::PointIcon(const QString &name, PointIcon::IconNamespace nsp, const TrackDataItem *item)
{
    mName = name;
#ifdef DEBUG_ICONS
    qDebug() << "named" << name << "in nsp" << nsp;
#endif // DEBUG_ICONS
    mNsp = PointIcon::NamespaceUnknown;

    // First try: system and application icons
    if (nsp==PointIcon::NamespaceAuto || nsp==PointIcon::NamespaceSystem)
    {
#ifdef DEBUG_ICONS
        qDebug() << "  trying system";
#endif // DEBUG_ICONS
        if (QIcon::hasThemeIcon(name))			// only if name known, so that
        {						// this will find an icon which
            mIcon = QIcon::fromTheme(name);		// should never be "unknown"
#ifdef DEBUG_ICONS
            qDebug() << "  found in theme null?" << mIcon.isNull();
#endif // DEBUG_ICONS
            if (!mIcon.isNull())			// should always be true
            {						// because of hasThemeIcon() above
                mNsp = PointIcon::NamespaceSystem;
                return;
            }
        }
    }

    // Second try: Garmin icons
    if (nsp==PointIcon::NamespaceAuto || nsp==PointIcon::NamespaceGarmin)
    {
#ifdef DEBUG_ICONS
        qDebug() << "  trying Garmin";
#endif // DEBUG_ICONS
        const QByteArray cData = name.toLocal8Bit();	// do not combine these, can't
        const char *cName = cData.constData();		// keep pointer into temporary!

        int idx = -1;
        for (int i = 0; i<numGarminNames; ++i)		// search the Garmin name list
        {
            if (strcmp(garminNames[i], cName)==0)
            {
                idx = i+1;				// record the index (1-based)
                break;
            }
        }

        if (idx>0)					// name was found
        {
#ifdef DEBUG_ICONS
            qDebug() << "  found at index" << idx;
#endif // DEBUG_ICONS
            setGarminPixmap(&mIcon, idx);
            if (!mIcon.isNull())			// always true unless load error
            {
                mNsp = PointIcon::NamespaceGarmin;
                return;
            }
        }
    }

    // Third try: OsmAnd icons
    if (nsp==PointIcon::NamespaceAuto || nsp==PointIcon::NamespaceOsmand)
    {
#ifdef DEBUG_ICONS
        qDebug() << "  trying OsmAnd";
#endif // DEBUG_ICONS

        if (!sIsOsmandSetup) findOsmandPaths();
        if (sOsmandPaths.contains(name))
        {
            setOsmandPixmap(&mIcon, name, item);
            if (!mIcon.isNull())			// always true unless load error
            {
                mNsp = PointIcon::NamespaceOsmand;
                return;
            }
        }
    }

#ifdef DEBUG_ICONS
    qDebug() << "  name not found";
#endif // DEBUG_ICONS

    mIcon = QIcon::fromTheme("unknown");		// last resort fallback
    if (mNsp==PointIcon::NamespaceUnknown && nsp!=PointIcon::NamespaceAuto) mNsp = nsp;
}


PointIcon::PointIcon(const QString &name, const QColor &col)
{
    mName = name;
    mNsp = PointIcon::NamespaceColour;
#ifdef DEBUG_ICONS
    qDebug() << "for colour" << col << "named" << name;
#endif // DEBUG_ICONS

    // originally from WaypointImageProvider::icon()
    setIconPixmap(&mIcon, col, KIconLoader::SizeSmall);
    setIconPixmap(&mIcon, col, KIconLoader::SizeMedium);
}

//////////////////////////////////////////////////////////////////////////
//									//
//  Listing icon names							//
//									//
//////////////////////////////////////////////////////////////////////////

/* static */ QStringList PointIcon::allNames(PointIcon::IconNamespace nsp)
{
    QStringList result;

    if (nsp==PointIcon::NamespaceGarmin)
    {
        for (int i = 0; i<numGarminNames; ++i) result.append(garminNames[i]);
    }
    else if (nsp==PointIcon::NamespaceOsmand)
    {
        if (!sIsOsmandSetup) findOsmandPaths();
        // TODO: maybe filter "seamark" names - we don't use them
        // and they account for about 1/4 of the total
        result = sOsmandPaths.keys();
    }
    else qWarning() << "requested for invalid namespace" << nsp;

    // There is no need to sort the result, IconSelector does that.
    return (result);
}

//////////////////////////////////////////////////////////////////////////
//									//
//  Namespaces and display strings					//
//									//
//////////////////////////////////////////////////////////////////////////

/* static */ QString PointIcon::namespaceDisplayName(PointIcon::IconNamespace nsp)
{
    switch (nsp)
    {
case PointIcon::NamespaceColour:	return (i18n("Image"));
case PointIcon::NamespaceSystem:	return (i18n("System"));
case PointIcon::NamespaceGarmin:	return (i18n("Garmin"));
case PointIcon::NamespaceOsmand:	return (i18n("OsmAnd"));
case PointIcon::NamespaceAuto:		return (i18n("(error)"));
default:				return (i18n("(unknown)"));
    }
}


/* static */ QByteArray PointIcon::namespaceInternalName(PointIcon::IconNamespace nsp)
{
    // Only for namespaces which are actual symbol sets.  The "system"
    // set is a sensible value here, although there is no GUI to
    // actually assign a system icon to a point.
    switch (nsp)
    {
case PointIcon::NamespaceGarmin:	return ("garmin");
case PointIcon::NamespaceOsmand:	return ("osmand");
case PointIcon::NamespaceSystem:	return ("system");
default:				return ("");
    }
}


/* static */ PointIcon::IconNamespace PointIcon::namespaceId(const QByteArray &nsn)
{
    if (nsn=="image") return (PointIcon::NamespaceColour);
    if (nsn=="system") return (PointIcon::NamespaceSystem);
    if (nsn=="garmin") return (PointIcon::NamespaceGarmin);
    if (nsn=="osmand") return (PointIcon::NamespaceOsmand);
    return (PointIcon::NamespaceAuto);
}
