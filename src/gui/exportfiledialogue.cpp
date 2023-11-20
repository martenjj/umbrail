
#include "exportfiledialogue.h"

#include <qformlayout.h>
#include <qcheckbox.h>
#include <qtreeview.h>
#include <qdebug.h>
#include <qheaderview.h>
#include <qlabel.h>

#include <klocalizedstring.h>
#include <kurlrequester.h>

#include <kfdialog/recentsaver.h>

#include "importerexporterbase.h"
#include "filesmodel.h"
#include "trackdata.h"
#include "pointicon.h"

//////////////////////////////////////////////////////////////////////////
//									//
//  HomePointsModel							//
//									//
//////////////////////////////////////////////////////////////////////////

// TODO: use a QSortFilterProxyModel on the PointsModel

enum COLUMN
{
    COL_HOME,
    COL_WORK,
    COL_NAME,						// name
    COL_COUNT						// how many - must be last
};


HomePointsModel::HomePointsModel(QObject*pnt)
    : QAbstractTableModel(pnt)
{
}


void HomePointsModel::setSourceModel(QAbstractItemModel *srcModel)
{
    mSourceModel = srcModel;
    connect(srcModel, &QAbstractItemModel::modelReset, this, &HomePointsModel::slotRebuildPointsList);
    connect(srcModel, &QAbstractItemModel::layoutChanged, this, &HomePointsModel::slotRebuildPointsList);

    slotRebuildPointsList();
}


int HomePointsModel::columnCount(const QModelIndex &idx) const
{
    return (COL_COUNT);
}


QVariant HomePointsModel::data(const QModelIndex &idx, int role) const
{
    const int row = idx.row();
    const int col = idx.column();
    const TrackDataItem *item = mPoints.value(row);

    switch (role)
    {
case Qt::DisplayRole:
        if (col==COL_NAME) return (item==nullptr ? i18nc("No point selected", "(none)") : item->name());
        break;

case Qt::DecorationRole:
        if (col==COL_NAME) return (item==nullptr ? QIcon::fromTheme("edit-delete") : item->icon()->icon());
        break;

case Qt::CheckStateRole:
        if (col==COL_HOME) return (row==mHomeIndex ? Qt::Checked : Qt::Unchecked);
        if (col==COL_WORK) return (row==mWorkIndex ? Qt::Checked : Qt::Unchecked);
        break;
    }

    return (QVariant());
}


QVariant HomePointsModel::headerData(int section, Qt::Orientation orient, int role) const
{
    if (orient==Qt::Horizontal)				// horizontal header only
    {
        if (role==Qt::DisplayRole && orient==Qt::Horizontal)
        {
            if (section==COL_NAME) return (i18n("Point"));
        }

        if (role==Qt::DecorationRole)
        {
            if (section==COL_HOME) return (QIcon::fromTheme("go-home"));
            if (section==COL_WORK) return (QIcon::fromTheme("meeting-participant"));
        }
    }

    return (QVariant());
}


bool HomePointsModel::setData(const QModelIndex &idx, const QVariant &value, int role)
{
    const int row = idx.row();
    const int col = idx.column();

    qDebug() << "row" << row << "col" << idx.column() << "role" << role << "value" << value;

    if (role==Qt::CheckStateRole)
    {
        if (col==COL_HOME) mHomeIndex = (value==Qt::Checked ? row : -1);
        if (col==COL_WORK) mWorkIndex = (value==Qt::Checked ? row : -1);
        // The check boxes are exclusive, so the entire column needs
        // to be marked as changed when one is toggled.
        emit dataChanged(createIndex(0, col), createIndex(rowCount(QModelIndex())-1, col));
    }

    return (true);
}


Qt::ItemFlags HomePointsModel::flags(const QModelIndex &idx) const
{
    Qt::ItemFlags f = Qt::ItemIsEnabled|Qt::ItemNeverHasChildren;
    if (idx.column()==COL_HOME || idx.column()==COL_WORK) f |= Qt::ItemIsUserCheckable;
    return (f);
}


void HomePointsModel::slotRebuildPointsList()
{
    beginResetModel();
    mPoints.clear();
    mPoints.append(nullptr);				// special value for "none"

    mHomeIndex = -1;					// nothing selected yet
    mWorkIndex = -1;

    FilesModel *filesModel = qobject_cast<FilesModel *>(mSourceModel);
    Q_ASSERT(filesModel!=nullptr);
    const TrackDataItem *root = filesModel->rootFileItem();
    if (root!=nullptr) buildPointsList(root);		// may not have been set yet

    endResetModel();
    qDebug() << "total points" << mPoints.count();
}


void HomePointsModel::buildPointsList(const TrackDataItem *item)
{
    if (dynamic_cast<const TrackDataWaypoint *>(item)!=nullptr)
    {
        TrackData::WaypointFlags f = static_cast<TrackData::WaypointFlags>(item->metadata("flags").toInt());
        if (f & TrackData::HomePoint) mPoints.append(item);
    }
    else						// assuming waypoints have no children
    {
        const int n = item->childCount();
        for (int i = 0; i<n; ++i) buildPointsList(item->childAt(i));
    }
}


QString HomePointsModel::homePoint() const
{
    return (mHomeIndex<=0 ? QString() : mPoints[mHomeIndex]->name());
}


QString HomePointsModel::workPoint() const
{
    return (mWorkIndex<=0 ? QString() : mPoints[mWorkIndex]->name());
}

//////////////////////////////////////////////////////////////////////////
//									//
//  ExportFileDialogue							//
//									//
//////////////////////////////////////////////////////////////////////////

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
    HomePointsModel *listModel = new HomePointsModel(this);
    listModel->setSourceModel(model);
    listModel->sort(2);
    mListView->setModel(listModel);
    connect(mListView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &ExportFileDialogue::slotSettingChanged);
}


QUrl ExportFileDialogue::selectedUrl() const
{
    QUrl u = mUrlRequester->url();
    RecentSaver recent("export");
    recent.save(u);
    return (u);
}


QString ExportFileDialogue::homePoint() const
{
    HomePointsModel *listModel = qobject_cast<HomePointsModel *>(mListView->model());
    Q_ASSERT(listModel!=nullptr);
    return (listModel->homePoint());
}


QString ExportFileDialogue::workPoint() const
{
    HomePointsModel *listModel = qobject_cast<HomePointsModel *>(mListView->model());
    Q_ASSERT(listModel!=nullptr);
    return (listModel->workPoint());
}


void ExportFileDialogue::slotSettingChanged()
{
    setButtonEnabled(QDialogButtonBox::Ok, mUrlRequester->url().isValid());
}
