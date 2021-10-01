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


class SettingsMapStylePage : public KPageWidgetItem
{
    Q_OBJECT

public:
    explicit SettingsMapStylePage(QWidget *pnt = nullptr);
    virtual ~SettingsMapStylePage() = default;

public slots:
    void slotSave();
    void slotDefaults();

private slots:
    void slotItemChanged();

private:
    KColorButton *mLineColourButton;
    KColorButton *mPointColourButton;
    QCheckBox *mSelectedUseSystemCheck;
    QCheckBox *mShowTrackArrowsCheck;
    KColorButton *mSelectedOuterButton;
    KColorButton *mSelectedInnerButton;
};


class SettingsFilesPage : public KPageWidgetItem
{
    Q_OBJECT

public:
    explicit SettingsFilesPage(QWidget *pnt = nullptr);
    virtual ~SettingsFilesPage() = default;

public slots:
    void slotSave();
    void slotDefaults();

private slots:
    void slotItemChanged();
    void slotClearFileWarnings();

private:
    QCheckBox *mTimezoneCheck;
    KUrlRequester *mAudioNotesRequester;
};


class SettingsMediaPage : public KPageWidgetItem
{
    Q_OBJECT

public:
    explicit SettingsMediaPage(QWidget *pnt = nullptr);
    virtual ~SettingsMediaPage() = default;

public slots:
    void slotSave();
    void slotDefaults();

private slots:
    void slotItemChanged();

private:
    QComboBox *mPhotoViewerCombo;
    QCheckBox *mUseGpsCheck;
    QCheckBox *mUseTimeCheck;
    QSpinBox *mTimeThresholdSpinbox;
};


class SettingsServicesPage : public KPageWidgetItem
{
    Q_OBJECT

public:
    explicit SettingsServicesPage(QWidget *pnt = nullptr);
    virtual ~SettingsServicesPage() = default;

public slots:
    void slotSave();
    void slotDefaults();

private slots:
    void slotItemChanged();

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
