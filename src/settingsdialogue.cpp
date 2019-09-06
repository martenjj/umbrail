
#include "settingsdialogue.h"

#include <qformlayout.h>
#include <qdialogbuttonbox.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qgroupbox.h>
#include <qspinbox.h>
#include <qdebug.h>

#include <klocalizedstring.h>
#include <kpagedialog.h>
#include <kcolorbutton.h>
#include <kurlrequester.h>
#include <kconfigskeleton.h>
#include <kservice.h>
#include <kmimetypetrader.h>

#include <dialogbase.h>
#include <dialogstatesaver.h>

#include "settings.h"
#include "filescontroller.h"

//////////////////////////////////////////////////////////////////////////
//									//
//  SettingsDialogue							//
//									//
//////////////////////////////////////////////////////////////////////////

SettingsDialogue::SettingsDialogue(QWidget *pnt)
    : KPageDialog(pnt)
{
    setObjectName("SettingsDialogue");

    setModal(true);
    buttonBox()->setStandardButtons(QDialogButtonBox::Ok|QDialogButtonBox::Cancel|QDialogButtonBox::RestoreDefaults);
    buttonBox()->button(QDialogButtonBox::RestoreDefaults)->setIcon(QIcon::fromTheme("edit-undo"));

    setFaceType(KPageDialog::Auto);

    KPageWidgetItem *page = new SettingsMapStylePage(this);
    connect(buttonBox(), SIGNAL(accepted()), page, SLOT(slotSave()));
    connect(buttonBox()->button(QDialogButtonBox::RestoreDefaults), SIGNAL(clicked()), page, SLOT(slotDefaults()));
    addPage(page);

    page = new SettingsFilesPage(this);
    connect(buttonBox(), SIGNAL(accepted()), page, SLOT(slotSave()));
    connect(buttonBox()->button(QDialogButtonBox::RestoreDefaults), SIGNAL(clicked()), page, SLOT(slotDefaults()));
    addPage(page);

    page = new SettingsMediaPage(this);
    connect(buttonBox(), SIGNAL(accepted()), page, SLOT(slotSave()));
    connect(buttonBox()->button(QDialogButtonBox::RestoreDefaults), SIGNAL(clicked()), page, SLOT(slotDefaults()));
    addPage(page);

    setMinimumSize(440, 360);
    new DialogStateSaver(this);
}

//////////////////////////////////////////////////////////////////////////
//									//
//  SettingsMapStylePage						//
//									//
//////////////////////////////////////////////////////////////////////////

