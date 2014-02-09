
#include "settingsdialogue.h"

#include <qformlayout.h>

#include <kdebug.h>
#include <klocale.h>
#include <kpagedialog.h>
#include <kcolorbutton.h>

#include "settings.h"
#include "style.h"



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








    setMinimumSize(400,360);
    restoreDialogSize(KGlobal::config()->group(objectName()));
}


SettingsDialogue::~SettingsDialogue()
{
    KConfigGroup grp = KGlobal::config()->group(objectName());
    saveDialogSize(grp);
}




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
    fl->addRow(i18n("Default line colour:"), mLineColourButton);

    mSelectedColourButton = new KColorButton(Settings::selectedLineColour(), w);
    mSelectedColourButton->setAlphaChannelEnabled(false);
    fl->addRow(i18n("Selected line colour:"), mSelectedColourButton);




}





void SettingsMapStylePage::slotSave()
{
    Settings::setLineColour(mLineColourButton->color());
    Settings::setSelectedLineColour(mSelectedColourButton->color());



    // Update the global style from the new application settings
    Style::globalStyle()->setLineColour(mLineColourButton->color());



}


void SettingsMapStylePage::slotDefaults()
{
    KConfigSkeletonItem *kcsi = Settings::self()->lineColourItem();
    kcsi->setDefault();
    mLineColourButton->setColor(Settings::lineColour());

    kcsi = Settings::self()->selectedLineColourItem();
    kcsi->setDefault();
    mSelectedColourButton->setColor(Settings::selectedLineColour());




}
