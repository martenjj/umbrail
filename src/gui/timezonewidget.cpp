//////////////////////////////////////////////////////////////////////////
//									//
//  Project:	Umbrail - GPX track viewer and editor			//
//									//
//////////////////////////////////////////////////////////////////////////
//									//
//  Copyright (C) 2005, S.R.Haque <srhaque@iee.org>			//
//  Copyright (C) 2009, David Faure <faure@kde.org>			//
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

#include "timezonewidget.h"

#include <qdebug.h>
#include <qfile.h>
#include <qpixmap.h>
#include <qtimezone.h>
#include <qdatetime.h>
#include <qstandardpaths.h>

#include <klocalizedstring.h>


#undef DEBUG_ZONES


enum Columns
{
    CityColumn = 0,
    RegionColumn,
    CommentColumn
};

enum Roles
{
    ZoneRole = Qt::UserRole+1
};


static bool localeLessThan(const QString &a, const QString &b)
{
    return (QString::localeAwareCompare(a, b)<0);
}


TimeZoneWidget::TimeZoneWidget(QWidget *parent, const QList<QByteArray> &zones)
    : QTreeWidget(parent)
{
    mItemsCheckable = false;
    mSingleSelection = true;

    setRootIsDecorated(false);
    setHeaderLabels(QStringList() << i18nc("Define an area in the time zone, like a town area", "Area") << i18nc("Time zone", "Region") << i18n("Comment"));

    // Collect zones by localized city names, so that they can be sorted properly.
    QStringList cities;
    QHash<QString, QTimeZone> zonesByCity;

    // If the user did not provide a time zone list, we'll use the system default.
    QList<QByteArray> zoneIds = zones;
    if (zoneIds.isEmpty())
    {
        zoneIds = QTimeZone::availableTimeZoneIds();
        // add UTC to the default list
        QTimeZone utc = QTimeZone::utc();
        qDebug() << "UTC: dn" << i18n(utc.id().constData()) << "country" << utc.country();
        cities.append(i18n(utc.id().constData()));
        zonesByCity.insert(i18n(utc.id().constData()), utc);
    }

    foreach (const QByteArray &zoneId, zoneIds)
    {
        const QTimeZone zone(zoneId);
        const QString continentCity = zone.id();
#ifdef DEBUG_ZONES
        qDebug() << "zone:" << continentCity;
#endif
        const int separator = continentCity.lastIndexOf('/');
        // Make up the localized key that will be used for sorting.
        // Example: i18n(Asia/Tokyo) -> key = "i18n(Tokyo)|i18n(Asia)|Asia/Tokyo"
        // The zone name is appended to ensure unicity even with equal translations (#174918)
        const QString key = continentCity.mid(separator + 1) + '|'
                            + continentCity.left(separator) + '|' + zone.id();
        cities.append(key);
        zonesByCity.insert(key, zone);
    }
    std::sort(cities.begin(), cities.end(), localeLessThan);

    foreach (const QString &key, cities)
    {
        const QTimeZone zone = zonesByCity.value(key);
        const QByteArray tzName = zone.id();
#ifdef DEBUG_ZONES
        qDebug() << "city" << key << "zone" << tzName;
#endif
        QString comment = zone.comment();
        if (!comment.isEmpty()) comment = i18n(comment.toLocal8Bit().constData());

        // Convert:
        //
        //  "Europe/London", "GB" -> "London", "Europe/GB".
        //  "UTC",           ""   -> "UTC",    "".
        QStringList continentCity = i18n(tzName.constData()).split('/');

        QTreeWidgetItem *listItem = new QTreeWidgetItem(this);
        listItem->setText(CityColumn, continentCity.last());

        QString countryCode;
        // from http://stackoverflow.com/questions/24109270/getting-country-code-for-qlocalecountry
        QList<QLocale> locales = QLocale::matchingLocales(QLocale::AnyLanguage,
                                                          QLocale::AnyScript,
                                                          zone.country());
        if (!locales.isEmpty())
        {
            countryCode = locales.first().name();
            if (countryCode.contains('_')) countryCode = countryCode.section('_', -1).toLower();
        }
        else qWarning() << "no locales found for country" << zone.country();
#ifdef DEBUG_ZONES
        qDebug() << "  country code" << countryCode;
#endif
        QString countryName = QLocale::countryToString(zone.country());
#ifdef Q_OS_UNIX
        if (countryCode=="C") countryName = i18n("POSIX");
#endif
#ifdef DEBUG_ZONES
        qDebug() << "  country name" << countryName;
#endif
        if (countryName.isEmpty())
        {
            if (countryCode.isEmpty()) continentCity.removeLast();
            else continentCity.last() = countryCode.toUpper();
        }
        else
        {
            continentCity.removeLast();
            if (continentCity.isEmpty() || countryName!=continentCity.last()) continentCity.append(countryName);
        }
#ifdef DEBUG_ZONES
        qDebug() << "  cc" << continentCity.join('|');
#endif
        listItem->setText(RegionColumn, continentCity.join(QChar('/')));
        listItem->setText(CommentColumn, comment);
        listItem->setData(CityColumn, ZoneRole, tzName);	// store zone ID in custom role

        // Locate the flag from share/kf5/locale/countries/%1/flag.png
        QString flag = QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                                              QString("kf5/locale/countries/%1/flag.png").arg(countryCode));
        if (QFile::exists(flag)) listItem->setIcon(RegionColumn, QPixmap(flag));
    }
}