SettingsMapStylePage::SettingsMapStylePage(QWidget *pnt)
    : KPageWidgetItem(new QWidget(pnt))
{
    setName(i18nc("@title:tab", "Map"));
    setHeader(i18n("Settings for the map display"));
    setIcon(QIcon::fromTheme("marble"));

    QWidget *w = widget();
    QFormLayout *fl = new QFormLayout(w);

    const KConfigSkeletonItem *kcsi = Settings::self()->lineColourItem();
    mLineColourButton = new KColorButton(Settings::lineColour(), w);
    mLineColourButton->setAlphaChannelEnabled(false);
    mLineColourButton->setToolTip(kcsi->toolTip());
    fl->addRow(kcsi->label(), mLineColourButton);

    kcsi = Settings::self()->pointColourItem();
    mPointColourButton = new KColorButton(Settings::pointColour(), w);
    mPointColourButton->setAlphaChannelEnabled(false);
    mPointColourButton->setToolTip(kcsi->toolTip());
    fl->addRow(kcsi->label(), mPointColourButton);

    kcsi = Settings::self()->showTrackArrowsItem();
    mShowTrackArrowsCheck = new QCheckBox(kcsi->label(), w);
    mShowTrackArrowsCheck->setChecked(Settings::showTrackArrows());
    connect(mShowTrackArrowsCheck, SIGNAL(toggled(bool)), SLOT(slotItemChanged()));
    mShowTrackArrowsCheck->setToolTip(kcsi->toolTip());
    fl->addRow("", mShowTrackArrowsCheck);

    fl->addItem(DialogBase::verticalSpacerItem());

    kcsi = Settings::self()->selectedUseSystemColoursItem();
    mSelectedUseSystemCheck = new QCheckBox(kcsi->label(), w);
    mSelectedUseSystemCheck->setChecked(Settings::selectedUseSystemColours());
    connect(mSelectedUseSystemCheck, SIGNAL(toggled(bool)), SLOT(slotItemChanged()));
    mSelectedUseSystemCheck->setToolTip(kcsi->toolTip());
    fl->addRow(i18n("Selection:"), mSelectedUseSystemCheck);

    mSelectedOuterButton = new KColorButton(Settings::selectedMarkOuter(), w);
    mSelectedOuterButton->setAlphaChannelEnabled(false);
    kcsi = Settings::self()->selectedMarkOuterItem();
    mSelectedOuterButton->setToolTip(kcsi->toolTip());
    fl->addRow(kcsi->label(), mSelectedOuterButton);

    mSelectedInnerButton = new KColorButton(Settings::selectedMarkInner(), w);
    mSelectedInnerButton->setAlphaChannelEnabled(false);
    kcsi = Settings::self()->selectedMarkInnerItem();
    mSelectedInnerButton->setToolTip(kcsi->toolTip());
    fl->addRow(kcsi->label(), mSelectedInnerButton);

    slotItemChanged();
}


void SettingsMapStylePage::slotSave()
{
    Settings::setLineColour(mLineColourButton->color());
    Settings::setPointColour(mPointColourButton->color());
    Settings::setSelectedMarkOuter(mSelectedOuterButton->color());
    Settings::setSelectedMarkInner(mSelectedInnerButton->color());
    Settings::setSelectedUseSystemColours(mSelectedUseSystemCheck->isChecked());
    Settings::setShowTrackArrows(mShowTrackArrowsCheck->isChecked());
}


void SettingsMapStylePage::slotDefaults()
{
    KConfigSkeletonItem *kcsi = Settings::self()->lineColourItem();
    kcsi->setDefault();
    mLineColourButton->setColor(Settings::lineColour());
    kcsi = Settings::self()->pointColourItem();
    kcsi->setDefault();
    mPointColourButton->setColor(Settings::pointColour());

    kcsi = Settings::self()->showTrackArrowsItem();
    kcsi->setDefault();
    mShowTrackArrowsCheck->setChecked(Settings::showTrackArrows());

    kcsi = Settings::self()->selectedUseSystemColoursItem();
    kcsi->setDefault();
    mSelectedUseSystemCheck->setChecked(Settings::selectedUseSystemColours());

    kcsi = Settings::self()->selectedMarkOuterItem();
    kcsi->setDefault();
    mSelectedOuterButton->setColor(Settings::selectedMarkOuter());

    kcsi = Settings::self()->selectedMarkInnerItem();
    kcsi->setDefault();
    mSelectedInnerButton->setColor(Settings::selectedMarkInner());

    slotItemChanged();
}


void SettingsMapStylePage::slotItemChanged()
{
    bool syscol = mSelectedUseSystemCheck->isChecked();
    mSelectedOuterButton->setEnabled(!syscol);
    mSelectedInnerButton->setEnabled(!syscol);
}

//////////////////////////////////////////////////////////////////////////
//									//
//  SettingsFilesPage							//
//									//
//////////////////////////////////////////////////////////////////////////

