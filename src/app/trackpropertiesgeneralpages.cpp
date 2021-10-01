//////////////////////////////////////////////////////////////////////////
//									//
//  Project:	Umbrail - GPX track viewer and editor			//
//									//
//////////////////////////////////////////////////////////////////////////
//									//
//  Copyright (c) 2014-2021 Jonathan Marten <jjm@keelhaul.me.uk>	//
//  Home and download page: <http://github.com/martenjj/umbrail>	//
//									//
//  This program is free software; you can redistribute it and/or	//
//  modify it under the terms of the GNU General Public License as	//
//  published by the Free Software Foundation, either version 3 of	//
//  the License or (at your option) any later version.			//
//									//
//  It is distributed in the hope that it will be useful, but		//
//  WITHOUT ANY WARRANTY;  without even the implied warranty of		//
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the	//
//  GNU General Public License for more details.			//
//									//
//  You should have received a copy of the GNU General Public License	//
//  along with this program;  see the file COPYING for further		//
//  details.  If not, see <http://gnu.org/licenses/gpl>.      		//
//									//
//////////////////////////////////////////////////////////////////////////

#include "trackpropertiesgeneralpages.h"

#include <qformlayout.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qdebug.h>
#include <qlineedit.h>

#include <klocalizedstring.h>
#include <kiconloader.h>
#include <ktextedit.h>

#include <kfdialog/dialogbase.h>

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
    else mNameEdit->setReadOnly(isReadOnly());
    connect(mNameEdit, &QLineEdit::textChanged, this, &TrackItemGeneralPage::slotNameChanged);

    addSeparatorField();
    mFormLayout->addRow(i18nc("@label:textbox", "Name:"), mNameEdit);
    addSeparatorField();
							// note whether named at start
    mHasExplicitName = (!items->isEmpty() && items->first()->hasExplicitName());

    mTypeCombo = nullptr;				// fields which may be created later
    mDescEdit = nullptr;
    mPositionLabel = nullptr;
    mTimeLabel = nullptr;
    mTimeStartLabel = mTimeEndLabel = nullptr;
}


bool TrackItemGeneralPage::isDataValid() const
{
    const QString &name = dataModel()->data("name").toString();
    qDebug() << "name" << name << "enabled?" << mNameEdit->isEnabled();
    if (!mNameEdit->isEnabled()) return (true);		// multiple items, entry ignored
    if (!mHasExplicitName) return (true);		// no explicit name, null allowed
    return (!name.isEmpty());				// name entered
}


