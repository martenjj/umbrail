
#include "trackproperties.h"

#include <qformlayout.h>
#include <qgridlayout.h>
#include <qlabel.h>

#include <kdebug.h>
#include <kdialog.h>
#include <klocale.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klineedit.h>
#include <kurlrequester.h>
#include <kfiledialog.h>

#include "trackdata.h"
#include "filescontroller.h"
#include "variableunitdisplay.h"












TrackItemGeneralPage::TrackItemGeneralPage(const QList<TrackDataItem *> items, QWidget *pnt)
    : QWidget(pnt)
{
    kDebug();
    setObjectName("TrackItemGeneralPage");
    Q_ASSERT(!items.isEmpty());

    QGridLayout *gl = new QGridLayout(this);

    mTypeLabel = new QLabel("?", this);
    gl->addWidget(mTypeLabel, 0, 0, Qt::AlignLeft);

    mIconLabel = new QLabel(this);
    mIconLabel->setPixmap(KIconLoader::global()->loadIcon(items.first()->iconName(),
                                                          KIconLoader::NoGroup,
                                                          KIconLoader::SizeMedium));
    gl->addWidget(mIconLabel, 0, 1, Qt::AlignRight);
    gl->setRowMinimumHeight(1, 2*KDialog::spacingHint());

    mFormLayout = new QFormLayout;
    mFormLayout->setMargin(0);

    mNameEdit = new KLineEdit(this);
    if (items.count()==1) mNameEdit->setText(items.first()->name());
    else mNameEdit->setEnabled(false);
    connect(mNameEdit, SIGNAL(textChanged(const QString &)), SLOT(slotDataChanged()));

    mFormLayout->addRow(i18nc("@label:textbox", "Name:"), mNameEdit);
    mFormLayout->addRow(new QLabel(this));

    gl->addLayout(mFormLayout, 2, 0, 1, -1);
    gl->setRowStretch(2, 1);
}



void TrackItemGeneralPage::slotDataChanged()
{
    emit enableButtonOk(isDataValid());
}


bool TrackItemGeneralPage::isDataValid() const
{
    bool ok = true;
    if (mNameEdit->isEnabled()) ok = ok && !mNameEdit->text().isEmpty();
    return (ok);
}



QString TrackItemGeneralPage::newItemName() const
{							// only if editable
    if (!mNameEdit->isEnabled()) return (QString::null);
    return (mNameEdit->text());
}



void TrackItemGeneralPage::addTimeDistanceSpeedFields(const QList<TrackDataItem *> &items, bool bothTimes)
{
    TimeRange tsp = TrackData::unifyTimeSpans(items);
    QLabel *l = new QLabel(TrackData::formattedTime(tsp.start()), this);
    l->setTextInteractionFlags(Qt::TextSelectableByMouse|Qt::TextSelectableByKeyboard);
    mFormLayout->addRow(i18nc("@label:textbox", "Time start:"), l);

    l = new QLabel(TrackData::formattedTime(tsp.finish()), this);
    l->setTextInteractionFlags(Qt::TextSelectableByMouse|Qt::TextSelectableByKeyboard);
    mFormLayout->addRow(i18nc("@label:textbox", "Time end:"), l);

    unsigned tt = tsp.timeSpan();
    l = new QLabel(TrackData::formattedDuration(tt), this);
    l->setTextInteractionFlags(Qt::TextSelectableByMouse|Qt::TextSelectableByKeyboard);
    mFormLayout->addRow(i18nc("@label:textbox", "Time span:"), l);

    if (bothTimes)
    {
        tt = TrackData::sumTotalTravelTime(items);
        l = new QLabel(TrackData::formattedDuration(tt), this);
        l->setTextInteractionFlags(Qt::TextSelectableByMouse|Qt::TextSelectableByKeyboard);
        mFormLayout->addRow(i18nc("@label:textbox", "Travel time:"), l);
    }

    double dist = TrackData::sumTotalTravelDistance(items);
    VariableUnitDisplay *vl = new VariableUnitDisplay(VariableUnitDisplay::Distance, this);
    vl->setValue(dist);
    mFormLayout->addRow(i18nc("@label:textbox", "Travel distance:"), vl);

    double averageSpeed = dist/(tt/3600.0);
    vl = new VariableUnitDisplay(VariableUnitDisplay::Speed, this);
    vl->setValue(averageSpeed);
    mFormLayout->addRow(i18nc("@label:textbox", "Average speed:"), vl);
}



void TrackItemGeneralPage::addBoundingAreaField(const QList<TrackDataItem *> &items)
{
    BoundingArea bb = TrackData::unifyBoundingAreas(items);
    QLabel *l = new QLabel(TrackData::formattedLatLong(bb.north(), bb.west()), this);
    l->setTextInteractionFlags(Qt::TextSelectableByMouse|Qt::TextSelectableByKeyboard);
    mFormLayout->addRow(i18nc("@label:textbox", "Bounding area:"), l);
    l = new QLabel(TrackData::formattedLatLong(bb.south(), bb.east()));
    l->setTextInteractionFlags(Qt::TextSelectableByMouse|Qt::TextSelectableByKeyboard);
    mFormLayout->addRow(QString::null, l);
}



void TrackItemGeneralPage::addChildCountField(const QList<TrackDataItem *> &items, const QString &labelText)
{
    if (items.count()!=1) return;			// only for a single item

    QLabel *l = new QLabel(QString::number(items.first()->childCount()), this);
    l->setTextInteractionFlags(Qt::TextSelectableByMouse|Qt::TextSelectableByKeyboard);
    mFormLayout->addRow(labelText, l);
}










