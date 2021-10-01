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

#include "mapthemedialogue.h"

#include <qlayout.h>
#include <qlabel.h>
#include <qlistwidget.h>
#include <qstandarditemmodel.h>
#include <qdebug.h>
#include <qpushbutton.h>

#include <klocalizedstring.h>


#define PIXMAP_SIZE		40			// size of preview pixmap


MapThemeDialogue::MapThemeDialogue(QStandardItemModel *model, QWidget *pnt)
    : DialogBase(pnt)
{
    setObjectName("MapThemeDialogue");

    setModal(true);
    setButtons(QDialogButtonBox::Ok|QDialogButtonBox::Apply|QDialogButtonBox::Close);
    setWindowTitle(i18n("Select Map Theme"));

    connect(buttonBox()->button(QDialogButtonBox::Ok),  &QPushButton::clicked,
            this, [this]() { emit themeSelected(themeId()); });
    connect(buttonBox()->button(QDialogButtonBox::Apply), &QPushButton::clicked,
            this, [this]() { emit themeSelected(themeId()); });

    QWidget *vb = new QWidget(this);
    QVBoxLayout *vlay = new QVBoxLayout(vb);
    setMainWidget(vb);

    QLabel *l = new QLabel(i18n("Available Themes:"), vb);
    vlay->addWidget(l);

    mListBox = new QListWidget(vb);
    mListBox->setSelectionMode(QAbstractItemView::SingleSelection);
    mListBox->setUniformItemSizes(true);
    vlay->addWidget(mListBox, 1);
    l->setBuddy(mListBox);

    setMinimumSize(QSize(370, 200));

    mModel = model;
    createDisplay();
}


void MapThemeDialogue::setThemeId(const QString &id)
{
    qDebug() << id;

    for (int i = 0; i<mListBox->count(); ++i)
    {
        QListWidgetItem *item = mListBox->item(i);
        if (item==nullptr) continue;

        QString itemId = item->data(Qt::UserRole).toString();
        if (itemId==id)
        {
            item->setSelected(true);
            mListBox->scrollToItem(item, QAbstractItemView::PositionAtCenter);
            break;
        }
    }

}


QString MapThemeDialogue::themeId() const
{
    QList<QListWidgetItem *> selected = mListBox->selectedItems();
    if (selected.count()==0) return (QString());	// should never happen

    QString id = selected.first()->data(Qt::UserRole).toString();
    qDebug() << id;
    return (id);
}


void MapThemeDialogue::createDisplay()
{
    qDebug() << "rows" << mModel->rowCount();
    for (int i = 0; i<mModel->rowCount(); ++i)
    {
        QStandardItem *themeData = mModel->item(i);
        if (themeData==nullptr) continue;

        QString id = themeData->data(Qt::UserRole+1).toString();
        if (!id.startsWith("earth/")) continue;		// only this planet!
        qDebug() << "  " << i << id;

        QListWidgetItem *item = new QListWidgetItem();

        QWidget *hbox = new QWidget(this);
        QHBoxLayout *hlay = new QHBoxLayout(hbox);
        hlay->setMargin(0);
        hlay->setSpacing(DialogBase::horizontalSpacing());

        QLabel *label = new QLabel(hbox);
        QIcon ic = themeData->data(Qt::DecorationRole).value<QIcon>();
        label->setPixmap(ic.pixmap(PIXMAP_SIZE, PIXMAP_SIZE));
        hlay->addSpacing(DialogBase::horizontalSpacing());
        hlay->addWidget(label);

        label = new QLabel(QString("<qt><b>%1</b><br>%2")
                           .arg(themeData->data(Qt::DisplayRole).toString())
                           .arg(id));
        label->setTextInteractionFlags(Qt::NoTextInteraction);
        hlay->addSpacing(DialogBase::horizontalSpacing());
        hlay->addWidget(label);

        hlay->addStretch(1);

        item->setData(Qt::UserRole, id);		// for select/lookup
        item->setToolTip(themeData->data(Qt::ToolTipRole).toString());

        mListBox->addItem(item);
        mListBox->setItemWidget(item, hbox);
        item->setData(Qt::SizeHintRole, QSize(1, PIXMAP_SIZE+4));
    }							// X-size is ignored
}
