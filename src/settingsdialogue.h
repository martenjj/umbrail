
#ifndef SETTINGSDIALOGUE_H
#define SETTINGSDIALOGUE_H


#include <kpagedialog.h>



class KColorButton;



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

private:
    KColorButton *mLineColourButton;
    KColorButton *mSelectedColourButton;
};


#endif							// SETTINGSDIALOGUE_H
