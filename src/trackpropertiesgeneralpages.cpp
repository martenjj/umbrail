
#include "trackpropertiesgeneralpages.h"

#include <qformlayout.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qdebug.h>
#include <qlineedit.h>

#include <klocalizedstring.h>
#include <kiconloader.h>
#include <ktextedit.h>

#include <dialogbase.h>

#include "trackdata.h"
#include "trackdatalabel.h"
#include "filescontroller.h"
#include "variableunitdisplay.h"
#include "itemtypecombo.h"
#include "timezoneselector.h"
#include "latlongdialogue.h"
#include "mediaplayer.h"
#include "metadatamodel.h"
#include "dataindexer.h"

//////////////////////////////////////////////////////////////////////////
//									//
//  TrackItemGeneralPage						//
//									//
//////////////////////////////////////////////////////////////////////////

TrackItemGeneralPage::TrackItemGeneralPage(const QList<TrackDataItem *> *items, QWidget *pnt)
    : TrackPropertiesPage(items, pnt)
{
    setObjectName("TrackItemGeneralPage");

    mNameEdit = new QLineEdit(this);
    if (items->count()>1) mNameEdit->setEnabled(false);
    connect(mNameEdit, &QLineEdit::textChanged, this, &TrackItemGeneralPage::slotNameChanged);

    addSeparatorField();
    mFormLayout->addRow(i18nc("@label:textbox", "Name:"), mNameEdit);
    addSeparatorField();

    mTypeCombo = nullptr;				// fields which may be created later
    mDescEdit = nullptr;
    mPositionLabel = nullptr;
    mTimeLabel = nullptr;
    mTimeStartLabel = mTimeEndLabel = nullptr;
}


bool TrackItemGeneralPage::isDataValid() const
{
    const QVariant &name = dataModel()->data("name");
    qDebug() << "name" << name << "enabled?" << mNameEdit->isEnabled();
    if (!mNameEdit->isEnabled()) return (true);
    return (!name.isNull());
}


void TrackItemGeneralPage::refreshData()
{
    qDebug();

    mNameEdit->setText(dataModel()->data("name").toString());
    if (mTypeCombo!=nullptr) mTypeCombo->setType(dataModel()->data("type").toString());

    if (mDescEdit!=nullptr)
    {
        QString desc = dataModel()->data("desc").toString();
        if (!desc.isEmpty() && !desc.endsWith('\n')) desc += '\n';
        mDescEdit->setPlainText(desc);
    }

    if (mPositionLabel!=nullptr)
    {
        const QString pos = TrackData::formattedLatLong(dataModel()->latitude(), dataModel()->longitude());
        mPositionLabel->setText(pos);
    }

    const QTimeZone *tz = dataModel()->timeZone();
    if (mTimeLabel!=nullptr)
    {
        mTimeLabel->setDateTime(dataModel()->data("time").toDateTime());
        mTimeLabel->setTimeZone(tz);
    }

    if (mTimeStartLabel!=nullptr) mTimeStartLabel->setTimeZone(tz);
    if (mTimeEndLabel!=nullptr) mTimeEndLabel->setTimeZone(tz);
}


// QString TrackItemGeneralPage::newItemName() const
// {							// only if editable
    // if (!mNameEdit->isEnabled()) return (QString());
    // return (mNameEdit->text());
// }


// QString TrackItemGeneralPage::newItemDesc() const
// {							// only if editable
    // if (mDescEdit==NULL) return ("-");			// not for this data
    // if (!mDescEdit->isEnabled()) return ("-");		// not applicable
    // return (mDescEdit->toPlainText().trimmed());
// }



// QString TrackItemGeneralPage::newTrackType() const
// {
    // if (mTypeCombo==NULL) return ("-");			// not for this data
    // if (!mTypeCombo->isEnabled()) return ("-");		// not applicable
// 
    // int idx = mTypeCombo->currentIndex();
    // if (idx==0) return ("");				// first is always "none"
    // return (mTypeCombo->currentText());
// }



