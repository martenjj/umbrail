
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



TrackPropertiesDialogue::TrackPropertiesDialogue(const QList<TrackDataItem *> *items, QWidget *pnt)
    : KDialog(pnt)
{
    setObjectName("TrackPropertiesDialogue");

    setModal(true);
    setButtons(KDialog::Ok|KDialog::Cancel);
    showButtonSeparator(false);

    Q_ASSERT(!items->isEmpty());
    const TrackDataItem *item = items->first();
    Q_ASSERT(item!=NULL);
    QString zoneName = item->timeZone();

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

    mTabWidget = new KTabWidget(this);
    mTabWidget->setTabsClosable(false);
    gl->addWidget(mTabWidget, 1, 0, 1, -1);

    setMainWidget(w);

    TrackPropertiesPage *page = item->createPropertiesGeneralPage(items, this);
    mGeneralPage = qobject_cast<TrackItemGeneralPage *>(page);
    Q_ASSERT(mGeneralPage!=NULL);
    mGeneralPage->setTimeZone(zoneName);
    connect(mGeneralPage, SIGNAL(dataChanged()), SLOT(slotDataChanged()));
    connect(mGeneralPage, SIGNAL(timeZoneChanged(const QString &)),
            mGeneralPage, SLOT(setTimeZone(const QString &)));
    connect(mGeneralPage, SIGNAL(pointPositionChanged(double,double)),
            mGeneralPage, SLOT(slotPointPositionChanged(double,double)));

    mTabWidget->addTab(page, i18nc("@title:tab", "General"));

    typeLabel->setText(mGeneralPage->typeText(items->count()));

    page = item->createPropertiesDetailPage(items, this);
    mDetailPage = qobject_cast<TrackItemDetailPage *>(page);
    Q_ASSERT(mDetailPage!=NULL);
    mDetailPage->setTimeZone(zoneName);
    connect(mDetailPage, SIGNAL(dataChanged()), SLOT(slotDataChanged()));
    connect(mGeneralPage, SIGNAL(timeZoneChanged(const QString &)),
            mDetailPage, SLOT(setTimeZone(const QString &)));
    connect(mGeneralPage, SIGNAL(pointPositionChanged(double,double)),
            mDetailPage, SLOT(slotPointPositionChanged(double,double)));

    mTabWidget->addTab(page, i18nc("@title:tab", "Details"));

    page = item->createPropertiesStylePage(items, this);
    mStylePage = qobject_cast<TrackItemStylePage *>(page);
    Q_ASSERT(mStylePage!=NULL);
    mStylePage->setTimeZone(zoneName);
    connect(mStylePage, SIGNAL(dataChanged()), SLOT(slotDataChanged()));
    connect(mGeneralPage, SIGNAL(timeZoneChanged(const QString &)),
            mStylePage, SLOT(setTimeZone(const QString &)));
    mTabWidget->addTab(page, i18nc("@title:tab", "Style"));

    setMinimumSize(320,380);
    KConfigGroup grp = KGlobal::config()->group(objectName());
    restoreDialogSize(grp);
    int idx = grp.readEntry("Index", -1);
    if (idx!=-1) mTabWidget->setCurrentIndex(idx);

    // TODO: hasStyle() a virtual of TrackDataItem
    bool styleEnabled = (items->count()==1);		// whether "Style" is applicable here
    if (styleEnabled && dynamic_cast<const TrackDataTrackpoint *>(items->first())!=NULL) styleEnabled = false;
    if (styleEnabled && dynamic_cast<const TrackDataWaypoint *>(items->first())!=NULL) styleEnabled = false;
    if (styleEnabled && dynamic_cast<const TrackDataFolder *>(items->first())!=NULL) styleEnabled = false;
    mTabWidget->setTabEnabled(2, styleEnabled);
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


QString TrackPropertiesDialogue::newItemDesc() const
{
    return (mGeneralPage->newItemDesc());
}


QString TrackPropertiesDialogue::newTimeZone() const
{
    return (mGeneralPage->newTimeZone());
}


Style TrackPropertiesDialogue::newStyle() const
{
    if (!mTabWidget->isTabEnabled(2)) return (Style::null);	// style not applicable
    return (mStylePage->newStyle());				// style from page
}


bool TrackPropertiesDialogue::newPointPosition(double *newLat, double *newLon)
{
    return (mGeneralPage->newPointPosition(newLat, newLon));
}


QString TrackPropertiesDialogue::newTrackType() const
{
    TrackTrackGeneralPage *trackPage = qobject_cast<TrackTrackGeneralPage *>(mGeneralPage);
    if (trackPage!=NULL) return (trackPage->newTrackType());	// style from page

    TrackSegmentGeneralPage *segPage = qobject_cast<TrackSegmentGeneralPage *>(mGeneralPage);
    if (segPage!=NULL) return (segPage->newTrackType());	// style from page

    return ("-");					// not applicable, but
}							// not the same as "blank"