void TrackItemGeneralPage::refreshData()
{
    mNameEdit->setPlaceholderText(i18n("Specify an item name..."));
    const QString &name = dataModel()->data("name").toString();
    if (!mHasExplicitName)				// no explicit name set
    {
        mNameEdit->setText("");
        if (!name.isEmpty()) mNameEdit->setPlaceholderText(name);
    }
    else						// item has explicit name
    {
        mNameEdit->setText(name);
    }

    if (mTypeCombo!=nullptr) mTypeCombo->setType(dataModel()->data("type").toString());

    if (mDescEdit!=nullptr && mDescEdit->isEnabled())
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


void TrackItemGeneralPage::slotNameChanged(const QString &text)
{
    if (!mNameEdit->isEnabled()) return;		// name is read only
    dataModel()->setData(DataIndexer::index("name"), text);
}


void TrackItemGeneralPage::slotTypeChanged(const QString &text)
{							// do not use 'text', it could be "none"
    dataModel()->setData(DataIndexer::index("type"), mTypeCombo->typeText());
}


void TrackItemGeneralPage::slotDescChanged()
{
    const QString desc = mDescEdit->toPlainText().trimmed();
    qDebug() << desc;
    dataModel()->setData(DataIndexer::index("desc"), desc);
}


void TrackItemGeneralPage::slotChangePosition()
{
    LatLongDialogue d(this);
    d.setLatLong(dataModel()->latitude(), dataModel()->longitude());
    d.setButtonText(QDialogButtonBox::Ok, i18nc("@action:button", "Set"));

    if (!d.exec()) return;

    dataModel()->setData(DataIndexer::index("latitude"), d.latitude());
    dataModel()->setData(DataIndexer::index("longitude"), d.longitude());
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
        Q_ASSERT(tdi!=nullptr);

        mTypeCombo->setEnabled(!isReadOnly());
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
        mDescEdit->setReadOnly(isReadOnly());
        connect(mDescEdit, &KTextEdit::textChanged, this, &TrackItemGeneralPage::slotDescChanged);
    }
    else mDescEdit->setEnabled(false);

    // This is necessary in order that the other data fields (e.g. time or time span)
    // do not get stretched vertically, which puts the labels out of line with
    // their corresponding data fields.
    mDescEdit->setSizePolicy(mDescEdit->sizePolicy().horizontalPolicy(), QSizePolicy::Minimum);

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
    b->setEnabled(!isReadOnly());
    connect(b, &QAbstractButton::clicked, this, &TrackItemGeneralPage::slotChangePosition);
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
        Q_ASSERT(fileItem!=nullptr);
        mUrlRequester->setText(fileItem->fileName().toDisplayString());
        mTimeZoneSel->setItems(items);			// use these to get timezone

        // The time zone is allowed to be changed even if the file
        // is read only.  This case is handled specially in FilesController.
        mTimeZoneSel->setEnabled(!isReadOnly() || filesController()->isSettingTimeZone());
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


void TrackFileGeneralPage::refreshData()
{
    TrackItemGeneralPage::refreshData();

    if (mTimeZoneSel!=nullptr)
    {
        const QString zoneName = dataModel()->data("timezone").toString();
        mTimeZoneSel->setTimeZone(zoneName);
    }
}


void TrackFileGeneralPage::slotTimeZoneChanged(const QString &zoneName)
{
    dataModel()->setData(DataIndexer::index("timezone"), zoneName);
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

    mWaypoint = nullptr;
    mStatusCombo = nullptr;

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

        QPushButton *actionButton = nullptr;
        switch (mWaypoint->waypointType())
        {
case TrackData::WaypointAudioNote:
            actionButton = new QPushButton(QIcon::fromTheme("media-playback-start"), "", this);
            actionButton->setToolTip(i18nc("@info:tooltip", "Play the audio note"));
            connect(actionButton, &QAbstractButton::clicked, this, &TrackWaypointGeneralPage::slotPlayAudioNote);
            break;

case TrackData::WaypointVideoNote:
            actionButton = new QPushButton(QIcon::fromTheme("media-playback-start"), "", this);
            actionButton->setToolTip(i18nc("@info:tooltip", "Play the video note"));
            connect(actionButton, &QAbstractButton::clicked, this, &TrackWaypointGeneralPage::slotPlayVideoNote);
            break;

case TrackData::WaypointPhoto:
            actionButton = new QPushButton(QIcon::fromTheme("document-preview"), "", this);
            actionButton->setToolTip(i18nc("@info:tooltip", "View the photo"));
            connect(actionButton, &QAbstractButton::clicked, this, &TrackWaypointGeneralPage::slotViewPhotoNote);
            break;

default:    break;
        }

        if (actionButton!=nullptr)			// action button is present
        {
            // Synchronise with shortcut for "track_play_media" in MainWindow::setupActions()
            actionButton->setShortcut(Qt::CTRL+Qt::Key_P);

            hb->setFocusProxy(actionButton);
            hb->setFocusPolicy(Qt::StrongFocus);
            hlay->addWidget(actionButton);
        }

        mFormLayout->addRow(i18nc("@label:textbox", "Media:"), hb);

        addSeparatorField();
    }

    addPositionFields(items);
    addTimeField(items);
    if (mWaypoint==nullptr) addTimeSpanFields(items);
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

    mStatusCombo->setCurrentIndex(dataModel()->data("status").toInt());
}


void TrackWaypointGeneralPage::addStatusField(const QList<TrackDataItem *> *items)
{
    mStatusCombo = new QComboBox(this);
    mStatusCombo->setSizePolicy(QSizePolicy::Expanding, mStatusCombo->sizePolicy().verticalPolicy());

    mStatusCombo->addItem(QIcon::fromTheme("unknown"), TrackData::formattedWaypointStatus(TrackData::StatusNone), TrackData::StatusNone);
    mStatusCombo->addItem(QIcon::fromTheme("task-ongoing"), TrackData::formattedWaypointStatus(TrackData::StatusTodo), TrackData::StatusTodo);
    mStatusCombo->addItem(QIcon::fromTheme("task-complete"), TrackData::formattedWaypointStatus(TrackData::StatusDone), TrackData::StatusDone);
    mStatusCombo->addItem(QIcon::fromTheme("task-attempt"), TrackData::formattedWaypointStatus(TrackData::StatusQuestion), TrackData::StatusQuestion);
    mStatusCombo->addItem(QIcon::fromTheme("task-reject"), TrackData::formattedWaypointStatus(TrackData::StatusUnwanted), TrackData::StatusUnwanted);

    if (items->count()>1)
    {
        mStatusCombo->addItem(QIcon::fromTheme("task-delegate"), i18n("(No change)"), TrackData::StatusInvalid);
        mStatusCombo->setCurrentIndex(mStatusCombo->count()-1);
    }
    mStatusCombo->setEnabled(!isReadOnly());

    connect(mStatusCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &TrackWaypointGeneralPage::slotStatusChanged);
    mFormLayout->addRow(i18nc("@label:listbox", "Status:"), mStatusCombo);
}


void TrackWaypointGeneralPage::slotStatusChanged(int idx)
{
    qDebug() << idx;

    const int status = mStatusCombo->itemData(idx).toInt();
    dataModel()->setData(DataIndexer::index("status"), (status==0 ? QVariant() : status));
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
