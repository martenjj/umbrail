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

#include "trackpropertiespage.h"

#include <qformlayout.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qdebug.h>
#include <qtimezone.h>
#include <qboxlayout.h>

#include <klocalizedstring.h>
#include <kmessagewidget.h>

#include <kfdialog/dialogbase.h>

#include "trackdata.h"
#include "metadatamodel.h"


// This constructor cannot use dataModel(), because it has not been set yet.
TrackPropertiesPage::TrackPropertiesPage(const QList<TrackDataItem *> *items, QWidget *pnt)
    : QWidget(pnt),
      ApplicationDataInterface(pnt)
{
    Q_ASSERT(items!=nullptr);
    Q_ASSERT(!items->isEmpty());

    mTimeZoneWarning = nullptr;

    mMainLayout = new QVBoxLayout(this);
    mFormLayout = new QFormLayout(nullptr);
    mMainLayout->addLayout(mFormLayout);

    mIsEmpty = (TrackData::sumTotalChildCount(items)==0);
    if (mIsEmpty)
    {
        if (IS(TrackDataAbstractPoint, items->first())) mIsEmpty = false;
    }
}


void TrackPropertiesPage::addSeparatorField(const QString &title)
{
    if (title.isEmpty())				// no title, just some space
    {
        mFormLayout->addItem(new QSpacerItem(1, DialogBase::verticalSpacing(), QSizePolicy::Minimum, QSizePolicy::Fixed));
    }
    else						// title, a separator line
    {
        QGroupBox *sep = new QGroupBox(title, this);
        sep->setFlat(true);
        mFormLayout->addRow(sep);
    }
}


void TrackPropertiesPage::disableIfEmpty(QWidget *field, bool always)
{
    if (!isEmpty() && !always) return;

    QWidget *l = mFormLayout->labelForField(field);
    if (l!=nullptr) l->setEnabled(false);
    field->setEnabled(false);
}


// This may be called during derived class construction, so it cannot
// use dataModel() for the same reasons as above.
void TrackPropertiesPage::addTimeZoneWarning()
{
    if (mTimeZoneWarning!=nullptr) return;

    mTimeZoneWarning = new KMessageWidget(this);
    mTimeZoneWarning->setMessageType(KMessageWidget::Warning);
    mTimeZoneWarning->setPosition(KMessageWidget::Inline);
    mTimeZoneWarning->setIcon(QIcon::fromTheme("dialog-warning"));
    mTimeZoneWarning->setCloseButtonVisible(false);
    mTimeZoneWarning->setWordWrap(true);
    mTimeZoneWarning->setText(i18n("The file time zone is not set. Times are displayed in UTC."));
    mTimeZoneWarning->setVisible(false);

    mMainLayout->addStretch(1);
    mMainLayout->addWidget(mTimeZoneWarning);
}


void TrackPropertiesPage::updateTimeZoneWarning()
{
    if (mTimeZoneWarning==nullptr) return;

    const QTimeZone *tz = dataModel()->timeZone();
    mTimeZoneWarning->setVisible(tz==nullptr || !tz->isValid());
}