void TimeZoneWidget::setItemsCheckable(bool enable)
{
    mItemsCheckable = enable;
    const int count = topLevelItemCount();
    for (int row = 0; row < count; ++row)
    {
        QTreeWidgetItem *listItem = topLevelItem(row);
        listItem->setCheckState(CityColumn, Qt::Unchecked);
    }
    QTreeWidget::setSelectionMode(QAbstractItemView::NoSelection);
}


bool TimeZoneWidget::itemsCheckable() const
{
    return (mItemsCheckable);
}


QString TimeZoneWidget::displayName(const QTimeZone &zone)
{
    return (zone.displayName(QDateTime::currentDateTime()));
}


QStringList TimeZoneWidget::selection() const
{
    QStringList sel;

    // Loop through all entries.
    // Do not use selectedItems() because it skips hidden items, making it
    // impossible to use a KTreeWidgetSearchLine.
    // There is no QTreeWidgetItemConstIterator, hence the const_cast :/
    QTreeWidgetItemIterator it(const_cast<TimeZoneWidget *>(this), mItemsCheckable ? QTreeWidgetItemIterator::Checked : QTreeWidgetItemIterator::Selected);
    for (; *it; ++it) {
        sel.append((*it)->data(CityColumn, ZoneRole).toString());
    }

    return (sel);
}


void TimeZoneWidget::setSelected(const QByteArray &zone, bool selected)
{
    bool found = false;

    // The code was using findItems( zone, Qt::MatchExactly, ZoneColumn )
    // previously, but the underlying model only has 3 columns, the "hidden" column
    // wasn't available in there.

    if (!mItemsCheckable)
    {
        // Runtime compatibility for < 4.3 apps, which don't call the setMultiSelection reimplementation.
        mSingleSelection = (QTreeWidget::selectionMode() == QAbstractItemView::SingleSelection);
    }

    // Loop through all entries.
    const int rowCount = model()->rowCount(QModelIndex());
    for (int row = 0; row < rowCount; ++row)
    {
        const QModelIndex index = model()->index(row, CityColumn);
        const QByteArray tzName = index.data(ZoneRole).value<QByteArray>();
        if (tzName==zone)
        {
            if (mSingleSelection && selected) clearSelection();

            if (mItemsCheckable)
            {
                QTreeWidgetItem *listItem = itemFromIndex(index);
                listItem->setCheckState(CityColumn, selected ? Qt::Checked : Qt::Unchecked);
            }
            else
            {
                selectionModel()->select(index, selected ? (QItemSelectionModel::Select | QItemSelectionModel::Rows) : (QItemSelectionModel::Deselect | QItemSelectionModel::Rows));
            }

            scrollTo(index);				// ensure the selected item is visible
            found = true;

            if (mSingleSelection && selected) break;
        }
    }

    if (!found) qDebug() << "No such zone: " << zone;
}


void TimeZoneWidget::clearSelection()
{
    if (mItemsCheckable) {
        // Un-select all items
        const int rowCount = model()->rowCount(QModelIndex());
        for (int row = 0; row < rowCount; ++row) {
            const QModelIndex index = model()->index(row, 0);
            QTreeWidgetItem *listItem = itemFromIndex(index);
            listItem->setCheckState(CityColumn, Qt::Unchecked);
        }
    } else {
        QTreeWidget::clearSelection();
    }
}


void TimeZoneWidget::setSelectionMode(QAbstractItemView::SelectionMode mode)
{
    mSingleSelection = (mode == QAbstractItemView::SingleSelection);
    if (!mItemsCheckable) {
        QTreeWidget::setSelectionMode(mode);
    }
}


QAbstractItemView::SelectionMode TimeZoneWidget::selectionMode() const
{
    if (mItemsCheckable) {
        return mSingleSelection ? QTreeWidget::SingleSelection : QTreeWidget::MultiSelection;
    } else {
        return QTreeWidget::selectionMode();
    }
}

