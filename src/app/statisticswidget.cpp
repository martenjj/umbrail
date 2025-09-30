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

#include "statisticswidget.h"

#include <qgridlayout.h>
#include <qlabel.h>
#include <qprogressbar.h>
#include <qpushbutton.h>

#include <klocalizedstring.h>
#ifdef USE_KCOLORSCHEME
#include <kcolorscheme.h>
#endif

#include <kfdialog/dialogstatewatcher.h>

#include "mainwindow.h"
#include "filescontroller.h"
#include "filesview.h"
#include "trackdata.h"
#include "itemcounter.h"


StatisticsWidget::StatisticsWidget(QWidget *pnt)
    : DialogBase(pnt),
      ApplicationDataInterface(pnt)
{
    setObjectName("StatisticsWidget");
    setButtons(QDialogButtonBox::Close);

    QList<TrackDataItem *> items = filesController()->filesView()->selectedItems();
    const ItemCounter counter(&items, ItemCounter::RecurseAll|ItemCounter::WaypointStatus|ItemCounter::PointDetail);
    mTotalPoints = counter.count(ItemCounter::Trackpoint)+counter.count(ItemCounter::Waypoint)+counter.count(ItemCounter::Routepoint);

    mWidget = new QWidget(this);
    mLayout = new QGridLayout(mWidget);
    mEnableSection = true;

    addRow(i18nc("@title:row", "Total points:"), mTotalPoints);
    addRow(i18nc("@title:row", "With time:"), counter.count(ItemCounter::DetailTime));
    addRow(i18nc("@title:row", "With elevation:"), counter.count(ItemCounter::DetailEle));
    mLayout->setRowMinimumHeight(mLayout->rowCount(), DialogBase::verticalSpacing());

    addRow(i18nc("@title:row", "Track points:"), counter.count(ItemCounter::Trackpoint), true);
    addRow(i18nc("@title:row", "With GPS speed:"), counter.count(ItemCounter::DetailSpeed));
    addRow(i18nc("@title:row", "With GPS HDOP:"), counter.count(ItemCounter::DetailHdop));
    addRow(i18nc("@title:row", "With GPS heading:"), counter.count(ItemCounter::DetailHeading));
    mLayout->setRowMinimumHeight(mLayout->rowCount(), DialogBase::verticalSpacing());

    addRow(i18nc("@title:row", "Waypoints:"), counter.count(ItemCounter::Waypoint), true);
    addRow(TrackData::formattedWaypointStatus(TrackData::StatusTodo)+':', counter.count(ItemCounter::StatusTodo));
    addRow(TrackData::formattedWaypointStatus(TrackData::StatusDone)+':', counter.count(ItemCounter::StatusDone));
    addRow(TrackData::formattedWaypointStatus(TrackData::StatusQuestion)+':', counter.count(ItemCounter::StatusQuestion));
    addRow(TrackData::formattedWaypointStatus(TrackData::StatusUnwanted)+':', counter.count(ItemCounter::StatusUnwanted));
    addRow(i18nc("@title:row", "None/Other:"), counter.count(ItemCounter::StatusOther));
    mLayout->setRowMinimumHeight(mLayout->rowCount(), DialogBase::verticalSpacing());

    addRow(i18nc("@title:row", "Route points:"), counter.count(ItemCounter::Routepoint), true);

    mLayout->setRowStretch(mLayout->rowCount(), 1);
    mLayout->setColumnStretch(5, 1);
    mLayout->setColumnMinimumWidth(2, DialogBase::horizontalSpacing());
    mLayout->setColumnMinimumWidth(4, DialogBase::horizontalSpacing());

    setMainWidget(mWidget);
    stateWatcher()->setSaveOnButton(buttonBox()->button(QDialogButtonBox::Close));
}


void StatisticsWidget::addRow(const QString &text, int num, bool newSection)
{
    const int row = mLayout->rowCount();

    if (newSection) mEnableSection = (num>0);

    QLabel *l = new QLabel(text, mWidget);
    l->setEnabled(mEnableSection);
    mLayout->addWidget(l, row, 0, Qt::AlignRight);

    l = new QLabel(QString::number(num), mWidget);
    l->setEnabled(mEnableSection);
    mLayout->addWidget(l, row, 1, Qt::AlignRight);

    // Do not show a percentage for the grand total (it will of course
    // always be 100%).  QGridLayout::rowCount() above will have
    // returned 1 for the initial empty layout.
    if (row>1)
    {
        const int pct = qRound(num*100.0/mTotalPoints);
        l = new QLabel(QString("(%1%)").arg(pct), mWidget);
        l->setEnabled(mEnableSection);
        mLayout->addWidget(l, row, 3);

        QProgressBar *p = new QProgressBar(mWidget);
        p->setMinimum(0);
        p->setMaximum(100);
        p->setValue(pct);
        p->setMaximumHeight(p->height()/2);
        p->setTextVisible(false);

        QPalette pal = p->palette();
#ifdef USE_KCOLORSCHEME
        KColorScheme sch(QPalette::Normal);
        KColorScheme::BackgroundRole back = KColorScheme::NeutralBackground;
        if (pct<20) back = KColorScheme::NegativeBackground;
        else if (pct>80) back = KColorScheme::PositiveBackground;

        pal.setColor(QPalette::Normal, QPalette::Highlight, sch.background(back).color());
#else
        Qt::GlobalColor back;
        if (pct<20) back = Qt::darkGray;
        else if (pct<40) back = Qt::darkRed;
        else if (pct<60) back = Qt::red;
        else if (pct<80) back = Qt::darkYellow;
        else if (pct<90) back = Qt::darkGreen;
        else back = Qt::green;

        pal.setColor(QPalette::Normal, QPalette::Highlight, back);
        pal.setColor(QPalette::Inactive, QPalette::Highlight, back);
#endif
        p->setPalette(pal);
        p->setEnabled(mEnableSection);
        mLayout->addWidget(p, row, 5);
    }
}
