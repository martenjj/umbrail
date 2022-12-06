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

#include "iconselector.h"

#include <qlistwidget.h>
#include <qcombobox.h>
#include <qformlayout.h>
#include <qpushbutton.h>
#include <qdebug.h>

#include <klocalizedstring.h>

#include "pointiconprovider.h"


IconSelector::IconSelector(const QString &sym, QWidget *pnt)
    : DialogBase(pnt)
{
    mSelectedName = sym;

    setObjectName("IconSelector");
    setWindowTitle(i18n("Select Symbol"));
    setButtons(QDialogButtonBox::Ok|QDialogButtonBox::Cancel|QDialogButtonBox::Reset);
    setButtonText(QDialogButtonBox::Ok, i18nc("@action:button", "Select"));
    setButtonText(QDialogButtonBox::Reset, i18nc("@action:button", "Clear"));

    QWidget *w = new QWidget(this);
    QFormLayout *fl = new QFormLayout(w);

    mSourceCombo = new QComboBox(this);
    mSourceCombo->setSizePolicy(QSizePolicy::Expanding, mSourceCombo->sizePolicy().verticalPolicy());
    // TODO: icons to identify vendor
    mSourceCombo->addItem(i18nc("Name of symbol set", "Garmin"), PointIcon::NamespaceGarmin);
    mSourceCombo->addItem(i18nc("Name of symbol set", "OsmAnd"), PointIcon::NamespaceOsmand);
    fl->addRow(i18n("Symbol set:"), mSourceCombo);

    fl->addItem(DialogBase::verticalSpacerItem());

    mList = new QListWidget(this);
    mList->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    mList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    mList->setSelectionMode(QAbstractItemView::SingleSelection);
    mList->setViewMode(QListView::IconMode);
    mList->setMovement(QListView::Static);
    mList->setUniformItemSizes(true);
    mList->setSpacing(DialogBase::verticalSpacing());
    mList->setResizeMode(QListView::Adjust);
    mList->setMinimumWidth(400);

    fl->addRow(mList);

    setMainWidget(w);
    slotSourceChanged();
    slotSelectionChanged();

    connect(mSourceCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &IconSelector::slotSourceChanged);
    connect(mList, &QListWidget::itemSelectionChanged, this, &IconSelector::slotSelectionChanged);
    connect(buttonBox()->button(QDialogButtonBox::Reset), &QAbstractButton::clicked, this, &IconSelector::slotClearIcon);
}


QString IconSelector::selectedIconName() const
{
    QList<QListWidgetItem *> sel = mList->selectedItems();
    if (sel.isEmpty()) return (QString());
    QListWidgetItem *item = sel.first();
    return (item->text());
}


PointIcon::IconNamespace IconSelector::selectedNamespace() const
{
    return (static_cast<PointIcon::IconNamespace>(mSourceCombo->currentData().toInt()));
}


void IconSelector::slotSourceChanged()
{
    PointIcon::IconNamespace nsp = selectedNamespace();

    mList->clear();
    QListWidgetItem *selectedItem = nullptr;
    QStringList names = PointIcon::allNames(nsp);
    // Hopefully more efficient to sort the names before creating the
    // list view, instead of sorting the view items afterwards.
    std::sort(names.begin(), names.end());

    for (const QString &name : qAsConst(names))
    {
        QListWidgetItem *item = new QListWidgetItem(PointIconProvider::self()->icon(name, nsp)->icon(), name);
        item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
        // AutoToolTipDelegate does not work for this sort of view,
        // so unconditionally set the tool tip.
        item->setToolTip(name);
        item->setSizeHint(QSize(80, 50));
        mList->addItem(item);

        if (name==mSelectedName) selectedItem = item;
    }

    if (selectedItem!=nullptr)
    {
        selectedItem->setSelected(true);
        mList->setCurrentItem(selectedItem);
        mList->scrollToItem(selectedItem, QAbstractItemView::PositionAtCenter);
    }
    setButtonEnabled(QDialogButtonBox::Ok, mList->selectedItems().count()==1);
}


void IconSelector::slotSelectionChanged()
{
    setButtonEnabled(QDialogButtonBox::Ok, mList->selectedItems().count()==1);
}


void IconSelector::slotClearIcon()
{
    mList->setCurrentItem(nullptr);
    accept();
}
