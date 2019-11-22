
#include "trackpropertiesdialogue.h"

#include <qgridlayout.h>
#include <qlabel.h>
#include <qdebug.h>
#include <qtabwidget.h>
#include <qpushbutton.h>

#include <klocalizedstring.h>
#include <kiconloader.h>
#include <kconfiggroup.h>
#include <kstandardguiitem.h>

#include <dialogstatewatcher.h>

#include "trackdata.h"
#include "metadatamodel.h"
#include "dataindexer.h"
#include "trackpropertiespage.h"
#include "trackpropertiesgeneralpages.h"
#include "trackpropertiesdetailpages.h"
#include "trackpropertiesstylepages.h"
#include "trackpropertiesplotpages.h"
#include "trackpropertiesmetadatapages.h"


static int sNextPageIndex = -1;				// page index to open with


TrackPropertiesDialogue::TrackPropertiesDialogue(const QList<TrackDataItem *> *items, QWidget *pnt)
    : DialogBase(pnt),
      DialogStateSaver(this)
{
    setObjectName("TrackPropertiesDialogue");

    setModal(true);
    setButtons(QDialogButtonBox::Ok|QDialogButtonBox::Close);
    stateWatcher()->setSaveOnButton(buttonBox()->button(QDialogButtonBox::Close));
    mCloseButtonShown = true;

    Q_ASSERT(!items->isEmpty());
    const TrackDataItem *item = items->first();
    Q_ASSERT(item!=nullptr);
    mItemType = item->type();

    mDataModel = new MetadataModel(item, this);

    QWidget *w = new QWidget(this);
    QGridLayout *gl = new QGridLayout(w);
    gl->setColumnMinimumWidth(0, DialogBase::horizontalSpacing());
    gl->setColumnStretch(1, 1);
    gl->setColumnMinimumWidth(3, DialogBase::horizontalSpacing());

    QLabel *typeLabel = new QLabel("?", this);
    gl->addWidget(typeLabel, 0, 1, Qt::AlignLeft);

    // TODO: if the items are files, there are more than one, and they are
    // of mixed types (although we only support GPX files at present), the
    // icon should be "unknown".
    QLabel *iconLabel = new QLabel(this);
    iconLabel->setPixmap(item->icon().pixmap(KIconLoader::SizeMedium));
    gl->addWidget(iconLabel, 0, 2, Qt::AlignRight);

    gl->setRowMinimumHeight(1, 2*DialogBase::verticalSpacing());

    mTabWidget = new QTabWidget(this);
    mTabWidget->setTabsClosable(false);
    gl->addWidget(mTabWidget, 2, 0, 1, -1);

    setMainWidget(w);

    const TrackPropertiesInterface *propif = dynamic_cast<const TrackPropertiesInterface *>(item);
    Q_ASSERT(propif!=nullptr);

    TrackPropertiesPage *page = propif->createPropertiesGeneralPage(items, this);
    addPage(page, i18nc("@title:tab", "General"));
    TrackItemGeneralPage *gp = qobject_cast<TrackItemGeneralPage *>(page);
    Q_ASSERT(gp!=nullptr);
    typeLabel->setText(gp->typeText(items->count()));

    page = propif->createPropertiesDetailPage(items, this);
    addPage(page, i18nc("@title:tab", "Details"));

    page = propif->createPropertiesStylePage(items, this);
    addPage(page, i18nc("@title:tab", "Style"), (items->count()==1));

    page = propif->createPropertiesPlotPage(items, this);
    addPage(page, i18nc("@title:tab", "Plot"), (items->count()==1));

    page = propif->createPropertiesMetadataPage(items, this);
    addPage(page, i18nc("@title:tab", "Metadata"), (items->count()==1));

    setButtonEnabled(QDialogButtonBox::Ok, false);	// no data changed yet

    setMinimumSize(320,380);
    setStateSaver(this);
							// now that everything is set up
    connect(mTabWidget, &QTabWidget::currentChanged, this, &TrackPropertiesDialogue::slotTabChanged);
    connect(mDataModel, &MetadataModel::metadataChanged, this, &TrackPropertiesDialogue::slotModelDataChanged);
}


