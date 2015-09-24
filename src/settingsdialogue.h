
#ifndef SETTINGSDIALOGUE_H
#define SETTINGSDIALOGUE_H


#include <kpagedialog.h>


class QCheckBox;
class QSpinBox;
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
    QCheckBox *mShowTrackArrowsCheck;
    KColorButton *mSelectedOuterButton;
    KColorButton *mSelectedInnerButton;
};


class SettingsFilesPage : public KPageWidgetItem
{
    Q_OBJECT

public:
    SettingsFilesPage(QWidget *pnt = NULL);
    virtual ~SettingsFilesPage()				{}

public slots:
    void slotSave();
    void slotDefaults();

private slots:
    void slotItemChanged();

private:
    KUrlRequester *mAudioNotesRequester;
    QCheckBox *mUseGpsCheck;
    QCheckBox *mUseTimeCheck;
    QSpinBox *mTimeThresholdSpinbox;
};

#endif							// SETTINGSDIALOGUE_H