// TrackData::WaypointStatus TrackItemGeneralPage::newWaypointStatus() const
// {
    // if (mStatusCombo==NULL) return (TrackData::StatusInvalid);
							// // not for this data
    // if (!mStatusCombo->isEnabled()) return (TrackData::StatusInvalid);
							// // not applicable
    // int status = mStatusCombo->itemData(mStatusCombo->currentIndex()).toInt();
    // return (static_cast<TrackData::WaypointStatus>(status));
// }



// QString TrackItemGeneralPage::newTimeZone() const
// {							// only if editable
//     if (mTimeZoneSel==NULL || !mTimeZoneSel->isEnabled()) return (QString());
//     return (mTimeZoneSel->timeZone());
// }



// bool TrackItemGeneralPage::newPointPosition(double *newLat, double *newLon)
// {
    // if (!mPositionChanged) return (false);
    // *newLat = mPositionLatitude;
    // *newLon = mPositionLongitude;
    // return (true);
// }


void TrackItemGeneralPage::slotNameChanged(const QString &text)
{

    if (!mNameEdit->isEnabled()) return;		// name is read only
    dataModel()->setData(DataIndexer::self()->index("name"), text);
}



void TrackItemGeneralPage::slotTypeChanged(const QString &text)
{							// do not use 'text', it could be "none"
    dataModel()->setData(DataIndexer::self()->index("type"), mTypeCombo->typeText());
}


void TrackItemGeneralPage::slotDescChanged()
{
    const QString desc = mDescEdit->toPlainText().trimmed();
    qDebug() << desc;
    dataModel()->setData(DataIndexer::self()->index("desc"), desc);
}


void TrackItemGeneralPage::slotChangePosition()
{
    LatLongDialogue d(this);
    d.setLatLong(dataModel()->latitude(), dataModel()->longitude());
    d.setButtonText(QDialogButtonBox::Ok, i18nc("@action:button", "Set"));

    if (!d.exec()) return;

    dataModel()->setData(DataIndexer::self()->index("latitude"), d.latitude());
    dataModel()->setData(DataIndexer::self()->index("longitude"), d.longitude());
    refreshData();					// update the position display
}


void TrackItemGeneralPage::addTimeSpanFields(const QList<TrackDataItem *> *items)
{
    // Time start/end cannot change with container or multiple item
    // metadata, but the time zone can.

    TimeRange tsp = TrackData::unifyTimeSpans(items);
    mTimeStartLabel = new TrackDataLabel(tsp.start(), this);
    mFormLayout->addRow(i18nc("@label:textbox", "Time start:"), mTimeStartLabel);
    disableIfEmpty(mTimeStartLabel);

    mTimeEndLabel = new TrackDataLabel(tsp.finish(), this);
    mFormLayout->addRow(i18nc("@label:textbox", "Time end:"), mTimeEndLabel);
    disableIfEmpty(mTimeEndLabel);
}


void TrackItemGeneralPage::addTypeField(const QList<TrackDataItem *> *items)
{
    mTypeCombo = new ItemTypeCombo(this);

    if (items->count()==1)
    {
        const TrackDataItem *tdi = items->first();
        Q_ASSERT(tdi!=NULL);

        // TODO: are both needed?
        connect(mTypeCombo, &QComboBox::currentTextChanged, this, &TrackItemGeneralPage::slotTypeChanged);
        connect(mTypeCombo, &QComboBox::editTextChanged, this, &TrackItemGeneralPage::slotTypeChanged);
    }
    else mTypeCombo->setEnabled(false);

    mFormLayout->addRow(i18nc("@label:listbox", "Type:"), mTypeCombo);
}


void TrackItemGeneralPage::addDescField(const QList<TrackDataItem *> *items)
{
    mDescEdit = new KTextEdit(this);
    mDescEdit->setMaximumHeight(100);

    if (items->count()==1)
    {
        mDescEdit->setAcceptRichText(false);
        mDescEdit->setTabChangesFocus(true);
        connect(mDescEdit, &KTextEdit::textChanged, this, &TrackItemGeneralPage::slotDescChanged);
    }
    else mDescEdit->setEnabled(false);

    mFormLayout->addRow(i18nc("@label:listbox", "Description:"), mDescEdit);
}


