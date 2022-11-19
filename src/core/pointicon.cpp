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

#include "pointicon.h"

#include <qdebug.h>
#include <qstandardpaths.h>
#include <qhash.h>
#include <qimage.h>
#include <qbitmap.h>

#include <kiconloader.h>

#include "trackdata.h"

//////////////////////////////////////////////////////////////////////////
//									//
//  Colour key for provided colour - must agree with colour used in	//
//  master images.							//
//									//
//////////////////////////////////////////////////////////////////////////

#define COLOURKEY_FG		0xFF00FF		// magenta

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
    "Contact, Blonde",					// 222
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

//////////////////////////////////////////////////////////////////////////
//									//
//  Static caches for loaded master images				//
//									//
//////////////////////////////////////////////////////////////////////////

static QHash<int,QImage> sMasterImages;
static QHash<int,QImage> sGarminImages;

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
    if (!sMasterImages.contains(fileNo))			// master not found already
    {
        QString picFile = QString("icons/garmin/Waypoints_%1-%2.png")
                                  .arg(fileNo*garminNumPerFile+1, 3, 10, QLatin1Char('0'))
                                  .arg((fileNo+1)*garminNumPerFile, 3, 10, QLatin1Char('0'));
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
    const int fileNo = (idx-1)/garminNumPerFile;
    const int i = (idx-1) % garminNumPerFile;
    const int x = i % garminNumPerRow;
    const int y = i / garminNumPerRow;

    const QImage &img = masterGarminImage(fileNo);	// just as a reference
    if (img.isNull()) return;				// no image to use
							// assuming symbols are square
    const int pixPerSym = img.width()/garminNumPerRow;

    // Trim factors obtained by experiment
    QImage sym = img.copy(x*pixPerSym+6, y*pixPerSym+5, pixPerSym-12, pixPerSym-11);

    // TODO: does this do anything when stored in an icon?
    QBitmap mask = QBitmap::fromImage(sym.createHeuristicMask());
    QPixmap pix = QPixmap::fromImage(sym);
    pix.setMask(mask);

    //icon->addPixmap(QPixmap::fromImage(sym));
    icon->addPixmap(pix);
}

//////////////////////////////////////////////////////////////////////////
//									//
//  Constructors							//
//									//
//////////////////////////////////////////////////////////////////////////

PointIcon::PointIcon(const QString &name, PointIcon::IconNamespace nsp)
{
    qDebug() << "named" << name << "in nsp" << nsp;

    mName = name;
    mNsp = nsp;

    // First try: system and application icons
    if (nsp==PointIcon::NamespaceAuto || nsp==PointIcon::NamespaceSystem)
    {
        qDebug() << "try system";
        if (QIcon::hasThemeIcon(name))			// only if name known, so that
        {						// this will find an icon which
            mIcon = QIcon::fromTheme(name);		// should never be "unknown"
            qDebug() << "found in theme null?" << mIcon.isNull();
            if (!mIcon.isNull()) return;		// should always be true
        }
    }

    // Second try: Garmin icons
    if (nsp==PointIcon::NamespaceAuto || nsp==PointIcon::NamespaceGarmin)
    {
        qDebug() << "try Garmin";
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
            qDebug() << "found" << name << "at index" << idx;
            setGarminPixmap(&mIcon, idx);
            if (!mIcon.isNull()) return;		// always true unless load error
        }
    }

    mIcon = QIcon::fromTheme("unknown");		// last resort fallback
}


PointIcon::PointIcon(const QString &name, const QColor &col)
{
    qDebug() << "for colour" << col << "named" << name;

    mName = name;
    mNsp = PointIcon::NamespaceImage;

    // originally from WaypointImageProvider::icon()
    setIconPixmap(&mIcon, col, KIconLoader::SizeSmall);
    setIconPixmap(&mIcon, col, KIconLoader::SizeMedium);
}


QPixmap PointIcon::pixmap(int size) const
{
    // TODO: may be a relatively expensive operation, so cache result
    // Only ever called for map/profile with KIconLoader::SizeSmall
    QPixmap pix = mIcon.pixmap(size);
    QBitmap mask = pix.createHeuristicMask();
    pix.setMask(mask);
    return (pix);
}
