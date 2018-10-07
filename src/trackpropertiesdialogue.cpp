
#include "trackpropertiesdialogue.h"

#include <qgridlayout.h>
#include <qlabel.h>
#include <qdebug.h>
#include <qtabwidget.h>

#include <klocalizedstring.h>
#include <kiconloader.h>
#include <kconfiggroup.h>
#include <kstandardguiitem.h>

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

    Q_ASSERT(!items->isEmpty());
    const TrackDataItem *item = items->first();
    Q_ASSERT(item!=NULL);
    mItemType = item->type();
    mFileTimeZone = item->timeZone();

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

    mTabWidget = new QTabWidget(this);
    mTabWidget->setTabsClosable(false);

    gl->addWidget(mTabWidget, 1, 0, 1, -1);

    setMainWidget(w);

    const TrackPropertiesInterface *propif = dynamic_cast<const TrackPropertiesInterface *>(item);
    Q_ASSERT(propif!=nullptr);

    TrackPropertiesPage *page = propif->createPropertiesGeneralPage(items, this);
    mGeneralPage = qobject_cast<TrackItemGeneralPage *>(page);
    Q_ASSERT(mGeneralPage!=nullptr);
    addPage(page, i18nc("@title:tab", "General"));

    typeLabel->setText(mGeneralPage->typeText(items->count()));

    page = propif->createPropertiesDetailPage(items, this);
    mDetailPage = qobject_cast<TrackItemDetailPage *>(page);
    Q_ASSERT(mDetailPage!=nullptr);
    addPage(page, i18nc("@title:tab", "Details"));

    page = propif->createPropertiesStylePage(items, this);
    addPage(page, i18nc("@title:tab", "Style"), (items->count()==1));
    mStylePage = qobject_cast<TrackItemStylePage *>(page);

    page = propif->createPropertiesPlotPage(items, this);
    addPage(page, i18nc("@title:tab", "Plot"), (items->count()==1));
    mPlotPage = qobject_cast<TrackItemPlotPage *>(page);

    page = propif->createPropertiesMetadataPage(items, this);
    addPage(page, i18nc("@title:tab", "Metadata"), (items->count()==1));
    mMetadataPage = qobject_cast<TrackItemMetadataPage *>(page);
    Q_ASSERT(mMetadataPage!=nullptr);

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







// QString TrackPropertiesDialogue::newItemName() const
// {
    // return (mGeneralPage->newItemName());
// }


// QString TrackPropertiesDialogue::newItemDesc() const
// {
    // return (mGeneralPage->newItemDesc());
// }


// QString TrackPropertiesDialogue::newTimeZone() const
// {
//     return (mGeneralPage->newTimeZone());
// }


// QString TrackPropertiesDialogue::newBearingData() const
// {
//     if (mPlotPage==nullptr) return (QString());		// not applicable to this type
//     return (mPlotPage->newBearingData());
// }
// 
// 
// QString TrackPropertiesDialogue::newRangeData() const
// {
//     if (mPlotPage==nullptr) return (QString());		// not applicable to this type
//     return (mPlotPage->newRangeData());
// }


// TrackData::WaypointStatus TrackPropertiesDialogue::newWaypointStatus() const
// {
    // return (mGeneralPage->newWaypointStatus());
// }


// QColor TrackPropertiesDialogue::newColour() const
// {
//     if (mStylePage==nullptr) return (QColor());		// not applicable to this type
// 
//     const int idx = mTabWidget->indexOf(mStylePage);
//     Q_ASSERT(idx!=-1);					// not applicable to selection
//     if (!mTabWidget->isTabEnabled(idx)) return (QColor());
// 
//     return (mStylePage->newColour());			// colour from page
// }


// bool TrackPropertiesDialogue::newPointPosition(double *newLat, double *newLon) const
// {
    // return (mGeneralPage->newPointPosition(newLat, newLon));
// }


// QString TrackPropertiesDialogue::newTrackType() const
// {
    // TrackTrackGeneralPage *trackPage = qobject_cast<TrackTrackGeneralPage *>(mGeneralPage);
    // if (trackPage!=NULL) return (trackPage->newTrackType());	// setting from page
// 
    // TrackSegmentGeneralPage *segPage = qobject_cast<TrackSegmentGeneralPage *>(mGeneralPage);
    // if (segPage!=NULL) return (segPage->newTrackType());	// setting from page
// 
    // return ("-");					// not applicable, but
// }							// not the same as "blank"


void TrackPropertiesDialogue::saveConfig(QDialog *dialog, KConfigGroup &grp) const
{
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