TrackFileGeneralPage::TrackFileGeneralPage(const QList<TrackDataItem *> items, QWidget *pnt)
    : TrackItemGeneralPage(items, pnt)
{
    kDebug();
    setObjectName("TrackItemFilePage");

    mTypeLabel->setText(i18ncp("@item:intable", "<b>File</b>", "<b>%1 files</b>", items.count()));

    mNameEdit->setReadOnly(true);			// can't rename here for files

    mUrlRequester = new KUrlRequester(this);
    mUrlRequester->setFilter(FilesController::allImportFilters());
    mUrlRequester->fileDialog()->setCaption("Relocate Tracks File");

    connect(mUrlRequester, SIGNAL(textChanged(const QString &)), SLOT(slotDataChanged()));

    if (items.count()==1)				// a single item
    {
        TrackDataFile *fileItem = dynamic_cast<TrackDataFile *>(items.first());
        Q_ASSERT(fileItem!=NULL);
        mUrlRequester->setUrl(fileItem->fileName());
    }
    else						// may be mixed MIME types
    {
        mIconLabel->setPixmap(KIconLoader::global()->loadIcon("unknown",
                                                              KIconLoader::NoGroup,
                                                              KIconLoader::SizeMedium));
        mUrlRequester->setEnabled(false);		// can't edit for multiple items
    }

    mFormLayout->insertRow(mFormLayout->rowCount()-1, i18nc("@label:textbox", "File:"), mUrlRequester);

    addChildCountField(items, i18nc("@label:textbox", "Tracks:"));
    addBoundingAreaField(items);
    addTimeDistanceSpeedFields(items);
}






bool TrackFileGeneralPage::isDataValid() const
{
    bool ok = TrackItemGeneralPage::isDataValid();
    if (mUrlRequester->isEnabled()) ok = ok && mUrlRequester->url().isValid();
    return (ok);
}




KUrl TrackFileGeneralPage::newFileUrl() const
{							// only if editable
    if (!mUrlRequester->isEnabled()) return (KUrl());
    return (mUrlRequester->url());
}








TrackTrackGeneralPage::TrackTrackGeneralPage(const QList<TrackDataItem *> items, QWidget *pnt)
    : TrackItemGeneralPage(items, pnt)
{
    kDebug();
    setObjectName("TrackItemTrackPage");

    mTypeLabel->setText(i18ncp("@item:intable", "<b>Track</b>", "<b>%1 tracks</b>", items.count()));

    addChildCountField(items, i18nc("@label:textbox", "Segments:"));
    addBoundingAreaField(items);
    addTimeDistanceSpeedFields(items);
}






TrackSegmentGeneralPage::TrackSegmentGeneralPage(const QList<TrackDataItem *> items, QWidget *pnt)
    : TrackItemGeneralPage(items, pnt)
{
    kDebug();
    setObjectName("TrackItemSegmentPage");

    mTypeLabel->setText(i18ncp("@item:intable", "<b>Segment</b>", "<b>%1 segments</b>", items.count()));

    addChildCountField(items, i18nc("@label:textbox", "Points:"));
    addBoundingAreaField(items);
    addTimeDistanceSpeedFields(items, false);
}






TrackPointGeneralPage::TrackPointGeneralPage(const QList<TrackDataItem *> items, QWidget *pnt)
    : TrackItemGeneralPage(items, pnt)
{
    kDebug();
    setObjectName("TrackItemPointPage");

    mTypeLabel->setText(i18ncp("@item:intable", "<b>Point</b>", "<b>%1 points</b>", items.count()));

    if (items.count()==1)				// single selection
    {
        const TrackDataPoint *p = dynamic_cast<const TrackDataPoint *>(items.first());
        Q_ASSERT(p!=NULL);

        QLabel *l = new QLabel(p->formattedPosition(), this);
        l->setTextInteractionFlags(Qt::TextSelectableByMouse|Qt::TextSelectableByKeyboard);
        mFormLayout->addRow(i18nc("@label:textbox", "Position:"), l);

        l = new QLabel(p->formattedTime(), this);
        l->setTextInteractionFlags(Qt::TextSelectableByMouse|Qt::TextSelectableByKeyboard);
        mFormLayout->addRow(i18nc("@label:textbox", "Time:"), l);

        // TODO: variable units
        l = new QLabel(p->formattedElevation(), this);
        l->setTextInteractionFlags(Qt::TextSelectableByMouse|Qt::TextSelectableByKeyboard);
        mFormLayout->addRow(i18nc("@label:textbox", "Elevation:"), l);
    }
    else						// multiple selection
    {
        addBoundingAreaField(items);

        const TrackDataItem *seg = items.first()->parent();
        Q_ASSERT(seg!=NULL);				// find parent segment
        int firstIdx = seg->childIndex(items.first());	// its index of first item
        int num = items.count();

        bool contiguousSelection = true;		// assume so at start
        for (int i = 1; i<num; ++i)			// look at following items
        {
            if (seg->childAt(firstIdx+i)!=items.at(i))	// mismatch children/selection
            {
                contiguousSelection = false;
                break;
            }
        }

        if (contiguousSelection)			// selection is contiguous
        {
            addTimeDistanceSpeedFields(items, false);
        }
    }
}
