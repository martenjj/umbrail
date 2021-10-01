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

#ifndef ERRORREPORTER_H
#define ERRORREPORTER_H

#include <qlist.h>
#include <qstringlist.h>

class QUrl;
class ErrorRecord;


class ErrorReporter
{
public:

    enum Severity
    {
        NoError,
        Warning,
        Error,
        Fatal
    };

    ErrorReporter();
    virtual ~ErrorReporter();

    void setFile(const QUrl &file);
    void setError(ErrorReporter::Severity severity, const QString &message, int line = -1);

    ErrorReporter::Severity severity() const		{ return (mSeverity); }

    QStringList messageList() const;
    int messageCount() const;

private:
    ErrorReporter::Severity mSeverity;
    QList<ErrorRecord *> mList;
};

#endif							// ERRORREPORTER_H
