//////////////////////////////////////////////////////////////////////////
//									//
//  Project:	Umbrail - GPX track viewer and editor			//
//									//
//////////////////////////////////////////////////////////////////////////
//									//
//  Copyright (c) 2014-2025 Jonathan Marten <jjm@keelhaul.me.uk>	//
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
#include <qcache.h>

#include <kiconloader.h>
#include <klocalizedstring.h>

#include "abstracticonprovider.h"
#include "garminiconprovider.h"
#include "osmandiconprovider.h"

//////////////////////////////////////////////////////////////////////////
//									//
//  Debugging switches							//
//									//
//////////////////////////////////////////////////////////////////////////

#undef DEBUG_ICONS
#undef DEBUG_CACHE

//////////////////////////////////////////////////////////////////////////
//									//
//  Icon provider registration						//
//									//
//////////////////////////////////////////////////////////////////////////

static QList<AbstractIconProvider *> sIconProviders;


/* static */ void PointIcon::initProviders()
{
    if (!sIconProviders.isEmpty()) return;

    // The order in which providers are registered here sets their priority
    // for name search and the GUI.  Although in our current workflow we are
    // likely to have more emphasis on OsmAnd, Garmin is placed here first
    // because a search of its smaller number of icons will be faster than
    // OsmAnd's thousands.
    sIconProviders.append(new GarminIconProvider);
    sIconProviders.append(new OsmandIconProvider);

    qDebug() << "have" << sIconProviders.count() << "icon providers";
}


/* static */ const QList<AbstractIconProvider *> *PointIcon::allProviders()
{
    return (&sIconProviders);
}

//////////////////////////////////////////////////////////////////////////
//									//
//  Colour key for provided colour - must agree with colour used in	//
//  master images.							//
//									//
//////////////////////////////////////////////////////////////////////////

#define COLOURKEY_FG		0xFF00FF		// magenta

//////////////////////////////////////////////////////////////////////////
//									//
//  Static cache for loaded master images				//
//									//
//////////////////////////////////////////////////////////////////////////

static QHash<int,QImage> sMasterImages;

//////////////////////////////////////////////////////////////////////////
//									//
//  Static cache for rendered icon images				//
//									//
//////////////////////////////////////////////////////////////////////////

typedef QCache<QString,PointIcon> PointIconCache;
Q_GLOBAL_STATIC(PointIconCache, sIconCache, 5000)

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
//  Constructors							//
//									//
//////////////////////////////////////////////////////////////////////////

