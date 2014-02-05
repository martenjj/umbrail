
#include "trackpropertiesgeneralpages.h"

#include <qformlayout.h>
#include <qgridlayout.h>

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



TrackDataLabel::TrackDataLabel(const QString &str, QWidget *parent)
    : QLabel(str, parent)
{
    init();
}



TrackDataLabel::TrackDataLabel(const QDateTime &dt, QWidget *parent)
    : QLabel(TrackData::formattedTime(dt), parent)
{
    init();
}



TrackDataLabel::TrackDataLabel(double lat, double lon, QWidget *parent)
    : QLabel(TrackData::formattedLatLong(lat, lon), parent)
{
    init();
}



TrackDataLabel::TrackDataLabel(int i, QWidget *parent)
    : QLabel(QString::number(i), parent)
{
    init();
}



void TrackDataLabel::init()
{
    setTextInteractionFlags(Qt::TextSelectableByMouse|Qt::TextSelectableByKeyboard);
}








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
    TrackDataLabel *l = new TrackDataLabel(tsp.start(), this);
    mFormLayout->addRow(i18nc("@label:textbox", "Time start:"), l);

    l = new TrackDataLabel(tsp.finish(), this);
    mFormLayout->addRow(i18nc("@label:textbox", "Time end:"), l);

    unsigned tt = tsp.timeSpan();
    l = new TrackDataLabel(TrackData::formattedDuration(tt), this);
    mFormLayout->addRow(i18nc("@label:textbox", "Time span:"), l);

    if (bothTimes)
    {
        tt = TrackData::sumTotalTravelTime(items);
        l = new TrackDataLabel(TrackData::formattedDuration(tt), this);
        mFormLayout->addRow(i18nc("@label:textbox", "Travel time:"), l);
    }

    double dist = TrackData::sumTotalTravelDistance(items);
    VariableUnitDisplay *vl = new VariableUnitDisplay(VariableUnitDisplay::Distance, this);
    vl->setSaveId("totaltraveldistance");
    vl->setValue(dist);
    mFormLayout->addRow(i18nc("@label:textbox", "Travel distance:"), vl);

    double averageSpeed = dist/(tt/3600.0);
    vl = new VariableUnitDisplay(VariableUnitDisplay::Speed, this);
    vl->setSaveId("averagespeed");
    vl->setValue(averageSpeed);
    mFormLayout->addRow(i18nc("@label:textbox", "Average speed:"), vl);
}



void TrackItemGeneralPage::addBoundingAreaField(const QList<TrackDataItem *> &items)
{
    BoundingArea bb = TrackData::unifyBoundingAreas(items);
    TrackDataLabel *l = new TrackDataLabel(bb.north(), bb.west(), this);
    mFormLayout->addRow(i18nc("@label:textbox", "Bounding area:"), l);
    l = new TrackDataLabel(bb.south(), bb.east(), this);
    mFormLayout->addRow(QString::null, l);
}



void TrackItemGeneralPage::addChildCountField(const QList<TrackDataItem *> &items, const QString &labelText)
{
    if (items.count()!=1) return;			// only for a single item

    TrackDataLabel *l = new TrackDataLabel(items.first()->childCount(), this);
    mFormLayout->addRow(labelText, l);
}










TrackFileGeneralPage::TrackFileGeneralPage(const QList<TrackDataItem *> items, QWidget *pnt)
    : TrackItemGeneralPage(items, pnt)
{
    kDebug();
    setObjectName("TrackFileGeneralPage");

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
    setObjectName("TrackTrackGeneralPage");

    mTypeLabel->setText(i18ncp("@item:intable", "<b>Track</b>", "<b>%1 tracks</b>", items.count()));

    addChildCountField(items, i18nc("@label:textbox", "Segments:"));
    addBoundingAreaField(items);
    addTimeDistanceSpeedFields(items);
}






TrackSegmentGeneralPage::TrackSegmentGeneralPage(const QList<TrackDataItem *> items, QWidget *pnt)
    : TrackItemGeneralPage(items, pnt)
{
    kDebug();
    setObjectName("TrackSegmentGeneralPage");

    mTypeLabel->setText(i18ncp("@item:intable", "<b>Segment</b>", "<b>%1 segments</b>", items.count()));

    addChildCountField(items, i18nc("@label:textbox", "Points:"));
    addBoundingAreaField(items);
    addTimeDistanceSpeedFields(items, false);
}






TrackPointGeneralPage::TrackPointGeneralPage(const QList<TrackDataItem *> items, QWidget *pnt)
    : TrackItemGeneralPage(items, pnt)
{
    kDebug();
    setObjectName("TrackPointGeneralPage");

    mTypeLabel->setText(i18ncp("@item:intable", "<b>Point</b>", "<b>%1 points</b>", items.count()));

    if (items.count()==1)				// single selection
    {
        const TrackDataPoint *p = dynamic_cast<const TrackDataPoint *>(items.first());
        Q_ASSERT(p!=NULL);

        TrackDataLabel *l = new TrackDataLabel(p->formattedPosition(), this);
        mFormLayout->addRow(i18nc("@label:textbox", "Position:"), l);

        l = new TrackDataLabel(p->time(), this);
        mFormLayout->addRow(i18nc("@label:textbox", "Time:"), l);

        double ele = p->elevation();
        if (!isnan(ele))
        {
            VariableUnitDisplay *vl = new VariableUnitDisplay(VariableUnitDisplay::Elevation, this);
            vl->setSaveId("elevation");
            vl->setValue(ele);
            mFormLayout->addRow(i18nc("@label:textbox", "Elevation:"), vl);
        }

        mFormLayout->addRow(new QLabel(this));

        QString s = p->hdop();
        if (!s.isEmpty())
        {
            l = new TrackDataLabel(s, this);
            mFormLayout->addRow(i18nc("@label:textbox", "GPS HDOP:"), l);
        }

        s = p->speed();
        if (!s.isEmpty())
        {
            double speed = s.toDouble();
            l = new TrackDataLabel(QString::number(speed, 'f', 3), this);
            mFormLayout->addRow(i18nc("@label:textbox", "GPS speed:"), l);
        }
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







QWidget *TrackDataRoot::createPropertiesGeneralPage(const QList<TrackDataItem *> items, QWidget *pnt)
{
    return (NULL);
}


QWidget *TrackDataFile::createPropertiesGeneralPage(const QList<TrackDataItem *> items, QWidget *pnt)
{
    return (new TrackFileGeneralPage(items, pnt));
}


QWidget *TrackDataTrack::createPropertiesGeneralPage(const QList<TrackDataItem *> items, QWidget *pnt)
{
    return (new TrackTrackGeneralPage(items, pnt));
}


QWidget *TrackDataSegment::createPropertiesGeneralPage(const QList<TrackDataItem *> items, QWidget *pnt)
{
    return (new TrackSegmentGeneralPage(items, pnt));
}


QWidget *TrackDataPoint::createPropertiesGeneralPage(const QList<TrackDataItem *> items, QWidget *pnt)
{
    return (new TrackPointGeneralPage(items, pnt));
}
