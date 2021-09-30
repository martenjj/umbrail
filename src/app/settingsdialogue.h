
#ifndef SETTINGSDIALOGUE_H
#define SETTINGSDIALOGUE_H


#include <kpagedialog.h>


class QCheckBox;
class QComboBox;
class QSpinBox;
class QLineEdit;
class KColorButton;
class KUrlRequester;


class SettingsDialogue : public KPageDialog
{
    Q_OBJECT

public:
    explicit SettingsDialogue(QWidget *pnt = nullptr);
    virtual ~SettingsDialogue() = default;
};


class SettingsPage : public KPageWidgetItem
{
    Q_OBJECT

public:
    explicit SettingsPage(QWidget *pnt = nullptr);
    virtual ~SettingsPage() = default;

public slots:
    virtual void slotSave() = 0;
    virtual void slotDefaults() = 0;

protected slots:
    virtual void slotItemChanged()			{}
};


class SettingsMapStylePage : public SettingsPage
{
    Q_OBJECT

public:
    explicit SettingsMapStylePage(QWidget *pnt = nullptr);
    virtual ~SettingsMapStylePage() = default;

public slots:
    void slotSave() override;
    void slotDefaults() override;

protected slots:
    void slotItemChanged() override;

private:
    KColorButton *mLineColourButton;
    KColorButton *mPointColourButton;
    QCheckBox *mSelectedUseSystemCheck;
    QCheckBox *mShowTrackArrowsCheck;
    KColorButton *mSelectedOuterButton;
    KColorButton *mSelectedInnerButton;
};


class SettingsFilesPage : public SettingsPage
{
    Q_OBJECT

public:
    explicit SettingsFilesPage(QWidget *pnt = nullptr);
    virtual ~SettingsFilesPage() = default;

public slots:
    void slotSave() override;
    void slotDefaults() override;

private slots:
    void slotClearFileWarnings();

private:
    QCheckBox *mTimezoneCheck;
    KUrlRequester *mAudioNotesRequester;
};


class SettingsMediaPage : public SettingsPage
{
    Q_OBJECT

public:
    explicit SettingsMediaPage(QWidget *pnt = nullptr);
    virtual ~SettingsMediaPage() = default;

public slots:
    void slotSave() override;
    void slotDefaults() override;

protected slots:
    void slotItemChanged() override;

private:
    QComboBox *mPhotoViewerCombo;
    QCheckBox *mUseGpsCheck;
    QCheckBox *mUseTimeCheck;
    QSpinBox *mTimeThresholdSpinbox;
};


class SettingsServicesPage : public SettingsPage
{
    Q_OBJECT

public:
    explicit SettingsServicesPage(QWidget *pnt = nullptr);
    virtual ~SettingsServicesPage() = default;

public slots:
    void slotSave() override;
    void slotDefaults() override;

private:
    QComboBox *mOSMBrowserCombo;
#ifdef ENABLE_OPEN_WITH_GOOGLE
    QComboBox *mGoogleBrowserCombo;
#endif // ENABLE_OPEN_WITH_GOOGLE
#ifdef ENABLE_OPEN_WITH_BING
    QComboBox *mBingBrowserCombo;
#endif // ENABLE_OPEN_WITH_BING
    QLineEdit *mGeonamesUserEdit;
};

#endif							// SETTINGSDIALOGUE_H
