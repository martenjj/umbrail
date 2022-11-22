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
#include <qformlayout.h>
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
    setButtons(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);

    connect(this, &QDialog::accepted, this, &AddressEditDialogue::slotAccept);

    mModel = model;

    QWidget *w = new QWidget(this);
    QFormLayout *lay = new QFormLayout(w);

    mStreetEdit = new QLineEdit(w);
    mStreetEdit->setText(model->data("StreetAddress").toString());
    mStreetEdit->setClearButtonEnabled(true);
    connect(mStreetEdit, &QLineEdit::textEdited, this, &AddressEditDialogue::slotTextChanged);
    lay->addRow(i18n("Street:"), mStreetEdit);

    mCityEdit = new QLineEdit(w);
    mCityEdit->setText(model->data("City").toString());
    mCityEdit->setClearButtonEnabled(true);
    connect(mCityEdit, &QLineEdit::textEdited, this, &AddressEditDialogue::slotTextChanged);
    lay->addRow(i18n("City:"), mCityEdit);

    mStateEdit = new QLineEdit(w);
    mStateEdit->setText(model->data("State").toString());
    mStateEdit->setClearButtonEnabled(true);
    connect(mStateEdit, &QLineEdit::textEdited, this, &AddressEditDialogue::slotTextChanged);
    lay->addRow(i18n("State:"), mStateEdit);

    mPostCodeEdit = new QLineEdit(w);
    mPostCodeEdit->setText(model->data("PostalCode").toString());
    mPostCodeEdit->setClearButtonEnabled(true);
    connect(mPostCodeEdit, &QLineEdit::textEdited, this, &AddressEditDialogue::slotTextChanged);
    lay->addRow(i18n("Post Code:"), mPostCodeEdit);

    // TODO: country a dropdown of known ones
    mCountryEdit = new QLineEdit(w);
    mCountryEdit->setText(model->data("Country").toString());
    mCountryEdit->setClearButtonEnabled(true);
    connect(mCountryEdit, &QLineEdit::textEdited, this, &AddressEditDialogue::slotTextChanged);
    lay->addRow(i18n("Country:"), mCountryEdit);

    lay->addItem(DialogBase::verticalSpacerItem());

    mAddressPreview = new QTextEdit(w);
    mAddressPreview->setReadOnly(true);
    mAddressPreview->setLineWrapMode(QTextEdit::NoWrap);
    mAddressPreview->setTabChangesFocus(true);
    mAddressPreview->setWordWrapMode(QTextOption::NoWrap);
    mAddressPreview->setMaximumHeight(80);
    lay->addRow(i18n("Preview:"),  mAddressPreview);

    setMainWidget(w);
    w->setMinimumWidth(300);

    slotTextChanged();					// update preview
}


void AddressEditDialogue::slotAccept()
{
    mModel->setData(DataIndexer::index("StreetAddress"), mStreetEdit->text());
    mModel->setData(DataIndexer::index("City"), mCityEdit->text());
    mModel->setData(DataIndexer::index("State"), mStateEdit->text());
    mModel->setData(DataIndexer::index("PostalCode"), mPostCodeEdit->text());
    mModel->setData(DataIndexer::index("Country"), mCountryEdit->text());
}


void AddressEditDialogue::slotTextChanged()
{
    mAddressPreview->setText(TrackData::formattedAddress(mStreetEdit->text(),
                                                         mCityEdit->text(),
                                                         mStateEdit->text(),
                                                         mPostCodeEdit->text(),
                                                         mCountryEdit->text()).join('\n'));
}