void TrackItemGeneralPage::addPositionFields(const QList<TrackDataItem *> *items)
{
    if (items->count()!=1) return;			// only for single selection

    QWidget *hb = new QWidget(this);
    QHBoxLayout *hlay = new QHBoxLayout(hb);
    hlay->setMargin(0);
    hlay->setSpacing(DialogBase::horizontalSpacing());
    mPositionLabel = new TrackDataLabel(QString(), this);
    hlay->addWidget(mPositionLabel);
    hlay->addStretch(1);

    QPushButton *b = new QPushButton(i18nc("@action:button", "Change..."), this);
    b->setToolTip(i18nc("@info:tooltip", "Change the latitude/longitude position"));
    connect(b, SIGNAL(clicked()), SLOT(slotChangePosition()));
    hb->setFocusProxy(b);
    hb->setFocusPolicy(Qt::StrongFocus);
    hlay->addWidget(b);
    mFormLayout->addRow(i18nc("@label:textbox", "Position:"), hb);
}


void TrackItemGeneralPage::addTimeField(const QList<TrackDataItem *> *items)
{
    if (items->count()!=1) return;			// only for single selection

    mTimeLabel = new TrackDataLabel(QDateTime(), this);
    mFormLayout->addRow(i18nc("@label:textbox", "Time:"), mTimeLabel);
    // cannot change in GUI, so no change slot needed
}

//////////////////////////////////////////////////////////////////////////
//									//
//  TrackFileGeneralPage						//
//									//
//////////////////////////////////////////////////////////////////////////

