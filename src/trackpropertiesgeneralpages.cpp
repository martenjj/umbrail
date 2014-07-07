
#include "trackpropertiesgeneralpages.h"

#include <qformlayout.h>

#include <kdebug.h>
#include <kdialog.h>
#include <klocale.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klineedit.h>
#include <ktextedit.h>
#include <kfiledialog.h>

#include "trackdata.h"
#include "trackdatalabel.h"
#include "filescontroller.h"
#include "variableunitdisplay.h"
#include "itemtypecombo.h"
#include "timezoneselector.h"





TrackItemGeneralPage::TrackItemGeneralPage(const QList<TrackDataItem *> items, QWidget *pnt)
    : TrackPropertiesPage(items, pnt)
{
    kDebug();
    setObjectName("TrackItemGeneralPage");

    mNameEdit = new KLineEdit(this);
    if (items.count()==1) mNameEdit->setText(items.first()->name());
    else mNameEdit->setEnabled(false);
    connect(mNameEdit, SIGNAL(textChanged(const QString &)), SLOT(slotDataChanged()));

    addSeparatorField();
    mFormLayout->addRow(i18nc("@label:textbox", "Name:"), mNameEdit);
    addSeparatorField();

    mTypeCombo = NULL;					// not applicable yet
    mDescEdit = NULL;
    mTimeZoneSel = NULL;
}




bool TrackItemGeneralPage::isDataValid() const
{
    if (!mNameEdit->isEnabled()) return (true);
    return (!mNameEdit->text().isEmpty());
}



QString TrackItemGeneralPage::newItemName() const
{							// only if editable
    if (!mNameEdit->isEnabled()) return (QString::null);
    return (mNameEdit->text());
}



QString TrackItemGeneralPage::newItemDesc() const
{							// only if editable
    if (mDescEdit==NULL) return ("-");			// not for this data
    if (!mDescEdit->isEnabled()) return ("-");		// not applicable
    return (mDescEdit->toPlainText().trimmed());
}



QString TrackItemGeneralPage::newTrackType() const
{
    if (mTypeCombo==NULL) return ("-");			// not for this data
    if (!mTypeCombo->isEnabled()) return ("-");		// not applicable

    int idx = mTypeCombo->currentIndex();
    if (idx==0) return (QString::null);			// first is always "none"
    return (mTypeCombo->currentText());
}



QString TrackItemGeneralPage::newTimeZone() const
{							// only if editable
    if (mTimeZoneSel==NULL || !mTimeZoneSel->isEnabled()) return (QString::null);
    return (mTimeZoneSel->timeZone());
}



void TrackItemGeneralPage::addTimeFields(const QList<TrackDataItem *> &items)
{
    TimeRange tsp = TrackData::unifyTimeSpans(items);
    TrackDataLabel *l = new TrackDataLabel(tsp.start(), this);
    mFormLayout->addRow(i18nc("@label:textbox", "Time start:"), l);
    disableIfEmpty(l);

    l = new TrackDataLabel(tsp.finish(), this);
    mFormLayout->addRow(i18nc("@label:textbox", "Time end:"), l);
    disableIfEmpty(l);
}





void TrackItemGeneralPage::addTypeDescFields(const QList<TrackDataItem *> &items)
{
    mTypeCombo = new ItemTypeCombo(this);

    mDescEdit = new KTextEdit(this);
    mDescEdit->setMaximumHeight(100);

    if (items.count()==1)
    {
        const TrackDataItem *tdi = items.first();
        Q_ASSERT(tdi!=NULL);

        mTypeCombo->setType(tdi->metadata("type"));
        connect(mTypeCombo, SIGNAL(currentIndexChanged(const QString &)), SLOT(slotDataChanged()));
        connect(mTypeCombo, SIGNAL(editTextChanged(const QString &)), SLOT(slotDataChanged()));

        mDescEdit->setAcceptRichText(false);
        mDescEdit->setTabChangesFocus(true);

        QString d = tdi->metadata("desc");
        if (!d.endsWith('\n')) d += "\n";
        mDescEdit->setPlainText(d);

        connect(mDescEdit, SIGNAL(textChanged()), SLOT(slotDataChanged()));
    }
    else
    {
        mTypeCombo->setEnabled(false);
        mDescEdit->setEnabled(false);
    }

    mFormLayout->addRow(i18nc("@label:listbox", "Type:"), mTypeCombo);
    mFormLayout->addRow(i18nc("@label:listbox", "Description:"), mDescEdit);
}








TrackFileGeneralPage::TrackFileGeneralPage(const QList<TrackDataItem *> items, QWidget *pnt)
    : TrackItemGeneralPage(items, pnt)
{
    kDebug();
    setObjectName("TrackFileGeneralPage");

    mNameEdit->setReadOnly(true);			// can't rename here for files

    mUrlRequester = new KLineEdit(this);
    mUrlRequester->setReadOnly(true);
    connect(mUrlRequester, SIGNAL(textChanged(const QString &)), SLOT(slotDataChanged()));

    mTimeZoneSel = new TimeZoneSelector(this);
    connect(mTimeZoneSel, SIGNAL(zoneChanged(const QString &)), SLOT(slotDataChanged()));
    connect(mTimeZoneSel, SIGNAL(zoneChanged(const QString &)), SIGNAL(timeZoneChanged(const QString &)));

    if (items.count()==1)				// a single item
    {
        TrackDataFile *fileItem = dynamic_cast<TrackDataFile *>(items.first());
        Q_ASSERT(fileItem!=NULL);
        mUrlRequester->setText(fileItem->fileName().pathOrUrl());

        QString zone = fileItem->metadata("timezone");
        if (!zone.isEmpty()) mTimeZoneSel->setTimeZone(zone);
    }
    else						// may be mixed MIME types
    {
        mUrlRequester->setEnabled(false);		// can't edit for multiple items
        mTimeZoneSel->setEnabled(false);
    }

    mFormLayout->insertRow(mFormLayout->rowCount()-1, i18nc("@label:textbox", "File:"), mUrlRequester);
    mFormLayout->addRow(i18nc("@label:textbox", "Time zone:"), mTimeZoneSel);

    addTimeFields(items);
}



QString TrackFileGeneralPage::typeText(int count) const
{
    return (i18ncp("@item:intable", "<b>File</b>", "<b>%1 files</b>", count));
}




bool TrackFileGeneralPage::isDataValid() const
{
    if (mUrlRequester->isEnabled() && mUrlRequester->text().isEmpty()) return (false);
    if (mTimeZoneSel->isEnabled() && mTimeZoneSel->timeZone().isEmpty()) return (false);
    return (TrackItemGeneralPage::isDataValid());
}







TrackTrackGeneralPage::TrackTrackGeneralPage(const QList<TrackDataItem *> items, QWidget *pnt)
    : TrackItemGeneralPage(items, pnt)
{
    kDebug();
    setObjectName("TrackTrackGeneralPage");

    addTimeFields(items);
    addSeparatorField();
    addTypeDescFields(items);
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
    addSeparatorField();
    addTypeDescFields(items);
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


CREATE_PROPERTIES_PAGE(File, General);
CREATE_PROPERTIES_PAGE(Track, General);
CREATE_PROPERTIES_PAGE(Segment, General);
CREATE_PROPERTIES_PAGE(Point, General);
