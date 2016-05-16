
#include "trackpropertiesgeneralpages.h"

#include <qformlayout.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qdebug.h>
#include <qlineedit.h>

#include <klocalizedstring.h>
#include <kiconloader.h>
#include <ktextedit.h>
#include <kfiledialog.h>

#include <dialogbase.h>

#include "trackdata.h"
#include "trackdatalabel.h"
#include "filescontroller.h"
#include "variableunitdisplay.h"
#include "itemtypecombo.h"
#include "timezoneselector.h"
#include "latlongdialogue.h"
#include "mediaplayer.h"


TrackItemGeneralPage::TrackItemGeneralPage(const QList<TrackDataItem *> *items, QWidget *pnt)
    : TrackPropertiesPage(items, pnt)
{
    qDebug();
    setObjectName("TrackItemGeneralPage");

    mNameEdit = new QLineEdit(this);
    if (items->count()==1) mNameEdit->setText(items->first()->name());
    else mNameEdit->setEnabled(false);
    connect(mNameEdit, SIGNAL(textChanged(const QString &)), SLOT(slotDataChanged()));

    addSeparatorField();
    mFormLayout->addRow(i18nc("@label:textbox", "Name:"), mNameEdit);
    addSeparatorField();

    mTypeCombo = NULL;					// not applicable yet
    mStatusCombo = NULL;
    mDescEdit = NULL;
    mTimeZoneSel = NULL;

    mPositionPoint = NULL;
    mPositionChanged = false;
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



TrackData::WaypointStatus TrackItemGeneralPage::newWaypointStatus() const
{
    if (mStatusCombo==NULL) return (TrackData::StatusInvalid);
							// not for this data
    if (!mStatusCombo->isEnabled()) return (TrackData::StatusInvalid);
							// not applicable
    int status = mStatusCombo->itemData(mStatusCombo->currentIndex()).toInt();
    return (static_cast<TrackData::WaypointStatus>(status));
}



QString TrackItemGeneralPage::newTimeZone() const
{							// only if editable
    if (mTimeZoneSel==NULL || !mTimeZoneSel->isEnabled()) return (QString::null);
    return (mTimeZoneSel->timeZone());
}



bool TrackItemGeneralPage::newPointPosition(double *newLat, double *newLon)
{
    if (!mPositionChanged) return (false);
    *newLat = mPositionLatitude;
    *newLon = mPositionLongitude;
    return (true);
}



void TrackItemGeneralPage::slotChangePosition()
{
    Q_ASSERT(mPositionPoint!=NULL);

    LatLongDialogue d(this);
    d.setLatLong(mPositionLatitude, mPositionLongitude);
    d.setButtonText(QDialogButtonBox::Ok, i18nc("@action:button", "Set"));

    if (d.exec())
    {
        mPositionChanged = true;
        mPositionLatitude = d.latitude();
        mPositionLongitude = d.longitude();
        emit pointPositionChanged(mPositionLatitude, mPositionLongitude);
    }
}



void TrackItemGeneralPage::addTimeSpanFields(const QList<TrackDataItem *> *items)
{
    TimeRange tsp = TrackData::unifyTimeSpans(items);
    TrackDataLabel *l = new TrackDataLabel(tsp.start(), this);
    mFormLayout->addRow(i18nc("@label:textbox", "Time start:"), l);
    disableIfEmpty(l);

    l = new TrackDataLabel(tsp.finish(), this);
    mFormLayout->addRow(i18nc("@label:textbox", "Time end:"), l);
    disableIfEmpty(l);
}



void TrackItemGeneralPage::addTypeField(const QList<TrackDataItem *> *items)
{
    mTypeCombo = new ItemTypeCombo(this);

    if (items->count()==1)
    {
        const TrackDataItem *tdi = items->first();
        Q_ASSERT(tdi!=NULL);

        mTypeCombo->setType(tdi->metadata("type"));
        connect(mTypeCombo, SIGNAL(currentIndexChanged(const QString &)), SLOT(slotDataChanged()));
        connect(mTypeCombo, SIGNAL(editTextChanged(const QString &)), SLOT(slotDataChanged()));
    }
    else mTypeCombo->setEnabled(false);

    mFormLayout->addRow(i18nc("@label:listbox", "Type:"), mTypeCombo);
}



void TrackItemGeneralPage::addStatusField(const QList<TrackDataItem *> *items)
{
    mStatusCombo = new QComboBox(this);
    mStatusCombo->setSizePolicy(QSizePolicy::Expanding, mStatusCombo->sizePolicy().verticalPolicy());

    mStatusCombo->addItem(QIcon::fromTheme("task-reject"), i18n("(None)"), TrackData::StatusNone);
    mStatusCombo->addItem(QIcon::fromTheme("task-ongoing"), i18n("To Do"), TrackData::StatusTodo);
    mStatusCombo->addItem(QIcon::fromTheme("task-complete"), i18n("Done"), TrackData::StatusDone);
    mStatusCombo->addItem(QIcon::fromTheme("dialog-warning"), i18n("Uncertain"), TrackData::StatusQuestion);

    if (items->count()>1)
    {
        mStatusCombo->addItem(QIcon::fromTheme("task-delegate"), i18n("(No change)"), TrackData::StatusInvalid);
        mStatusCombo->setCurrentIndex(mStatusCombo->count()-1);
    }
    else
    {
        const TrackDataItem *tdi = items->first();
        Q_ASSERT(tdi!=NULL);
        mStatusCombo->setCurrentIndex(tdi->metadata("status").toInt());
    }

    connect(mStatusCombo, SIGNAL(currentIndexChanged(const QString &)), SLOT(slotDataChanged()));

    mFormLayout->addRow(i18nc("@label:listbox", "Status:"), mStatusCombo);
}



void TrackItemGeneralPage::addDescField(const QList<TrackDataItem *> *items)
{
    mDescEdit = new KTextEdit(this);
    mDescEdit->setMaximumHeight(100);

    if (items->count()==1)
    {
        const TrackDataItem *tdi = items->first();
        Q_ASSERT(tdi!=NULL);

        mDescEdit->setAcceptRichText(false);
        mDescEdit->setTabChangesFocus(true);

        QString d = tdi->metadata("desc");
        if (!d.endsWith('\n')) d += "\n";
        mDescEdit->setPlainText(d);

        connect(mDescEdit, SIGNAL(textChanged()), SLOT(slotDataChanged()));
    }
    else mDescEdit->setEnabled(false);

    mFormLayout->addRow(i18nc("@label:listbox", "Description:"), mDescEdit);
}



void TrackItemGeneralPage::addPositionTimeFields(const QList<TrackDataItem *> *items)
{
    if (items->count()!=1) return;			// only for single selection

    TrackDataAbstractPoint *p = dynamic_cast<TrackDataAbstractPoint *>(items->first());
    Q_ASSERT(p!=NULL);
    mPositionPoint = p;
    mPositionLatitude = p->latitude();
    mPositionLongitude = p->longitude();

    QWidget *hb = new QWidget(this);
    QHBoxLayout *hlay = new QHBoxLayout(hb);
    hlay->setMargin(0);
    hlay->setSpacing(DialogBase::horizontalSpacing());
    TrackDataLabel *l = new TrackDataLabel(TrackData::formattedLatLong(mPositionLatitude, mPositionLongitude), this);
    hlay->addWidget(l);
    hlay->addStretch(1);
    mPositionLabel = l;

    QPushButton *b = new QPushButton(i18nc("@action:button", "Change..."), this);
    b->setToolTip(i18nc("@info:tooltip", "Change the latitude/longitude position"));
    connect(b, SIGNAL(clicked()), SLOT(slotChangePosition()));
    hb->setFocusProxy(b);
    hb->setFocusPolicy(Qt::StrongFocus);
    hlay->addWidget(b);
    mFormLayout->addRow(i18nc("@label:textbox", "Position:"), hb);

    l = new TrackDataLabel(p->time(), this);
    mFormLayout->addRow(i18nc("@label:textbox", "Time:"), l);
}



TrackFileGeneralPage::TrackFileGeneralPage(const QList<TrackDataItem *> *items, QWidget *pnt)
    : TrackItemGeneralPage(items, pnt)
{
    qDebug();
    setObjectName("TrackFileGeneralPage");

    mNameEdit->setReadOnly(true);			// can't rename here for files

    mUrlRequester = new QLineEdit(this);
    mUrlRequester->setReadOnly(true);
    connect(mUrlRequester, SIGNAL(textChanged(const QString &)), SLOT(slotDataChanged()));

    mTimeZoneSel = new TimeZoneSelector(this);
    connect(mTimeZoneSel, SIGNAL(zoneChanged(const QString &)), SLOT(slotDataChanged()));
    connect(mTimeZoneSel, SIGNAL(zoneChanged(const QString &)), SIGNAL(timeZoneChanged(const QString &)));

    if (items->count()==1)				// a single item
    {
        TrackDataFile *fileItem = dynamic_cast<TrackDataFile *>(items->first());
        Q_ASSERT(fileItem!=NULL);
        mUrlRequester->setText(fileItem->fileName().toDisplayString());

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

    addTimeSpanFields(items);
}



QString TrackFileGeneralPage::typeText(int count) const
{
    return (i18ncp("@item:intable", "<b>File</b>", "<b>%1 files</b>", count));
}



bool TrackFileGeneralPage::isDataValid() const
{
    if (mUrlRequester->isEnabled() && mUrlRequester->text().isEmpty()) return (false);
    return (TrackItemGeneralPage::isDataValid());
}



TrackTrackGeneralPage::TrackTrackGeneralPage(const QList<TrackDataItem *> *items, QWidget *pnt)
    : TrackItemGeneralPage(items, pnt)
{
    qDebug();
    setObjectName("TrackTrackGeneralPage");

    addTimeSpanFields(items);
    addSeparatorField();
    addTypeField(items);
    addDescField(items);
}



QString TrackTrackGeneralPage::typeText(int count) const
{
    return (i18ncp("@item:intable", "<b>Track</b>", "<b>%1 tracks</b>", count));
}



TrackSegmentGeneralPage::TrackSegmentGeneralPage(const QList<TrackDataItem *> *items, QWidget *pnt)
    : TrackItemGeneralPage(items, pnt)
{
    qDebug();
    setObjectName("TrackSegmentGeneralPage");

    addTimeSpanFields(items);
    addSeparatorField();
    addTypeField(items);
    addDescField(items);
}



QString TrackSegmentGeneralPage::typeText(int count) const
{
    return (i18ncp("@item:intable", "<b>Segment</b>", "<b>%1 segments</b>", count));
}



TrackTrackpointGeneralPage::TrackTrackpointGeneralPage(const QList<TrackDataItem *> *items, QWidget *pnt)
    : TrackItemGeneralPage(items, pnt)
{
    qDebug();
    setObjectName("TrackPointGeneralPage");

    addPositionTimeFields(items);
    if (items->count()>1) addTimeSpanFields(items);
}



QString TrackTrackpointGeneralPage::typeText(int count) const
{
    return (i18ncp("@item:intable", "<b>Point</b>", "<b>%1 points</b>", count));
}



TrackFolderGeneralPage::TrackFolderGeneralPage(const QList<TrackDataItem *> *items, QWidget *pnt)
    : TrackItemGeneralPage(items, pnt)
{
    qDebug();
    setObjectName("TrackFolderGeneralPage");
}



QString TrackFolderGeneralPage::typeText(int count) const
{
    return (i18ncp("@item:intable", "<b>Folder</b>", "<b>%1 folders</b>", count));
}



TrackWaypointGeneralPage::TrackWaypointGeneralPage(const QList<TrackDataItem *> *items, QWidget *pnt)
    : TrackItemGeneralPage(items, pnt)
{
    qDebug();
    setObjectName("TrackWaypointGeneralPage");

    mWaypoint = NULL;
    if (items->count()==1)
    {
        mWaypoint = dynamic_cast<const TrackDataWaypoint *>(items->first());
        Q_ASSERT(mWaypoint!=NULL);

        QString typeName;
        switch (mWaypoint->waypointType())
        {
case TrackData::WaypointNormal:		typeName = i18n("None");	break;
case TrackData::WaypointAudioNote:	typeName = i18n("Audio Note");	break;
case TrackData::WaypointVideoNote:	typeName = i18n("Video Note");	break;
case TrackData::WaypointPhoto:		typeName = i18n("Photo");	break;
case TrackData::WaypointStop:		typeName = i18n("Stop");	break;
default:				typeName = i18n("(Unknown)");	break;
        }

        QWidget *hb = new QWidget(this);
        QHBoxLayout *hlay = new QHBoxLayout(hb);
        hlay->setMargin(0);
        hlay->setSpacing(DialogBase::horizontalSpacing());

        QLabel *l = new QLabel(typeName, this);
        hlay->addWidget(l);
        hlay->addStretch(1);

        QPushButton *actionButton = NULL;
        switch (mWaypoint->waypointType())
        {
case TrackData::WaypointAudioNote:
            actionButton = new QPushButton(QIcon::fromTheme("media-playback-start"), QString::null, this);
            actionButton->setToolTip(i18nc("@info:tooltip", "Play the audio note"));
            connect(actionButton, SIGNAL(clicked()), SLOT(slotPlayAudioNote()));
            break;

case TrackData::WaypointVideoNote:
            actionButton = new QPushButton(QIcon::fromTheme("media-playback-start"), QString::null, this);
            actionButton->setToolTip(i18nc("@info:tooltip", "Play the video note"));
            connect(actionButton, SIGNAL(clicked()), SLOT(slotPlayVideoNote()));
            break;

case TrackData::WaypointPhoto:
            actionButton = new QPushButton(QIcon::fromTheme("document-preview"), QString::null, this);
            actionButton->setToolTip(i18nc("@info:tooltip", "View the photo"));
            connect(actionButton, SIGNAL(clicked()), SLOT(slotViewPhotoNote()));
            break;

default:    break;
        }

        if (actionButton!=NULL)				// action button is present
        {
            hb->setFocusProxy(actionButton);
            hb->setFocusPolicy(Qt::StrongFocus);
            hlay->addWidget(actionButton);
        }

        mFormLayout->addRow(i18nc("@label:textbox", "Type:"), hb);
        addSeparatorField();
    }
    else mWaypoint = NULL;

    addPositionTimeFields(items);
    if (items->count()>1) addTimeSpanFields(items);

    addSeparatorField();
    addStatusField(items);
    addDescField(items);
}


QString TrackWaypointGeneralPage::typeText(int count) const
{
    return (i18ncp("@item:intable", "<b>Waypoint</b>", "<b>%1 waypoints</b>", count));
}


void TrackWaypointGeneralPage::slotPlayAudioNote()
{
    MediaPlayer::playAudioNote(mWaypoint);
}


void TrackWaypointGeneralPage::slotPlayVideoNote()
{
    MediaPlayer::playVideoNote(mWaypoint);
}


void TrackWaypointGeneralPage::slotViewPhotoNote()
{
    MediaPlayer::viewPhotoNote(mWaypoint);
}




CREATE_PROPERTIES_PAGE(File, General);
CREATE_PROPERTIES_PAGE(Track, General);
CREATE_PROPERTIES_PAGE(Segment, General);
CREATE_PROPERTIES_PAGE(Trackpoint, General);
CREATE_PROPERTIES_PAGE(Folder, General);
CREATE_PROPERTIES_PAGE(Waypoint, General);
