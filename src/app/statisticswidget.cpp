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


StatisticsWidget::StatisticsWidget(QWidget *pnt)
    : DialogBase(pnt),
      ApplicationDataInterface(pnt)
{
    setObjectName("StatisticsWidget");
    setButtons(QDialogButtonBox::Close);

    mTotalPoints = 0;					// for all types of point
    mWithTime = 0;
    mWithElevation = 0;

    mTrackpoints = 0;					// classification of points
    mWaypoints = 0;
    mRoutepoints = 0;

    mWithGpsSpeed = 0;					// for track points
    mWithGpsHdop = 0;
    mWithGpsHeading = 0;

    mStatusTodo = 0;					// for waypoints
    mStatusDone = 0;
    mStatusUnwanted = 0;
    mStatusOther = 0;

    QVector<const TrackDataAbstractPoint *> points;	// all points, not just trackpoints
    filesController()->filesView()->selectedPoints(false).swap(points);
    for (const TrackDataAbstractPoint *tdp : std::as_const(points)) getPointData(tdp);

    mWidget = new QWidget(this);
    mLayout = new QGridLayout(mWidget);
    mEnableSection = true;

    addRow(i18nc("@title:row", "Total points:"), mTotalPoints);
    addRow(i18nc("@title:row", "With time:"), mWithTime);
    addRow(i18nc("@title:row", "With elevation:"), mWithElevation);
    mLayout->setRowMinimumHeight(mLayout->rowCount(), DialogBase::verticalSpacing());

    addRow(i18nc("@title:row", "Track points:"), mTrackpoints, true);
    addRow(i18nc("@title:row", "With GPS speed:"), mWithGpsSpeed);
    addRow(i18nc("@title:row", "With GPS HDOP:"), mWithGpsHdop);
    addRow(i18nc("@title:row", "With GPS heading:"), mWithGpsHeading);
    mLayout->setRowMinimumHeight(mLayout->rowCount(), DialogBase::verticalSpacing());

    addRow(i18nc("@title:row", "Waypoints:"), mWaypoints, true);
    addRow(TrackData::formattedWaypointStatus(TrackData::StatusTodo)+':', mStatusTodo);
    addRow(TrackData::formattedWaypointStatus(TrackData::StatusDone)+':', mStatusDone);
    addRow(TrackData::formattedWaypointStatus(TrackData::StatusQuestion)+'/'+
           TrackData::formattedWaypointStatus(TrackData::StatusUnwanted)+':', mStatusUnwanted);
    addRow(i18nc("@title:row", "None/Other:"), mStatusOther);
    mLayout->setRowMinimumHeight(mLayout->rowCount(), DialogBase::verticalSpacing());

    addRow(i18nc("@title:row", "Route points:"), mRoutepoints, true);

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


void StatisticsWidget::getPointData(const TrackDataAbstractPoint *point)
{
    const TrackDataAbstractPoint *tdp = AS(TrackDataAbstractPoint, point);
    if (tdp!=nullptr)					// is this a point?
    {
        ++mTotalPoints;					// count up total points

        const QDateTime dt = tdp->time();		// time available
        if (dt.isValid()) ++mWithTime;

        const double ele = tdp->elevation();		// elevation available
        if (!ISNAN(ele) && ele!=0) ++mWithElevation;	// (and also nozero)

        const QVariant speedMeta = tdp->metadata("speed");
        if (!speedMeta.isNull()) ++mWithGpsSpeed;	// GPS speed recorded

        const QVariant hdopMeta = tdp->metadata("hdop");
        if (!hdopMeta.isNull()) ++mWithGpsHdop;		// GPS HDOP recorded

        const QVariant headingMeta = tdp->metadata("heading");
        if (!headingMeta.isNull()) ++mWithGpsHeading;	// GPS heading recorded

        const TrackDataWaypoint *tdw = AS(TrackDataWaypoint, point);
        if (tdw!=nullptr)				// is this a waypoint?
        {
            ++mWaypoints;
            const TrackData::WaypointStatus status = static_cast<TrackData::WaypointStatus>(tdw->metadata("status").toInt());
            switch (status)
            {
case TrackData::StatusTodo:		++mStatusTodo;		break;
case TrackData::StatusDone:		++mStatusDone;		break;
case TrackData::StatusQuestion:
case TrackData::StatusUnwanted:		++mStatusUnwanted;	break;
default:				++mStatusOther;		break;
            }
        }
        else
        {
            if (IS(TrackDataRoutepoint, tdp)) ++mRoutepoints;
            else ++mTrackpoints;
        }
    }
}