void TrackPropertiesDialogue::addPage(TrackPropertiesPage *page,
                                      const QString &title,
                                      bool enabled)
{
    if (page!=nullptr)					// if page applies to item type
    {
        page->setDataModel(mDataModel);
        mTabWidget->addTab(page, title);
        mTabWidget->setTabEnabled(mTabWidget->count()-1, enabled);
    }
    else						// no page, just add a dummy one
    {
        QWidget *dummyPage = new QWidget(this);
        mTabWidget->addTab(dummyPage, title);
        mTabWidget->setTabEnabled(mTabWidget->count()-1, false);
    }

    mPageDataChanged.append(true);			// resize page update list
}							// and need refresh first time


void TrackPropertiesDialogue::slotModelDataChanged(int idx)
{
    qDebug() << DataIndexer::self()->name(idx);

    bool ok = true;
    const int num = mTabWidget->count();
    for (int i = 0; i<num; ++i)
    {
        TrackPropertiesPage *page = qobject_cast<TrackPropertiesPage *>(mTabWidget->widget(i));
        if (page==nullptr) continue;			// cast will fail for dummy pages
        mPageDataChanged[i] = true;			// note change needed next show
        if (!page->isDataValid()) ok = false;		// check page data valid
    }

    setButtonEnabled(QDialogButtonBox::Ok, ok);
    setButtonGuiItem(QDialogButtonBox::Close, KStandardGuiItem::cancel());
    mCloseButtonShown = false;
}


static void blockChildSignals(const QObjectList &widgets, bool block)
{
    for (QObjectList::const_iterator it = widgets.begin(); it!=widgets.end(); ++it)
    {
        QWidget *w = qobject_cast<QWidget *>(*it);
        if (w!=nullptr) w->blockSignals(block);
    }
}


void TrackPropertiesDialogue::slotTabChanged(int idx)
{
    qDebug() << "tab" << idx << "changed?" << mPageDataChanged[idx];
    if (!mPageDataChanged[idx]) return;			// nothing to do

    TrackPropertiesPage *page = qobject_cast<TrackPropertiesPage *>(mTabWidget->widget(idx));
    if (page==nullptr) return;				// cast will fail for dummy pages

    // Block GUI widget change signals over the call to refreshData(),
    // so that they do not try to update the model again with the same data
    // that they have just read from it.  Just blocking 'page' signals will
    // not have the desired effect.
    //
    // See https://www.qtcentre.org/threads/37730-Blocksignals-for-all-widgets-within-a-QDialog

    const QObjectList &pageWidgets = page->children();
    blockChildSignals(pageWidgets, true);
    page->refreshData();				// update page display data
    blockChildSignals(pageWidgets, false);

    mPageDataChanged[idx] = false;			// note page now up to date
}


void TrackPropertiesDialogue::showEvent(QShowEvent *ev)
{
    DialogBase::showEvent(ev);				// restore size and shown tab
    slotTabChanged(mTabWidget->currentIndex());		// refresh tab now showing
}


void TrackPropertiesDialogue::saveConfig(QDialog *dialog, KConfigGroup &grp) const
{
    // Because of the setSaveOnButton(QDialogButtonBox::Close) used in
    // the constructor, this will be called when that button is used
    // regardless of whether it says "Close" or "Cancel".  So we use
    // the flag to check what the button says, and only save state
    // if it is still "Close".
    if (result()==QDialog::Rejected)			// "Close" or "Cancel" button
    {
        if (!mCloseButtonShown) return;			// button is currently "Cancel"
    }

    grp.writeEntry(QString("Index%1").arg(mItemType), mTabWidget->currentIndex());
    DialogStateSaver::saveConfig(dialog, grp);
}


void TrackPropertiesDialogue::restoreConfig(QDialog *dialog, const KConfigGroup &grp)
{
    int page = sNextPageIndex;				// page explicitly requested
    sNextPageIndex = -1;				// note as now used
    if (page==-1) page = grp.readEntry(QString("Index%1").arg(mItemType), 0);
							// if none set, get from config
    mTabWidget->setCurrentIndex(page);

    DialogStateSaver::restoreConfig(dialog, grp);
}


void TrackPropertiesDialogue::setNextPageIndex(int page)
{
    sNextPageIndex = page;				// page to open next time
}
