//////////////////////////////////////////////////////////////////////////
//									//
//  Project:	Umbrail - GPX track viewer and editor			//
//									//
//////////////////////////////////////////////////////////////////////////
//									//
//  Copyright (C) 2005, S.R.Haque <srhaque@iee.org>			//
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

#ifndef TIMEZONEWIDGET_H
#define TIMEZONEWIDGET_H

#include <qtreewidget.h>

class QTimeZone;


/**
 * @brief A time zone selection widget.
 *
 * \b Detail:
 *
 * This class provides for selection of one or more time zones.
 *
 * \b Example:
 *
 * To use the class to implement a system timezone selection feature:
 * \code
 *
 *  // This adds a time zone widget to a dialog.
 *  m_timezones = new TimeZoneWidget(this);
 *  ...
 * \endcode
 *
 * To use the class to implement a multiple-choice custom time zone selector:
 * \code
 *
 *  m_timezones = new TimeZoneWidget( this, "Time zones", vcalendarTimezones );
 *  m_timezones->setSelectionMode( QTreeView::MultiSelection );
 *  ...
 * \endcode
 *
 * \image html ktimezonewidget.png "KDE Time Zone Widget"
 *
 * @author S.R.Haque <srhaque@iee.org>
 **/
class TimeZoneWidget : public QTreeWidget
{
    Q_OBJECT
    Q_PROPERTY(bool itemsCheckable READ itemsCheckable WRITE setItemsCheckable)
    Q_PROPERTY(QAbstractItemView::SelectionMode selectionMode READ selectionMode WRITE setSelectionMode)

public:
    /**
     * Constructs a time zone selection widget.
     *
     * @param parent The parent widget.
     * @param timeZones The time zone database to use. If 0, the system time zone
     *                  database is used.
     */
    explicit TimeZoneWidget(QWidget *parent = nullptr, const QList<QByteArray> &zones = QList<QByteArray>());

    /**
     * Destroys the time zone selection widget.
     */
    virtual ~TimeZoneWidget() = default;

    /**
     * Makes all items show a checkbox, so that the user can select multiple
     * timezones by means of checking checkboxes, rather than via multi-selection.
     *
     * In "items checkable" mode, the selection(), setSelected() and clearSelection()
     * methods work on the check states rather than on selecting/unselecting.
     *
     * @since 4.4
     */
    void setItemsCheckable(bool enable);

    /**
     * @return true if setItemsCheckable(true) was called.
     * @since 4.4
     */
    bool itemsCheckable() const;

    /**
     * Allows to select multiple timezones. This is the same as
     * setSelectionMode(TimeZoneWidget::MultiSelection) normally,
     * but in "items checkable" mode, this is rather about allowing to
     * check multiple items. In that case, the actual QTreeWidget selection
     * mode remains unchanged.
     * @since 4.4
     */
    void setSelectionMode(QAbstractItemView::SelectionMode mode);

    /**
     * @return the selection mode set by setSelectionMode().
     * @since 4.4
     */
    QAbstractItemView::SelectionMode selectionMode() const;

    /**
     * Returns the currently selected time zones. See QTreeView::selectionChanged().
     *
     * @return a list of time zone names, in the format used by the database
     *         supplied to the {@link TimeZoneWidget() } constructor.
     */
    QStringList selection() const;

    /**
     * Select/deselect the named time zone.
     *
     * @param zone The time zone name to be selected. Ignored if not recognized!
     * @param selected The new selection state.
     */
    void setSelected(const QByteArray &zone, bool selected);

    /**
     * Unselect all timezones.
     * This is the same as QTreeWidget::clearSelection, except in checkable items mode,
     * where items are all unchecked.
     * The overload is @since 4.4.
     */
    void clearSelection();

    /**
     * Format a time zone name in a standardised manner.
     *
     * @return formatted time zone name.
     */
    static QString displayName(const QTimeZone &zone);

private:
    bool mItemsCheckable;
    bool mSingleSelection;
};

#endif							// TIMEZONEWIDGET_H