SettingsFilesPage::SettingsFilesPage(QWidget *pnt)
    : KPageWidgetItem(new QWidget(pnt))
{
    setName(i18nc("@title:tab", "Files"));
    setHeader(i18n("Settings for files and locations"));
    setIcon(QIcon::fromTheme("folder"));

    QWidget *w = widget();
    QFormLayout *fl = new QFormLayout(w);

    const KConfigSkeletonItem *ski = Settings::self()->generalGroupTitleItem();
    Q_ASSERT(ski!=nullptr);
    QGroupBox *g = new QGroupBox(ski->label(), w);
    g->setFlat(true);
    fl->addRow(g);

    ski = Settings::self()->fileCheckTimezoneItem();
    Q_ASSERT(ski!=nullptr);

    mTimezoneCheck = new QCheckBox(ski->label(), w);
    mTimezoneCheck->setToolTip(ski->toolTip());
    mTimezoneCheck->setChecked(Settings::fileCheckTimezone());
    fl->addRow(mTimezoneCheck);

    QHBoxLayout *lay = new QHBoxLayout;
    lay->addStretch(1);

    fl->addItem(DialogBase::verticalSpacerItem());

    QPushButton *but = new QPushButton(QIcon::fromTheme("edit-clear"), i18n("Clear all file warnings"), w);
    but->setToolTip(i18n("Clear all remembered file questions and warnings."));

    connect(but, SIGNAL(clicked()), SLOT(slotClearFileWarnings()));
    lay->addWidget(but);
    fl->addRow(lay);

    fl->addItem(DialogBase::verticalSpacerItem());

    ski = Settings::self()->pathsGroupTitleItem();
    Q_ASSERT(ski!=nullptr);
    g = new QGroupBox(ski->label(), w);
    g->setFlat(true);
    fl->addRow(g);

    ski = Settings::self()->audioNotesDirectoryItem();
    Q_ASSERT(ski!=nullptr);
    mAudioNotesRequester = new KUrlRequester(w);
    mAudioNotesRequester->setMode(KFile::Directory|KFile::ExistingOnly);
    mAudioNotesRequester->setUrl(Settings::audioNotesDirectory());
    mAudioNotesRequester->setToolTip(ski->toolTip());
    fl->addRow(ski->label(), mAudioNotesRequester);

    slotItemChanged();
}


void SettingsFilesPage::slotSave()
{
    Settings::setFileCheckTimezone(mTimezoneCheck->isChecked());

    QUrl u = mAudioNotesRequester->url().adjusted(QUrl::StripTrailingSlash);
    u.setPath(u.path()+'/');
    Settings::setAudioNotesDirectory(u);
}


void SettingsFilesPage::slotDefaults()
{
    KConfigSkeletonItem *kcsi = Settings::self()->fileCheckTimezoneItem();
    kcsi->setDefault();
    mTimezoneCheck->setChecked(Settings::fileCheckTimezone());

    kcsi = Settings::self()->audioNotesDirectoryItem();
    kcsi->setDefault();
    mAudioNotesRequester->setUrl(Settings::audioNotesDirectory());

    slotItemChanged();
}


void SettingsFilesPage::slotItemChanged()
{
}


void SettingsFilesPage::slotClearFileWarnings()
{
    FilesController::resetAllFileWarnings();

    QPushButton *but = qobject_cast<QPushButton *>(sender());
    if (but!=nullptr) but->setEnabled(false);
}

//////////////////////////////////////////////////////////////////////////
//									//
//  SettingsMediaPage							//
//									//
//////////////////////////////////////////////////////////////////////////