/* protected */ PointIcon::PointIcon(const QString &name, PointIcon::IconNamespace nsp, const QVariant &colour, const QVariant &shape)
{
    mName = name;
#ifdef DEBUG_ICONS
    qDebug() << "named" << name << "in nsp" << nsp;
#endif // DEBUG_ICONS
    mNsp = PointIcon::NamespaceUnknown;

    // TODO: This is never called with NamespaceAuto when a system icon is
    // expected to be found.  So NamespaceAuto can be reinterpreted to mean
    // "any provider" and removed from this test.

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

    // Second try: enabled icon providers
    for (AbstractIconProvider *provider : std::as_const(sIconProviders))
    {
        if (nsp==PointIcon::NamespaceAuto && !provider->isEnabled()) continue;
        if (nsp==PointIcon::NamespaceAuto || nsp==provider->namespaceId())
        {
#ifdef DEBUG_ICONS
            qDebug() << "  trying provider" << provider->internalName();
#endif // DEBUG_ICONS
            if (provider->createIcon(&mIcon, name, colour, shape) && !mIcon.isNull())
            {
#ifdef DEBUG_ICONS
                qDebug() << "  found from provider";
#endif // DEBUG_ICONS
                mNsp = provider->namespaceId();
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


/* protected */ PointIcon::PointIcon(const QString &name, const QColor &col)
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
    for (AbstractIconProvider *provider : std::as_const(sIconProviders))
    {
        // There is no need to sort the result, IconSelector does that.
        if (provider->namespaceId()==nsp) return (provider->allIconNames());
    }

    qWarning() << "requested for unknown namespace" << nsp;
    return (QStringList());
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
case PointIcon::NamespaceAuto:		return (i18n("(error)"));
default:				/* fall through */;
    }

    for (const AbstractIconProvider *provider : std::as_const(sIconProviders))
    {
        if (provider->namespaceId()==nsp) return (provider->displayName());
    }
    return (i18n("(unknown)"));
}


/* static */ QByteArray PointIcon::namespaceInternalName(PointIcon::IconNamespace nsp)
{
    // Only for namespaces which are actual symbol sets.  The "system"
    // set is a sensible value here, although there is no GUI to
    // actually assign a system icon to a point.
    if (nsp==PointIcon::NamespaceSystem) return ("system");

    for (const AbstractIconProvider *provider : std::as_const(sIconProviders))
    {
        if (provider->namespaceId()==nsp) return (provider->internalName());
    }
    return ("");
}


/* static */ PointIcon::IconNamespace PointIcon::namespaceId(const QByteArray &nsn)
{
    if (nsn.isEmpty()) return (PointIcon::NamespaceAuto);
    if (nsn=="image") return (PointIcon::NamespaceColour);
    if (nsn=="system") return (PointIcon::NamespaceSystem);

    for (const AbstractIconProvider *provider : std::as_const(sIconProviders))
    {
        if (provider->internalName()==nsn) return (provider->namespaceId());
    }
    return (PointIcon::NamespaceAuto);
}


/* static */ QByteArray PointIcon::metadataKey(const QByteArray &nsn)
{
    if (nsn.isEmpty()) return ("");			// no namespace specified

    for (const AbstractIconProvider *provider : std::as_const(sIconProviders))
    {
        if (provider->internalName()==nsn) return (provider->metadataKey());
    }

    return ("");					// no namespace recognised
}

//////////////////////////////////////////////////////////////////////////
//									//
//  Cache statistics							//
//									//
//////////////////////////////////////////////////////////////////////////

void PointIcon::aboutToQuit()
{
    // Dump statistics.  Done in a separate function called when
    // the main window is closed, to ensure that they are shown
    // before the debug streams are closed.
    qDebug() << "cache used" << sIconCache->size() << "total cost" << sIconCache->totalCost();
}

//////////////////////////////////////////////////////////////////////////
//									//
//  Icon creation							//
//									//
//////////////////////////////////////////////////////////////////////////

/* static */ const PointIcon *PointIcon::create(const QString &name, PointIcon::IconNamespace nsp, const QVariant &colour, const QVariant &shape)
{
    QString cacheKey = name+'-'+QString::number(nsp);
    if (!colour.isNull()) cacheKey += '-'+colour.toString();
    if (!shape.isNull()) cacheKey += '-'+shape.toString();

    if (sIconCache->contains(cacheKey))
    {
#ifdef DEBUG_CACHE
        qDebug() << "found" << cacheKey << "in cache";
#endif
        return (sIconCache->object(cacheKey));
    }

    // Allocated here, deleted by cache when expired
    PointIcon *ic = new PointIcon(name, nsp, colour, shape);
#ifdef DEBUG_CACHE
    qDebug() << "saving" << cacheKey << "valid?" << ic->isValid() << "in cache";
#endif
    sIconCache->insert(cacheKey, ic, 2);		// named icon => lower cache cost
    return (ic);
}


/* static */ const PointIcon *PointIcon::create(const QColor &colour)
{
    const QString name = "colour-"+colour.name();	// name for this coloured icon

    if (sIconCache->contains(name))
    {
#ifdef DEBUG_CACHE
        qDebug() << "found" << name << "in cache";
#endif
        return (sIconCache->object(name));
    }

    PointIcon *ic = new PointIcon(name, colour);	// deleted by cache when expired
#ifdef DEBUG_CACHE
    qDebug() << "saving" << name << "valid?" << ic->isValid() << "in cache";
#endif
    sIconCache->insert(name, ic, 3);			// coloured item => higher cache cost
    return (ic);
}

//////////////////////////////////////////////////////////////////////////
//									//
//  Options for us or for a specified provider				//
//									//
//////////////////////////////////////////////////////////////////////////

/* static */ void PointIcon::setProviderOption(const QByteArray &nsn, const QString &key, const QString &value)
{
    qDebug() << "for provider" << nsn << "option" << key << "=" << value;

    if (nsn.isEmpty())					// an internal option,
    {							// there are none at present
    }
    else						// option for the named provider
    {
        for (AbstractIconProvider *provider : std::as_const(sIconProviders))
        {
            if (provider->internalName()==nsn)
            {
                if (key=="enabled") provider->setEnabled(static_cast<bool>(value.toInt()));
                else provider->setOption(key, value);
                break;
            }
        }
    }
}
