
#include "importfiledialogue.h"

#include <qformlayout.h>
#include <qcheckbox.h>
#include <qurl.h>

#include <klocalizedstring.h>
#include <kurlrequester.h>

#include <kfdialog/recentsaver.h>

#include "importerbase.h"


ImportFileDialogue::ImportFileDialogue(const QString &filter, QWidget *pnt)
    : DialogBase(pnt)
{
    setObjectName("ImportFileDialogue");

    setButtons(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
    setButtonText(QDialogButtonBox::Ok, i18n("Import"));
    setWindowTitle(i18n("Import File"));
    setButtonEnabled(QDialogButtonBox::Ok, false);

    QWidget *w = new QWidget(this);
    QFormLayout *form = new QFormLayout(w);
    setMainWidget(w);

    mUrlRequester = new KUrlRequester;
    if (!filter.isEmpty()) mUrlRequester->setNameFilter(filter);

    RecentSaver recent("import");
    mUrlRequester->setStartDir(recent.recentUrl());

    mUrlRequester->setWindowTitle(i18n("Select Import File"));
    connect(mUrlRequester, SIGNAL(textChanged(const QString &)), SLOT(slotUrlChanged(const QString &)));
    form->addRow(i18nc("@label:textbox", "File:"), mUrlRequester);

    form->setItem(form->rowCount(), QFormLayout::SpanningRole, DialogBase::verticalSpacerItem());

    mNoHomeCheck = new QCheckBox(i18nc("@option:check", "Ignore \"Home\" and \"Work\" waypoints"));
    mNoHomeCheck->setChecked(true);
    form->addRow(i18nc("@title:row", "Options:"), mNoHomeCheck);

    mMergeWaypointsCheck = new QCheckBox(i18nc("@option:check", "Merge with existing folders and waypoints"));
    mMergeWaypointsCheck->setChecked(false);
    form->addRow("", mMergeWaypointsCheck);

    setMinimumWidth(450);
}


void ImportFileDialogue::setOptions(ImporterExporterBase::Options opts)
{
    mNoHomeCheck->setChecked(opts & ImporterExporterBase::IgnoreHome);
    if (opts & ImporterExporterBase::MergeNotAllowed) mMergeWaypointsCheck->setEnabled(false);
    else mMergeWaypointsCheck->setChecked(opts & ImporterExporterBase::MergeWaypoints);
}


QUrl ImportFileDialogue::selectedUrl() const
{
    QUrl u = mUrlRequester->url();
    RecentSaver recent("import");
    recent.save(u);
    return (u);
}


void ImportFileDialogue::slotUrlChanged(const QString &text)
{
    setButtonEnabled(QDialogButtonBox::Ok, !text.isEmpty() && QUrl(text).isValid());
}


ImporterExporterBase::Options ImportFileDialogue::options() const
{
    ImporterExporterBase::Options opts = ImporterExporterBase::NoOption;
    if (mNoHomeCheck->isChecked()) opts |= ImporterExporterBase::IgnoreHome;
    if (mMergeWaypointsCheck->isChecked()) opts |= ImporterExporterBase::MergeWaypoints;
    return (opts);
}
