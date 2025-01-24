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

#include "addresseditdialogue.h"

#include <qlabel.h>
#include <qlineedit.h>
#include <qtextedit.h>
#include <qgridlayout.h>
#include <qdebug.h>
#include <qpushbutton.h>

#include <klocalizedstring.h>

#include "dataindexer.h"
#include "metadatamodel.h"
#include "trackdata.h"


// Based on NavMarks AddressDialogue but without the "Clear" button
AddressEditDialogue::AddressEditDialogue(MetadataModel *model, QWidget *pnt)
    : DialogBase(pnt)
{
    setObjectName("AddressEditDialogue");
    setModal(true);
    setWindowTitle(i18n("Edit Address"));
    setButtons(QDialogButtonBox::Ok|QDialogButtonBox::Cancel|QDialogButtonBox::Reset);

    connect(this, &QDialog::accepted, this, &AddressEditDialogue::slotAccept);
    connect(buttonBox()->button(QDialogButtonBox::Reset), &QAbstractButton::clicked, this, &AddressEditDialogue::slotReset);

    mModel = model;

    QWidget *w = new QWidget(this);
    QGridLayout *lay = new QGridLayout(w);

    mStreetEdit = new QLineEdit(w);
    mStreetEdit->setClearButtonEnabled(true);
    connect(mStreetEdit, &QLineEdit::textEdited, this, &AddressEditDialogue::slotTextChanged);
    lay->addWidget(mStreetEdit, 0, 1);
    QLabel *l = new QLabel(i18n("Street:"), w);
    l->setBuddy(mStreetEdit);
    lay->addWidget(l, 0, 0, Qt::AlignRight);

    mCityEdit = new QLineEdit(w);
    mCityEdit->setClearButtonEnabled(true);
    connect(mCityEdit, &QLineEdit::textEdited, this, &AddressEditDialogue::slotTextChanged);
    lay->addWidget(mCityEdit, 1, 1);
    l = new QLabel(i18n("City:"), w);
    l->setBuddy(mCityEdit);
    lay->addWidget(l, 1, 0, Qt::AlignRight);

    mStateEdit = new QLineEdit(w);
    mStateEdit->setClearButtonEnabled(true);
    connect(mStateEdit, &QLineEdit::textEdited, this, &AddressEditDialogue::slotTextChanged);
    lay->addWidget(mStateEdit, 2, 1);
    l = new QLabel(i18n("State:"), w);
    l->setBuddy(mStateEdit);
    lay->addWidget(l, 2, 0, Qt::AlignRight);

    mPostCodeEdit = new QLineEdit(w);
    mPostCodeEdit->setClearButtonEnabled(true);
    connect(mPostCodeEdit, &QLineEdit::textEdited, this, &AddressEditDialogue::slotTextChanged);
    lay->addWidget(mPostCodeEdit, 3, 1);
    l = new QLabel(i18n("Post Code:"), w);
    l->setBuddy(mPostCodeEdit);
    lay->addWidget(l, 3, 0, Qt::AlignRight);

    // TODO: country a dropdown of known ones
    mCountryEdit = new QLineEdit(w);
    mCountryEdit->setClearButtonEnabled(true);
    connect(mCountryEdit, &QLineEdit::textEdited, this, &AddressEditDialogue::slotTextChanged);
    lay->addWidget(mCountryEdit, 4, 1);
    l = new QLabel(i18n("Country:"), w);
    l->setBuddy(mCountryEdit);
    lay->addWidget(l, 4, 0, Qt::AlignRight);

    mAddressPreview = new QTextEdit(w);
    mAddressPreview->setReadOnly(true);
    mAddressPreview->setLineWrapMode(QTextEdit::NoWrap);
    mAddressPreview->setTabChangesFocus(true);
    mAddressPreview->setWordWrapMode(QTextOption::NoWrap);
    mAddressPreview->setMinimumHeight(100);
    lay->addWidget(mAddressPreview, 6, 1, Qt::AlignTop);
    l = new QLabel(i18n("Preview:"), w);
    lay->addWidget(l, 6, 0, Qt::AlignTop|Qt::AlignRight);

    lay->setRowStretch(6, 1);
    lay->setColumnStretch(1, 1);
    lay->setRowMinimumHeight(5, DialogBase::verticalSpacing());


    setMainWidget(w);
    w->setMinimumWidth(300);

    slotReset();					// set fields from data
}    							// and update the preview



void AddressEditDialogue::slotAccept()
{
    mModel->setData(DataIndexer::index("streetaddress"), mStreetEdit->text());
    mModel->setData(DataIndexer::index("city"), mCityEdit->text());
    mModel->setData(DataIndexer::index("state"), mStateEdit->text());
    mModel->setData(DataIndexer::index("postalcode"), mPostCodeEdit->text());
    mModel->setData(DataIndexer::index("country"), mCountryEdit->text());
}


void AddressEditDialogue::slotReset()
{
    mStreetEdit->setText(mModel->data("streetaddress").toString());
    mCityEdit->setText(mModel->data("city").toString());
    mStateEdit->setText(mModel->data("state").toString());
    mPostCodeEdit->setText(mModel->data("postalcode").toString());
    mCountryEdit->setText(mModel->data("country").toString());

    slotTextChanged();					// update the preview
}


void AddressEditDialogue::slotTextChanged()
{
    mAddressPreview->setText(TrackData::formattedAddress(mStreetEdit->text(),
                                                         mCityEdit->text(),
                                                         mStateEdit->text(),
                                                         mPostCodeEdit->text(),
                                                         mCountryEdit->text()).join('\n'));
}
