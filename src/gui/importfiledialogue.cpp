
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
    connect(mUrlRequester, &KUrlRequester::textChanged, this, &ImportFileDialogue::slotUrlChanged);
    form->addRow(i18nc("@label:textbox", "File:"), mUrlRequester);

    form->setItem(form->rowCount(), QFormLayout::SpanningRole, DialogBase::verticalSpacerItem());

    mNoHomeCheck = new QCheckBox(i18nc("@option:check", "Ignore \"Home\" and \"Work\" waypoints"));
    mNoHomeCheck->setChecked(true);
    form->addRow(i18nc("@title:row", "Options:"), mNoHomeCheck);

    mMergeWaypointsCheck = new QCheckBox(i18nc("@option:check", "Merge with existing folders and waypoints"));
    mMergeWaypointsCheck->setChecked(false);
    form->addRow("", mMergeWaypointsCheck);

    mMarkNewWaypointsCheck = new QCheckBox(i18nc("@option:check", "Mark newly imported waypoints"));
    mMarkNewWaypointsCheck->setChecked(false);
    form->addRow("", mMarkNewWaypointsCheck);

    setMinimumWidth(450);
}


void ImportFileDialogue::setOptions(const ImporterExporterOptions &opts)
{
    mNoHomeCheck->setChecked(opts.hasFlag(ImporterExporterOptions::IgnoreHome));
    if (opts.hasFlag(ImporterExporterOptions::MergeNotAllowed)) mMergeWaypointsCheck->setEnabled(false);
    else mMergeWaypointsCheck->setChecked(opts.hasFlag(ImporterExporterOptions::MergeWaypoints));
    mMarkNewWaypointsCheck->setChecked(opts.hasFlag(ImporterExporterOptions::MarkNewWaypoints));
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
    setButtonEnabled(QDialogButtonBox::Ok, mUrlRequester->url().isValid());
}


ImporterExporterOptions ImportFileDialogue::options() const
{
    ImporterExporterOptions::Flags f = ImporterExporterOptions::NoFlags;
    if (mNoHomeCheck->isChecked()) f |= ImporterExporterOptions::IgnoreHome;
    if (mMergeWaypointsCheck->isChecked()) f |= ImporterExporterOptions::MergeWaypoints;
    if (mMarkNewWaypointsCheck->isChecked()) f |= ImporterExporterOptions::MarkNewWaypoints;
    return (ImporterExporterOptions(f));
}
