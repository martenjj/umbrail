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

#ifndef TIMEZONEDIALOGUE_H
#define TIMEZONEDIALOGUE_H

#include <kfdialog/dialogbase.h>
#include <kfdialog/dialogstatesaver.h>

class TimeZoneWidget;


class TimeZoneDialogue : public DialogBase, public DialogStateSaver
{
    Q_OBJECT

public:
    explicit TimeZoneDialogue(QWidget *pnt = nullptr);
    virtual ~TimeZoneDialogue() = default;

    void setTimeZone(const QByteArray &zone);
    QString timeZone() const;

    void saveConfig(QDialog *dialog, KConfigGroup &grp) const override;
    void restoreConfig(QDialog *dialog, const KConfigGroup &grp) override;

protected slots:
    void slotUseUTC();
    void slotUseSystem();

private slots:
    void slotTimeZoneChanged();

private:
    TimeZoneWidget *mTimeZoneWidget;
    bool mReturnUTC;
};

#endif							// TIMEZONEDIALOGUE_H
