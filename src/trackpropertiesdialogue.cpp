
#include "trackpropertiesdialogue.h"

#include <qgridlayout.h>
#include <qlabel.h>
#include <qdebug.h>
#include <qtabwidget.h>

#include <klocalizedstring.h>
#include <kiconloader.h>
#include <kconfiggroup.h>

#include "trackdata.h"
#include "trackpropertiespage.h"
#include "trackpropertiesgeneralpages.h"
#include "trackpropertiesdetailpages.h"
#include "trackpropertiesstylepages.h"
#include "trackpropertiesmetadatapages.h"
#include "style.h"


static int sNextPageIndex = -1;				// page index to open with


TrackPropertiesDialogue::TrackPropertiesDialogue(const QList<TrackDataItem *> *items, QWidget *pnt)
    : DialogBase(pnt),
      DialogStateSaver(this)
{
    setObjectName("TrackPropertiesDialogue");

    setModal(true);
    setButtons(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);

    Q_ASSERT(!items->isEmpty());
    const TrackDataItem *item = items->first();
    Q_ASSERT(item!=NULL);
    QString zoneName = item->timeZone();

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
    Q_ASSERT(propif!=NULL);

    TrackPropertiesPage *page = propif->createPropertiesGeneralPage(items, this);
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

    page = propif->createPropertiesDetailPage(items, this);
    mDetailPage = qobject_cast<TrackItemDetailPage *>(page);
    Q_ASSERT(mDetailPage!=NULL);
    mDetailPage->setTimeZone(zoneName);
    connect(mDetailPage, SIGNAL(dataChanged()), SLOT(slotDataChanged()));
    connect(mGeneralPage, SIGNAL(timeZoneChanged(const QString &)),
            mDetailPage, SLOT(setTimeZone(const QString &)));
    connect(mGeneralPage, SIGNAL(pointPositionChanged(double,double)),
            mDetailPage, SLOT(slotPointPositionChanged(double,double)));

    mTabWidget->addTab(page, i18nc("@title:tab", "Details"));

    page = propif->createPropertiesStylePage(items, this);
    mStylePage = qobject_cast<TrackItemStylePage *>(page);
    Q_ASSERT(mStylePage!=NULL);
    mStylePage->setTimeZone(zoneName);
    connect(mStylePage, SIGNAL(dataChanged()), SLOT(slotDataChanged()));
    connect(mGeneralPage, SIGNAL(timeZoneChanged(const QString &)),
            mStylePage, SLOT(setTimeZone(const QString &)));
    mTabWidget->addTab(page, i18nc("@title:tab", "Style"));

    page = propif->createPropertiesMetadataPage(items, this);
    mMetadataPage = qobject_cast<TrackItemMetadataPage *>(page);
    Q_ASSERT(mMetadataPage!=NULL);
    connect(mMetadataPage, SIGNAL(dataChanged()), SLOT(slotDataChanged()));
    mTabWidget->addTab(page, i18nc("@title:tab", "Metadata"));

    setMinimumSize(320,380);
    setStateSaver(this);

    // TODO: hasStyle() a virtual of TrackDataItem
    bool styleEnabled = (items->count()==1);		// whether "Style" is applicable here
    if (styleEnabled && dynamic_cast<const TrackDataTrackpoint *>(items->first())!=NULL) styleEnabled = false;
    //if (styleEnabled && dynamic_cast<const TrackDataWaypoint *>(items->first())!=NULL) styleEnabled = false;
    if (styleEnabled && dynamic_cast<const TrackDataRoutepoint *>(items->first())!=NULL) styleEnabled = false;
    if (styleEnabled && dynamic_cast<const TrackDataFolder *>(items->first())!=NULL) styleEnabled = false;
    mTabWidget->setTabEnabled(2, styleEnabled);

    bool metadataEnabled = (items->count()==1);		// whether "Metadata" is applicable here
    mTabWidget->setTabEnabled(3, metadataEnabled);
}


void TrackPropertiesDialogue::slotDataChanged()
{
    setButtonEnabled(QDialogButtonBox::Ok, mGeneralPage->isDataValid() && mDetailPage->isDataValid() && mStylePage->isDataValid());
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


QString TrackPropertiesDialogue::newBearingLine() const
{
    return (mGeneralPage->newBearingLine());
}


TrackData::WaypointStatus TrackPropertiesDialogue::newWaypointStatus() const
{
    return (mGeneralPage->newWaypointStatus());
}


Style TrackPropertiesDialogue::newStyle() const
{
    if (!mTabWidget->isTabEnabled(2)) return (Style::null);	// style not applicable
    return (mStylePage->newStyle());				// style from page
}


bool TrackPropertiesDialogue::newPointPosition(double *newLat, double *newLon) const
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


void TrackPropertiesDialogue::saveConfig(QDialog *dialog, KConfigGroup &grp) const
{
    grp.writeEntry("Index", mTabWidget->currentIndex());
    DialogStateSaver::saveConfig(dialog, grp);
}


void TrackPropertiesDialogue::restoreConfig(QDialog *dialog, const KConfigGroup &grp)
{
    int page = sNextPageIndex;				// page explicitly requested
    sNextPageIndex = -1;				// note as now used
    if (page==-1) page = grp.readEntry("Index", 0);	// if none set, get from config
    mTabWidget->setCurrentIndex(page);

    DialogStateSaver::restoreConfig(dialog, grp);
}


void TrackPropertiesDialogue::setNextPageIndex(int page)
{
    sNextPageIndex = page;				// page to open next time
}
