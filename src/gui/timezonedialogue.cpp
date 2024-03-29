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

#include "timezonedialogue.h"

#include <qgridlayout.h>
#include <qlabel.h>
#include <qheaderview.h>
#include <qpushbutton.h>
#include <qdebug.h>
#include <qtimezone.h>

#include <klocalizedstring.h>
#include <kconfiggroup.h>
#include <ktreewidgetsearchline.h>

#include "timezonewidget.h"


TimeZoneDialogue::TimeZoneDialogue(QWidget *pnt)
    : DialogBase(pnt),
      DialogStateSaver(this)
{
    setObjectName("TimeZoneDialogue");

    setModal(true);
    setButtons(QDialogButtonBox::Ok|QDialogButtonBox::Cancel|QDialogButtonBox::Reset|QDialogButtonBox::RestoreDefaults);
    buttonBox()->button(QDialogButtonBox::Ok)->setDefault(true);
    setButtonText(QDialogButtonBox::Ok, i18n("Select"));
    setButtonText(QDialogButtonBox::Reset, i18nc("@action:button", "Reset to UTC"));
    setButtonText(QDialogButtonBox::RestoreDefaults, i18nc("@action:button", "System Time Zone"));
    setButtonIcon(QDialogButtonBox::RestoreDefaults, buttonBox()->button(QDialogButtonBox::Reset)->icon());
    setWindowTitle(i18n("Select Time Zone"));

    connect(buttonBox()->button(QDialogButtonBox::Reset), &QPushButton::clicked, this, &TimeZoneDialogue::slotUseUTC);
    connect(buttonBox()->button(QDialogButtonBox::RestoreDefaults), &QPushButton::clicked, this, &TimeZoneDialogue::slotUseSystem);

    QWidget *w = new QWidget(this);
    setMainWidget(w);
    QGridLayout *gl = new QGridLayout(w);

    // Filter the available time zones so that the list does not need too
    // much scrolling.  Accept only those zones which start with "Europe/"
    // or "UTC", and whose time offset is within 3 hours from UTC.
    // TODO: make this configurable, offset from current system time zone
    const QList<QByteArray> allZoneIds = QTimeZone::availableTimeZoneIds();
    QList<QByteArray> zoneIds;
    for (const QByteArray &zone : allZoneIds)
    {
        if (zone.startsWith("Europe/") || zone.startsWith("UTC"))
        {
            QTimeZone tz(zone);
            if (qAbs(tz.offsetFromUtc(QDateTime::currentDateTime()))<=(3*3600))
            {
                zoneIds.append(zone);
            }
        }
    }
    //qDebug() << zoneIds;

    mTimeZoneWidget = new TimeZoneWidget(this, zoneIds);
    mTimeZoneWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    connect(mTimeZoneWidget, &QTreeWidget::itemSelectionChanged, this, &TimeZoneDialogue::slotTimeZoneChanged);
    gl->addWidget(mTimeZoneWidget, 1, 0, 1, -1);

    KTreeWidgetSearchLine *sl = new KTreeWidgetSearchLine(this, mTimeZoneWidget);
    sl->setCaseSensitivity(Qt::CaseInsensitive);
    gl->addWidget(sl, 0, 1);

    QLabel *l = new QLabel(i18nc("@label:textbox", "Search:"), this);
    l->setBuddy(sl);
    gl->addWidget(l, 0, 0);

    setMinimumSize(400,320);
    setStateSaver(this);

    mReturnUTC = false;
    slotTimeZoneChanged();
    sl->setFocus(Qt::ActiveWindowFocusReason);
}


void TimeZoneDialogue::setTimeZone(const QByteArray &zone)
{
    qDebug() << zone;
    mTimeZoneWidget->setSelected(zone, true);
}


QString TimeZoneDialogue::timeZone() const
{
    if (mReturnUTC) return (QString());
    QStringList sel = mTimeZoneWidget->selection();
    return (sel.isEmpty() ? QString() : sel.first());
}


void TimeZoneDialogue::slotUseUTC()
{
    mReturnUTC = true;
    accept();
}


void TimeZoneDialogue::slotUseSystem()
{
    mTimeZoneWidget->setSelected(QTimeZone::systemTimeZone().id(), true);
    accept();
}


void TimeZoneDialogue::slotTimeZoneChanged()
{
    setButtonEnabled(QDialogButtonBox::Ok, !mTimeZoneWidget->selectedItems().isEmpty());
}


void TimeZoneDialogue::saveConfig(QDialog *dialog, KConfigGroup &grp) const
{
    grp.writeEntry("State", mTimeZoneWidget->header()->saveState().toHex());
    DialogStateSaver::saveConfig(dialog, grp);
}


void TimeZoneDialogue::restoreConfig(QDialog *dialog, const KConfigGroup &grp)
{
    QString colStates = grp.readEntry("State");
    if (!colStates.isEmpty()) mTimeZoneWidget->header()->restoreState(QByteArray::fromHex(colStates.toLocal8Bit()));
    DialogStateSaver::restoreConfig(dialog, grp);
}
