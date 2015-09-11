
#include "settingsdialogue.h"

#include <qformlayout.h>
#include <qformlayout.h>
#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qspinbox.h>

#include <kdebug.h>
#include <klocale.h>
#include <kpagedialog.h>
#include <kcolorbutton.h>
#include <kurlrequester.h>
#include <kconfigskeleton.h>

#include "settings.h"
#include "style.h"

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
    setButtons(KDialog::Ok|KDialog::Cancel|KDialog::Default);
    setFaceType(KPageDialog::Auto);
    showButtonSeparator(true);

    KPageWidgetItem *page = new SettingsMapStylePage(this);
    addPage(page);
    connect(this, SIGNAL(okClicked()), page, SLOT(slotSave()));
    connect(this, SIGNAL(applyClicked()), page, SLOT(slotSave()));
    connect(this, SIGNAL(defaultClicked()), page, SLOT(slotDefaults()));

    page = new SettingsFilesPage(this);
    addPage(page);
    connect(this, SIGNAL(okClicked()), page, SLOT(slotSave()));
    connect(this, SIGNAL(applyClicked()), page, SLOT(slotSave()));
    connect(this, SIGNAL(defaultClicked()), page, SLOT(slotDefaults()));

    setMinimumSize(400, 360);
    restoreDialogSize(KGlobal::config()->group(objectName()));
}


SettingsDialogue::~SettingsDialogue()
{
    KConfigGroup grp = KGlobal::config()->group(objectName());
    saveDialogSize(grp);
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
    setIcon(KIcon("marble"));

    QWidget *w = widget();
    QFormLayout *fl = new QFormLayout(w);

    mLineColourButton = new KColorButton(Settings::lineColour(), w);
    mLineColourButton->setAlphaChannelEnabled(false);
    const KConfigSkeletonItem *kcsi = Settings::self()->lineColourItem();
    mLineColourButton->setToolTip(kcsi->toolTip());
    fl->addRow(kcsi->label(), mLineColourButton);

    fl->addItem(new QSpacerItem(1, KDialog::spacingHint(), QSizePolicy::Minimum, QSizePolicy::Fixed));

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
    Settings::setSelectedMarkOuter(mSelectedOuterButton->color());
    Settings::setSelectedMarkInner(mSelectedInnerButton->color());
    Settings::setSelectedUseSystemColours(mSelectedUseSystemCheck->isChecked());

    // Update the global style from the new application settings
    Style::globalStyle()->setLineColour(mLineColourButton->color());
}


void SettingsMapStylePage::slotDefaults()
{
    KConfigSkeletonItem *kcsi = Settings::self()->lineColourItem();
    kcsi->setDefault();
    mLineColourButton->setColor(Settings::lineColour());

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
    setHeader(i18n("Settings for file locations and importing"));
    setIcon(KIcon("folder"));

    QWidget *w = widget();
    QFormLayout *fl = new QFormLayout(w);

    const KConfigSkeletonItem *ski = Settings::self()->pathsGroupTitleItem();
    Q_ASSERT(ski!=NULL);
    QGroupBox *g = new QGroupBox(ski->label(), w);
    g->setFlat(true);
    fl->addRow(g);

    ski = Settings::self()->audioNotesDirectoryItem();
    Q_ASSERT(ski!=NULL);
    mAudioNotesRequester = new KUrlRequester(w);
    mAudioNotesRequester->setMode(KFile::Directory|KFile::ExistingOnly|KFile::LocalOnly);
    mAudioNotesRequester->setUrl(KUrl(Settings::audioNotesDirectory()));
    mAudioNotesRequester->setToolTip(ski->toolTip());
    fl->addRow(ski->label(), mAudioNotesRequester);

    fl->addItem(new QSpacerItem(1, KDialog::spacingHint(), QSizePolicy::Fixed, QSizePolicy::Fixed));

    ski = Settings::self()->photoGroupTitleItem();
    Q_ASSERT(ski!=NULL);
    g = new QGroupBox(ski->label(), w);
    g->setFlat(true);
    fl->addRow(g);

    ski = Settings::self()->photoUseGpsItem();
    Q_ASSERT(ski!=NULL);
    mUseGpsCheck = new QCheckBox(ski->label(), w);
    mUseGpsCheck->setToolTip(ski->toolTip());
    mUseGpsCheck->setChecked(Settings::photoUseGps());
    fl->addRow(mUseGpsCheck);

    ski = Settings::self()->photoUseTimeItem();
    Q_ASSERT(ski!=NULL);
    mUseTimeCheck = new QCheckBox(ski->label(), w);
    mUseTimeCheck->setToolTip(ski->toolTip());
    mUseTimeCheck->setChecked(Settings::photoUseTime());
    connect(mUseTimeCheck, SIGNAL(toggled(bool)), SLOT(slotItemChanged()));
    fl->addRow(mUseTimeCheck);

    ski = Settings::self()->photoTimeThresholdItem();
    Q_ASSERT(ski!=NULL);
    mTimeThresholdSpinbox = new QSpinBox(w);
    mTimeThresholdSpinbox->setRange(ski->minValue().toInt(), ski->maxValue().toInt());
    mTimeThresholdSpinbox->setValue(Settings::photoTimeThreshold());
    mTimeThresholdSpinbox->setToolTip(ski->toolTip());
    fl->addRow(ski->label(), mTimeThresholdSpinbox);

    slotItemChanged();
}


void SettingsFilesPage::slotSave()
{
    Settings::setAudioNotesDirectory(mAudioNotesRequester->url().url());
    Settings::setPhotoUseGps(mUseGpsCheck->isChecked());
    Settings::setPhotoUseTime(mUseTimeCheck->isChecked());
    Settings::setPhotoTimeThreshold(mTimeThresholdSpinbox->value());
}


void SettingsFilesPage::slotDefaults()
{
    KConfigSkeletonItem *kcsi = Settings::self()->audioNotesDirectoryItem();
    kcsi->setDefault();
    mAudioNotesRequester->setUrl(KUrl(Settings::audioNotesDirectory()));

    kcsi = Settings::self()->photoUseGpsItem();
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


void SettingsFilesPage::slotItemChanged()
{
    mTimeThresholdSpinbox->setEnabled(mUseTimeCheck->isChecked());
}