TrackFileGeneralPage::TrackFileGeneralPage(const QList<TrackDataItem *> *items, QWidget *pnt)
    : TrackItemGeneralPage(items, pnt)
{
    setObjectName("TrackFileGeneralPage");

    mNameEdit->setReadOnly(true);			// can't rename here for files

    mUrlRequester = new QLineEdit(this);
    mUrlRequester->setReadOnly(true);

    mTimeZoneSel = new TimeZoneSelector(this);
    connect(mTimeZoneSel, &TimeZoneSelector::zoneChanged, this, &TrackFileGeneralPage::slotTimeZoneChanged);

    if (items->count()==1)				// a single item
    {
        TrackDataFile *fileItem = dynamic_cast<TrackDataFile *>(items->first());
        Q_ASSERT(fileItem!=NULL);
        mUrlRequester->setText(fileItem->fileName().toDisplayString());
        mTimeZoneSel->setItems(items);			// use these to get timezone
    }
    else						// may be mixed MIME types
    {
        mUrlRequester->setEnabled(false);		// can't edit for multiple items
        mTimeZoneSel->setEnabled(false);
    }

    mFormLayout->insertRow(mFormLayout->rowCount()-1, i18nc("@label:textbox", "File:"), mUrlRequester);
    mFormLayout->addRow(i18nc("@label:textbox", "Time zone:"), mTimeZoneSel);
    addSeparatorField();

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



// TODO: all of these that are empty can be eliminated
void TrackFileGeneralPage::refreshData()
{
    qDebug();

    TrackItemGeneralPage::refreshData();

    if (mTimeZoneSel!=nullptr)
    {
        const QString zoneName = dataModel()->data("timezone").toString();
        mTimeZoneSel->setTimeZone(zoneName);
    }
}


void TrackFileGeneralPage::slotTimeZoneChanged(const QString &zoneName)
{
    dataModel()->setData(DataIndexer::self()->index("timezone"), zoneName);
    refreshData();					// update times on this page
}

//////////////////////////////////////////////////////////////////////////
//									//
//  TrackTrackGeneralPage						//
//									//
//////////////////////////////////////////////////////////////////////////

TrackTrackGeneralPage::TrackTrackGeneralPage(const QList<TrackDataItem *> *items, QWidget *pnt)
    : TrackItemGeneralPage(items, pnt)
{
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


void TrackTrackGeneralPage::refreshData()
{
    TrackItemGeneralPage::refreshData();
    qDebug();
}

//////////////////////////////////////////////////////////////////////////
//									//
//  TrackSegmentGeneralPage						//
//									//
//////////////////////////////////////////////////////////////////////////

TrackSegmentGeneralPage::TrackSegmentGeneralPage(const QList<TrackDataItem *> *items, QWidget *pnt)
    : TrackItemGeneralPage(items, pnt)
{
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


void TrackSegmentGeneralPage::refreshData()
{
    TrackItemGeneralPage::refreshData();
    qDebug();
}

//////////////////////////////////////////////////////////////////////////
//									//
//  TrackTrackpointGeneralPage						//
//									//
//////////////////////////////////////////////////////////////////////////

TrackTrackpointGeneralPage::TrackTrackpointGeneralPage(const QList<TrackDataItem *> *items, QWidget *pnt)
    : TrackItemGeneralPage(items, pnt)
{
    setObjectName("TrackTrackpointGeneralPage");

    addPositionFields(items);
    addTimeField(items);
    if (items->count()>1) addTimeSpanFields(items);
}


QString TrackTrackpointGeneralPage::typeText(int count) const
{
    return (i18ncp("@item:intable", "<b>Point</b>", "<b>%1 points</b>", count));
}


void TrackTrackpointGeneralPage::refreshData()
{
    TrackItemGeneralPage::refreshData();
    qDebug();
}

//////////////////////////////////////////////////////////////////////////
//									//
//  TrackFolderGeneralPage						//
//									//
//////////////////////////////////////////////////////////////////////////

TrackFolderGeneralPage::TrackFolderGeneralPage(const QList<TrackDataItem *> *items, QWidget *pnt)
    : TrackItemGeneralPage(items, pnt)
{
    setObjectName("TrackFolderGeneralPage");
}


void TrackFolderGeneralPage::refreshData()
{
    TrackItemGeneralPage::refreshData();
    qDebug();
}


QString TrackFolderGeneralPage::typeText(int count) const
{
    return (i18ncp("@item:intable", "<b>Folder</b>", "<b>%1 folders</b>", count));
}

//////////////////////////////////////////////////////////////////////////
//									//
//  TrackWaypointGeneralPage						//
//									//
//////////////////////////////////////////////////////////////////////////

TrackWaypointGeneralPage::TrackWaypointGeneralPage(const QList<TrackDataItem *> *items, QWidget *pnt)
    : TrackItemGeneralPage(items, pnt)
{
    setObjectName("TrackWaypointGeneralPage");

    mWaypoint = NULL;
    mStatusCombo = NULL;

    if (items->count()==1)
    {
        // TODO: The "Media" field and the media that is output when the play
        // button is pressed is not automatically updated from the metadata.

        mWaypoint = dynamic_cast<const TrackDataWaypoint *>(items->first());
        Q_ASSERT(mWaypoint!=nullptr);

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
            actionButton = new QPushButton(QIcon::fromTheme("media-playback-start"), "", this);
            actionButton->setToolTip(i18nc("@info:tooltip", "Play the audio note"));
            connect(actionButton, SIGNAL(clicked()), SLOT(slotPlayAudioNote()));
            break;

case TrackData::WaypointVideoNote:
            actionButton = new QPushButton(QIcon::fromTheme("media-playback-start"), "", this);
            actionButton->setToolTip(i18nc("@info:tooltip", "Play the video note"));
            connect(actionButton, SIGNAL(clicked()), SLOT(slotPlayVideoNote()));
            break;

case TrackData::WaypointPhoto:
            actionButton = new QPushButton(QIcon::fromTheme("document-preview"), "", this);
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

        mFormLayout->addRow(i18nc("@label:textbox", "Media:"), hb);

        addSeparatorField();
    }

    addPositionFields(items);
    addTimeField(items);
    if (mWaypoint==NULL) addTimeSpanFields(items);
    addSeparatorField();

    addStatusField(items);
    addDescField(items);
}


QString TrackWaypointGeneralPage::typeText(int count) const
{
    return (i18ncp("@item:intable", "<b>Waypoint</b>", "<b>%1 waypoints</b>", count));
}


void TrackWaypointGeneralPage::refreshData()
{
    TrackItemGeneralPage::refreshData();

    qDebug() << "status" << dataModel()->data("status") << "=" << dataModel()->data("status").toInt();
    mStatusCombo->setCurrentIndex(dataModel()->data("status").toInt());
}


void TrackWaypointGeneralPage::addStatusField(const QList<TrackDataItem *> *items)
{
    mStatusCombo = new QComboBox(this);
    mStatusCombo->setSizePolicy(QSizePolicy::Expanding, mStatusCombo->sizePolicy().verticalPolicy());

    mStatusCombo->addItem(QIcon::fromTheme("unknown"), i18n("(None)"), TrackData::StatusNone);
    mStatusCombo->addItem(QIcon::fromTheme("task-ongoing"), i18n("To Do"), TrackData::StatusTodo);
    mStatusCombo->addItem(QIcon::fromTheme("task-complete"), i18n("Done"), TrackData::StatusDone);
    mStatusCombo->addItem(QIcon::fromTheme("dialog-warning"), i18n("Uncertain"), TrackData::StatusQuestion);
    mStatusCombo->addItem(QIcon::fromTheme("task-reject"), i18n("Unwanted"), TrackData::StatusUnwanted);

    if (items->count()>1)
    {
        mStatusCombo->addItem(QIcon::fromTheme("task-delegate"), i18n("(No change)"), TrackData::StatusInvalid);
        mStatusCombo->setCurrentIndex(mStatusCombo->count()-1);
    }

    connect(mStatusCombo, SIGNAL(currentIndexChanged(int)), SLOT(slotStatusChanged(int)));
    mFormLayout->addRow(i18nc("@label:listbox", "Status:"), mStatusCombo);
}


void TrackWaypointGeneralPage::slotStatusChanged(int idx)
{
    qDebug() << idx;

    const int status = mStatusCombo->itemData(idx).toInt();
// TODO: check what happens for multiple points and (no change) selected
//    const TrackData::WaypointStatus wps = static_cast<TrackData::WaypointStatus>(status);
//    if (wps==TrackData::StatusInvalid) return;		// no change

    dataModel()->setData(DataIndexer::self()->index("status"), (status==0 ? QVariant() : status));
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

//////////////////////////////////////////////////////////////////////////
//									//
//  TrackRouteGeneralPage						//
//									//
//////////////////////////////////////////////////////////////////////////

TrackRouteGeneralPage::TrackRouteGeneralPage(const QList<TrackDataItem *> *items, QWidget *pnt)
    : TrackItemGeneralPage(items, pnt)
{
    setObjectName("TrackRouteGeneralPage");

    addSeparatorField();
    addTypeField(items);
    addDescField(items);
}


QString TrackRouteGeneralPage::typeText(int count) const
{
    return (i18ncp("@item:intable", "<b>Route</b>", "<b>%1 routes</b>", count));
}


void TrackRouteGeneralPage::refreshData()
{
    TrackItemGeneralPage::refreshData();
    qDebug();
}

//////////////////////////////////////////////////////////////////////////
//									//
//  TrackRoutepointGeneralPage						//
//									//
//////////////////////////////////////////////////////////////////////////

TrackRoutepointGeneralPage::TrackRoutepointGeneralPage(const QList<TrackDataItem *> *items, QWidget *pnt)
    : TrackItemGeneralPage(items, pnt)
{
    setObjectName("TrackRoutepointGeneralPage");

    addPositionFields(items);
}


QString TrackRoutepointGeneralPage::typeText(int count) const
{
    return (i18ncp("@item:intable", "<b>Route point</b>", "<b>%1 route points</b>", count));
}


void TrackRoutepointGeneralPage::refreshData()
{
    TrackItemGeneralPage::refreshData();
    qDebug();
}

//////////////////////////////////////////////////////////////////////////
//									//
//  Page creation interface						//
//									//
//////////////////////////////////////////////////////////////////////////

CREATE_PROPERTIES_PAGE(File, General)
CREATE_PROPERTIES_PAGE(Track, General)
CREATE_PROPERTIES_PAGE(Segment, General)
CREATE_PROPERTIES_PAGE(Trackpoint, General)
CREATE_PROPERTIES_PAGE(Folder, General)
CREATE_PROPERTIES_PAGE(Waypoint, General)
CREATE_PROPERTIES_PAGE(Route, General)
CREATE_PROPERTIES_PAGE(Routepoint, General)
