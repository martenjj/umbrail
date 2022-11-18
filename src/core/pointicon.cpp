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
//  Static data								//
//									//
//////////////////////////////////////////////////////////////////////////

static QHash<int,QImage> sMasterImages;

//////////////////////////////////////////////////////////////////////////
//									//
//  Finding the master image and generating a pixmap for the provided	//
//  colour from that.  Originally from 'WaypointImageProviderPrivate'	//
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
    QImage img = masterImage(size);
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
//  Constructors							//
//									//
//////////////////////////////////////////////////////////////////////////

PointIcon::PointIcon(const QString &name, PointIcon::IconNamespace nsp)
{
    qDebug() << "named" << name << "in nsp" << nsp;

    mName = name;
    mNsp = nsp;

    // TODO: search and take account of namespace
    mIcon = QIcon::fromTheme(name);
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
