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

#ifndef IMPORTEREXPORTEROPTIONS_H
#define IMPORTEREXPORTEROPTIONS_H

#include <qflags.h>
#include <qstring.h>


class ImporterExporterOptions
{
public:
    enum Flag
    {
        NoFlags = 0x0000,
        // an import or export operation
        ImportExport = 0x0001,
        // export
        ToClipboard = 0x0010,
        SelectionOnly = 0x0020,
        // import
        IgnoreHome = 0x0100,
        MergeWaypoints = 0x0200,
        MarkNewWaypoints = 0x0400,
        MergeNotAllowed = 0x0800
    };
    Q_DECLARE_FLAGS(Flags, Flag)

    explicit ImporterExporterOptions(ImporterExporterOptions::Flags f = ImporterExporterOptions::NoFlags)
    {
        mFlags = f;
    }

    void setFlags(ImporterExporterOptions::Flags f)		{ mFlags = f; }
    ImporterExporterOptions::Flags flags() const		{ return (mFlags); }
    bool hasFlag(ImporterExporterOptions::Flags f) const	{ return (mFlags & f); }

    void setHomePoint(const QString &name)			{ mHomePoint = name; }
    const QString &homePoint() const				{ return (mHomePoint); }
    void setWorkPoint(const QString &name)			{ mWorkPoint = name; }
    const QString &workPoint() const				{ return (mWorkPoint); }

private:
    ImporterExporterOptions::Flags mFlags;
    QString mHomePoint;
    QString mWorkPoint;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(ImporterExporterOptions::Flags)

#endif							// IMPORTEREXPORTEROPTIONS_H
