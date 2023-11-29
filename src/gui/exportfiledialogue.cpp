
#include "exportfiledialogue.h"

#include <qformlayout.h>
#include <qtreeview.h>
#include <qdebug.h>
#include <qheaderview.h>
#include <qlabel.h>

#include <klocalizedstring.h>
#include <kurlrequester.h>

#include <kfdialog/recentsaver.h>

#include "importerexporterbase.h"
#include "homepointsdatamodel.h"


ExportFileDialogue::ExportFileDialogue(const QString &filter, QWidget *pnt)
    : DialogBase(pnt)
{
    setObjectName("ExportFileDialogue");

    setModal(true);
    setButtons(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
    setButtonText(QDialogButtonBox::Ok, i18n("Export"));
    setWindowTitle(i18n("Export File"));
    setButtonEnabled(QDialogButtonBox::Ok, false);

    QWidget *w = new QWidget(this);
    QFormLayout *form = new QFormLayout(w);
    setMainWidget(w);

    mUrlRequester = new KUrlRequester(w);
    if (!filter.isEmpty()) mUrlRequester->setNameFilter(filter);

    RecentSaver recent("export");
    mUrlRequester->setStartDir(recent.recentUrl());

    // TODO: no overwrite option passed to file dialog
    mUrlRequester->setMode(KFile::File|KFile::LocalOnly);
    mUrlRequester->setAcceptMode(QFileDialog::AcceptSave);
    mUrlRequester->setWindowTitle(i18n("Select Export File"));
    connect(mUrlRequester, &KUrlRequester::textChanged, this, &ExportFileDialogue::slotSettingChanged);
    form->addRow(i18nc("@label:textbox", "File:"), mUrlRequester);

    form->setItem(form->rowCount(), QFormLayout::SpanningRole, DialogBase::verticalSpacerItem());

    // The HomePointsModel allows a point selection to be toggled off
    // as well as on.  Therefore there is no need fo a check box to enable
    // generation of the "Home and "Work" points, nor a "None" row in the
    // selection list box.

    QLabel *l = new QLabel(i18n("Select the \"Home\" and \"Work\" points:"), this);
    form->addRow(l);

    mListView = new QTreeView(w);
    mListView->setAlternatingRowColors(true);
    mListView->setRootIsDecorated(false);
    mListView->setSelectionBehavior(QAbstractItemView::SelectRows);
    mListView->setSelectionMode(QAbstractItemView::NoSelection);
    mListView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    mListView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    QHeaderView *header = mListView->header();
    header->setSectionResizeMode(QHeaderView::ResizeToContents);
    header->setStretchLastSection(true);
    mListView->setToolTip(i18nc("@info:tooltip",
                                "Select the waypoint to be copied as \"Home\" or \"Work\" here.<br/>"
                                "Only those which have the \"Home point\" flag set are shown."));
    form->addRow(mListView);

    setMinimumSize(400, 300);
    slotSettingChanged();
}


void ExportFileDialogue::setSourceModel(QAbstractItemModel *model)
{
     mListView->setModel(model);
     connect(mListView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &ExportFileDialogue::slotSettingChanged);
}


QUrl ExportFileDialogue::selectedUrl() const
{
    QUrl u = mUrlRequester->url();
    RecentSaver recent("export");
    recent.save(u);
    return (u);
}


void ExportFileDialogue::slotSettingChanged()
{
    setButtonEnabled(QDialogButtonBox::Ok, mUrlRequester->url().isValid());
}


ImporterExporterOptions ExportFileDialogue::options() const
{
    HomePointsDataModel *listModel = qobject_cast<HomePointsDataModel *>(mListView->model());
    Q_ASSERT(listModel!=nullptr);

    ImporterExporterOptions opts;
    opts.setHomePoint(listModel->homePoint());
    opts.setWorkPoint(listModel->workPoint());
    return (opts);
}
