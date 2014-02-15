
#include "trackpropertiesdialogue.h"

#include <qgridlayout.h>
#include <qlabel.h>

#include <kdebug.h>
#include <klocale.h>
#include <ktabwidget.h>

#include "trackdata.h"
#include "trackpropertiespage.h"
#include "trackpropertiesgeneralpages.h"
#include "trackpropertiesdetailpages.h"
#include "trackpropertiesstylepages.h"
#include "style.h"



TrackPropertiesDialogue::TrackPropertiesDialogue(const QList<TrackDataItem *> &items, QWidget *pnt)
    : KDialog(pnt)
{
    setObjectName("TrackPropertiesDialogue");

    setModal(true);
    setButtons(KDialog::Ok|KDialog::Cancel);
    showButtonSeparator(false);

    Q_ASSERT(!items.isEmpty());
    TrackDataDisplayable *item = dynamic_cast<TrackDataDisplayable *>(items.first());
    Q_ASSERT(item!=NULL);

    QWidget *w = new QWidget(this);
    QGridLayout *gl = new QGridLayout(w);

    gl->setColumnMinimumWidth(0, KDialog::marginHint());
    gl->setColumnStretch(1, 1);
    gl->setColumnMinimumWidth(3, KDialog::marginHint());

    QLabel *typeLabel = new QLabel("?", this);
    gl->addWidget(typeLabel, 0, 1, Qt::AlignLeft);

    // TODO: if the items are files, there are more than one, and they are
    // of mixed types (although we only support GPX files at present), the
    // icon should be "unknown".
    QLabel *iconLabel = new QLabel(this);
    iconLabel->setPixmap(KIconLoader::global()->loadIcon(item->iconName(),
                                                         KIconLoader::NoGroup,
                                                         KIconLoader::SizeMedium));
    gl->addWidget(iconLabel, 0, 2, Qt::AlignRight);
    gl->setRowMinimumHeight(1, KDialog::spacingHint());

    mTabWidget = new KTabWidget(this);
    mTabWidget->setTabsClosable(false);
    gl->addWidget(mTabWidget, 2, 0, 1, -1);

    setMainWidget(w);

    TrackPropertiesPage *page = item->createPropertiesGeneralPage(items, this);
    mGeneralPage = qobject_cast<TrackItemGeneralPage *>(page);
    Q_ASSERT(mGeneralPage!=NULL);
    connect(mGeneralPage, SIGNAL(dataChanged()), SLOT(slotDataChanged()));
    mTabWidget->addTab(page, i18nc("@title:tab", "General"));

    typeLabel->setText(mGeneralPage->typeText(items.count()));

    page = item->createPropertiesDetailPage(items, this);
    mDetailPage = qobject_cast<TrackItemDetailPage *>(page);
    Q_ASSERT(mDetailPage!=NULL);
    connect(mDetailPage, SIGNAL(dataChanged()), SLOT(slotDataChanged()));
    mTabWidget->addTab(page, i18nc("@title:tab", "Statistics"));

    page = item->createPropertiesStylePage(items, this);
    mStylePage = qobject_cast<TrackItemStylePage *>(page);
    Q_ASSERT(mStylePage!=NULL);
    connect(mStylePage, SIGNAL(dataChanged()), SLOT(slotDataChanged()));
    mTabWidget->addTab(page, i18nc("@title:tab", "Style"));

    setMinimumSize(320,380);
    KConfigGroup grp = KGlobal::config()->group(objectName());
    restoreDialogSize(grp);
    int idx = grp.readEntry("Index", -1);
    if (idx!=-1) mTabWidget->setCurrentIndex(idx);

    if (items.count()>1 || dynamic_cast<const TrackDataPoint *>(items.first())!=NULL)
    {							// "Style" not applicable here
        mTabWidget->setTabEnabled(2, false);
    }
}


TrackPropertiesDialogue::~TrackPropertiesDialogue()
{
    KConfigGroup grp = KGlobal::config()->group(objectName());
    saveDialogSize(grp);
    grp.writeEntry("Index", mTabWidget->currentIndex());
}





void TrackPropertiesDialogue::slotDataChanged()
{
    enableButtonOk(mGeneralPage->isDataValid() && mDetailPage->isDataValid() && mStylePage->isDataValid());
}



QString TrackPropertiesDialogue::newItemName() const
{
    return (mGeneralPage->newItemName());
}


KUrl TrackPropertiesDialogue::newFileUrl() const
{
    TrackFileGeneralPage *filePage = qobject_cast<TrackFileGeneralPage *>(mGeneralPage);
    if (filePage==NULL) return (KUrl());
    return (filePage->newFileUrl());
}


Style TrackPropertiesDialogue::newStyle() const
{
    if (!mTabWidget->isTabEnabled(2)) return (Style::null);	// style not applicable
    return (mStylePage->newStyle());				// style from page
}