SettingsMediaPage::SettingsMediaPage(QWidget *pnt)
    : KPageWidgetItem(new QWidget(pnt))
{
    setName(i18nc("@title:tab", "Media"));
    setHeader(i18n("Settings for notes and photographs"));
    setIcon(QIcon::fromTheme("applications-multimedia"));

    QWidget *w = widget();
    QFormLayout *fl = new QFormLayout(w);

    const KConfigSkeletonItem *ski = Settings::self()->photoViewModeItem();
    Q_ASSERT(ski!=nullptr);
    mPhotoViewerCombo = new QComboBox(w);
    mPhotoViewerCombo->setToolTip(ski->toolTip());
    fl->addRow(ski->label(), mPhotoViewerCombo);

    int selectIndex = -1;
    KService::List services = KMimeTypeTrader::self()->query("image/jpeg", "KParts/ReadOnlyPart");
    if (services.isEmpty()) qWarning() << "No viewer part available";
    for (KService::List::const_iterator it = services.constBegin(); it!=services.constEnd(); ++it)
    {
        const KService::Ptr service = (*it);

        mPhotoViewerCombo->addItem(QIcon::fromTheme(service->icon()), service->name(), service->storageId());
        if (service->storageId()==Settings::photoViewMode()) selectIndex = mPhotoViewerCombo->count()-1;
    }

    if (selectIndex!=-1) mPhotoViewerCombo->setCurrentIndex(selectIndex);
    if (mPhotoViewerCombo->count()==0) mPhotoViewerCombo->setEnabled(false);

    fl->addItem(DialogBase::verticalSpacerItem());

    ski = Settings::self()->photoGroupTitleItem();
    Q_ASSERT(ski!=nullptr);
    QGroupBox *g = new QGroupBox(ski->label(), w);
    g->setFlat(true);
    fl->addRow(g);

    ski = Settings::self()->photoUseGpsItem();
    Q_ASSERT(ski!=nullptr);
    mUseGpsCheck = new QCheckBox(ski->label(), w);
    mUseGpsCheck->setToolTip(ski->toolTip());
    mUseGpsCheck->setChecked(Settings::photoUseGps());
    fl->addRow(mUseGpsCheck);

    ski = Settings::self()->photoUseTimeItem();
    Q_ASSERT(ski!=nullptr);
    mUseTimeCheck = new QCheckBox(ski->label(), w);
    mUseTimeCheck->setToolTip(ski->toolTip());
    mUseTimeCheck->setChecked(Settings::photoUseTime());
    connect(mUseTimeCheck, SIGNAL(toggled(bool)), SLOT(slotItemChanged()));
    fl->addRow(mUseTimeCheck);

    ski = Settings::self()->photoTimeThresholdItem();
    Q_ASSERT(ski!=nullptr);
    mTimeThresholdSpinbox = new QSpinBox(w);
    mTimeThresholdSpinbox->setRange(ski->minValue().toInt(), ski->maxValue().toInt());
    mTimeThresholdSpinbox->setValue(Settings::photoTimeThreshold());
    mTimeThresholdSpinbox->setSuffix(i18n(" seconds"));
    mTimeThresholdSpinbox->setToolTip(ski->toolTip());

    fl->addRow(ski->label(), mTimeThresholdSpinbox);

    slotItemChanged();
}


void SettingsMediaPage::slotSave()
{
    if (!mPhotoViewerCombo->isEnabled()) return;
    Settings::setPhotoViewMode(mPhotoViewerCombo->itemData(mPhotoViewerCombo->currentIndex()).toString());

    Settings::setPhotoUseGps(mUseGpsCheck->isChecked());
    Settings::setPhotoUseTime(mUseTimeCheck->isChecked());
    Settings::setPhotoTimeThreshold(mTimeThresholdSpinbox->value());
}


void SettingsMediaPage::slotDefaults()
{
    if (!mPhotoViewerCombo->isEnabled()) return;
    mPhotoViewerCombo->setCurrentIndex(0);

    KConfigSkeletonItem *kcsi = Settings::self()->photoUseGpsItem();
    kcsi->setDefault();
    mUseGpsCheck->setChecked(Settings::photoUseGps());

    kcsi = Settings::self()->photoUseTimeItem();
    kcsi->setDefault();
    mUseTimeCheck->setChecked(Settings::photoUseGps());

    kcsi = Settings::self()->photoTimeThresholdItem();
    kcsi->setDefault();
    mTimeThresholdSpinbox->setValue(Settings::photoTimeThreshold());

    slotItemChanged();
}


void SettingsMediaPage::slotItemChanged()
{
    mTimeThresholdSpinbox->setEnabled(mUseTimeCheck->isChecked());
}
