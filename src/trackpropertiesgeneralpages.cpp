
#include "trackpropertiesgeneralpages.h"

#include <qformlayout.h>

#include <kdebug.h>
#include <kdialog.h>
#include <klocale.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klineedit.h>
#include <kurlrequester.h>
#include <kfiledialog.h>

#include "trackdata.h"
#include "trackdatalabel.h"
#include "filescontroller.h"
#include "variableunitdisplay.h"







TrackItemGeneralPage::TrackItemGeneralPage(const QList<TrackDataItem *> items, QWidget *pnt)
    : QWidget(pnt)
{
    kDebug();
    setObjectName("TrackItemGeneralPage");
    Q_ASSERT(!items.isEmpty());

    mFormLayout = new QFormLayout(this);

    mNameEdit = new KLineEdit(this);
    if (items.count()==1) mNameEdit->setText(items.first()->name());
    else mNameEdit->setEnabled(false);
    connect(mNameEdit, SIGNAL(textChanged(const QString &)), SLOT(slotDataChanged()));

    addSpacerField();
    mFormLayout->addRow(i18nc("@label:textbox", "Name:"), mNameEdit);
    addSpacerField();
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



void TrackItemGeneralPage::addSpacerField()
{
    mFormLayout->addItem(new QSpacerItem(1, KDialog::spacingHint(), QSizePolicy::Minimum, QSizePolicy::Fixed));
}



void TrackItemGeneralPage::addTimeFields(const QList<TrackDataItem *> &items)
{
    TimeRange tsp = TrackData::unifyTimeSpans(items);
    TrackDataLabel *l = new TrackDataLabel(tsp.start(), this);
    mFormLayout->addRow(i18nc("@label:textbox", "Time start:"), l);

    l = new TrackDataLabel(tsp.finish(), this);
    mFormLayout->addRow(i18nc("@label:textbox", "Time end:"), l);
}








TrackFileGeneralPage::TrackFileGeneralPage(const QList<TrackDataItem *> items, QWidget *pnt)
    : TrackItemGeneralPage(items, pnt)
{
    kDebug();
    setObjectName("TrackFileGeneralPage");

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
//        mIconLabel->setPixmap(KIconLoader::global()->loadIcon("unknown",
//                                                              KIconLoader::NoGroup,
//                                                              KIconLoader::SizeMedium));
        mUrlRequester->setEnabled(false);		// can't edit for multiple items
    }

    mFormLayout->insertRow(mFormLayout->rowCount()-1, i18nc("@label:textbox", "File:"), mUrlRequester);

    addTimeFields(items);
}


QString TrackFileGeneralPage::typeText(int count) const
{
    return (i18ncp("@item:intable", "<b>File</b>", "<b>%1 files</b>", count));
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

    addTimeFields(items);
}


QString TrackTrackGeneralPage::typeText(int count) const
{
    return (i18ncp("@item:intable", "<b>Track</b>", "<b>%1 tracks</b>", count));
}






TrackSegmentGeneralPage::TrackSegmentGeneralPage(const QList<TrackDataItem *> items, QWidget *pnt)
    : TrackItemGeneralPage(items, pnt)
{
    kDebug();
    setObjectName("TrackSegmentGeneralPage");

    addTimeFields(items);
}


QString TrackSegmentGeneralPage::typeText(int count) const
{
    return (i18ncp("@item:intable", "<b>Segment</b>", "<b>%1 segments</b>", count));
}






TrackPointGeneralPage::TrackPointGeneralPage(const QList<TrackDataItem *> items, QWidget *pnt)
    : TrackItemGeneralPage(items, pnt)
{
    kDebug();
    setObjectName("TrackPointGeneralPage");

    if (items.count()==1)				// single selection
    {
        const TrackDataPoint *p = dynamic_cast<const TrackDataPoint *>(items.first());
        Q_ASSERT(p!=NULL);

        TrackDataLabel *l = new TrackDataLabel(p->formattedPosition(), this);
        mFormLayout->addRow(i18nc("@label:textbox", "Position:"), l);

        l = new TrackDataLabel(p->time(), this);
        mFormLayout->addRow(i18nc("@label:textbox", "Time:"), l);
    }
    else						// multiple selection
    {
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
            addTimeFields(items);
        }
    }
}


QString TrackPointGeneralPage::typeText(int count) const
{
    return (i18ncp("@item:intable", "<b>Point</b>", "<b>%1 points</b>", count));
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
