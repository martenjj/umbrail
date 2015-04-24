
#ifndef SETTINGSDIALOGUE_H
#define SETTINGSDIALOGUE_H


#include <kpagedialog.h>


class QCheckBox;
class KColorButton;
class KUrlRequester;


class SettingsDialogue : public KPageDialog
{
    Q_OBJECT

public:
    SettingsDialogue(QWidget *pnt = NULL);
    virtual ~SettingsDialogue();


private:

private:


};


class SettingsMapStylePage : public KPageWidgetItem
{
    Q_OBJECT

public:
    SettingsMapStylePage(QWidget *pnt = NULL);
    virtual ~SettingsMapStylePage()				{}

public slots:
    void slotSave();
    void slotDefaults();

private slots:
    void slotItemChanged();

private:
    KColorButton *mLineColourButton;
    QCheckBox *mSelectedUseSystemCheck;
    KColorButton *mSelectedOuterButton;
    KColorButton *mSelectedInnerButton;
};


class SettingsPathsPage : public KPageWidgetItem
{
    Q_OBJECT

public:
    SettingsPathsPage(QWidget *pnt = NULL);
    virtual ~SettingsPathsPage()				{}

public slots:
    void slotSave();
    void slotDefaults();

private slots:
    void slotItemChanged();

private:
    KUrlRequester *mAudioNotesRequester;
};

#endif							// SETTINGSDIALOGUE_H
