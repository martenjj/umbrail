
#include "waypointimageprovider.h"

#include <qdebug.h>
#include <qstandardpaths.h>
#include <qicon.h>
#include <qcache.h>
#include <qhash.h>
#include <qimage.h>

#include <kiconloader.h>

//////////////////////////////////////////////////////////////////////////
//									//
//  Debugging switches							//
//									//
//////////////////////////////////////////////////////////////////////////

#undef DEBUG_CACHE

//////////////////////////////////////////////////////////////////////////
//									//
//  Private								//
//									//
//////////////////////////////////////////////////////////////////////////

#define COLOURKEY_FG		0xFF00FF		// magenta


struct WaypointImageProviderPrivate
{
    QImage &masterImage(int size)
    {
        QImage img;
        if (!pMasterImages.contains(size))		// master not found already
        {
            QString picFile = "pics/waypoint-"+QString::number(size)+".png";
            QString imgFile = QStandardPaths::locate(QStandardPaths::AppDataLocation, picFile);
            if (!imgFile.isEmpty())			// look for master image file
            {
                QImage loadImg(imgFile);		// load master source image
                if (!loadImg.isNull()) img = loadImg;	// use the loaded image
                else qWarning() << "loading image failed" << imgFile;
            }
            else qWarning() << "cannot find image file" << picFile;

            qDebug() << "loaded" << imgFile << "size" << img.size();
            pMasterImages.insert(size, img);		// or null if a problem
        }

        Q_ASSERT(pMasterImages.contains(size));
        return (pMasterImages[size]);
    }


    void setIconPixmap(QIcon *icon, QColor col, int size)
    {
        QImage img = masterImage(size);
        if (img.isNull()) return;			// no image to use

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


    QHash<int,QImage> pMasterImages;
    QCache<QRgb,QIcon> pIconCache;
};

//////////////////////////////////////////////////////////////////////////
//									//
//  Constructor/destructor/instance					//
//									//
//////////////////////////////////////////////////////////////////////////

WaypointImageProvider::WaypointImageProvider()
    : d(new WaypointImageProviderPrivate)
{
    qDebug() << "cache size" << d->pIconCache.maxCost();
}


WaypointImageProvider::~WaypointImageProvider()
{
    delete d;
}


WaypointImageProvider *WaypointImageProvider::self()
{
    static WaypointImageProvider *instance = nullptr;
    if (instance==nullptr) instance = new WaypointImageProvider();
    return (instance);
}

//////////////////////////////////////////////////////////////////////////
//									//
//  Icon provider							//
//									//
//////////////////////////////////////////////////////////////////////////

QIcon WaypointImageProvider::icon(const QColor &col)
{
    QRgb key = col.rgb();

    if (d->pIconCache.contains(key))
    {
#ifdef DEBUG_CACHE
        qDebug() << "for" << col.name() << "found in cache";
#endif
        return (*d->pIconCache.object(key));
    }

#ifdef DEBUG_CACHE
    qDebug() << "for" << col.name() << "filling cache";
#endif
    QIcon *ic = new QIcon;
    d->setIconPixmap(ic, col, KIconLoader::SizeSmall);
    d->setIconPixmap(ic, col, KIconLoader::SizeMedium);
    d->pIconCache.insert(key, ic);
    return (*ic);
}
