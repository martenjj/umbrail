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

#include <klocalizedstring.h>

#include <kfdialog/dialogbase.h>

#include "trackdata.h"


TrackPropertiesPage::TrackPropertiesPage(const QList<TrackDataItem *> *items, QWidget *pnt)
    : QWidget(pnt),
      ApplicationDataInterface(pnt)
{
    Q_ASSERT(items!=nullptr);
    Q_ASSERT(!items->isEmpty());

    mFormLayout = new QFormLayout(this);

    mIsEmpty = (TrackData::sumTotalChildCount(items)==0);
    if (mIsEmpty && !items->isEmpty())
    {
        if (dynamic_cast<const TrackDataAbstractPoint *>(items->first())!=nullptr) mIsEmpty = false;
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
