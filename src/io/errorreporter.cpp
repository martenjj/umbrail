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

#include "errorreporter.h"

#include <qdebug.h>
#include <qurl.h>

#include <klocalizedstring.h>
#include <kcolorscheme.h>


class ErrorRecord
{
public:
    ErrorRecord(ErrorReporter::Severity severity);
    virtual ~ErrorRecord()				{}

    virtual QString format() const = 0;

protected:
    QString severityString() const;

private:
    ErrorReporter::Severity mSeverity;
};


class ErrorRecordFile : public ErrorRecord
{
public:
    ErrorRecordFile(const QUrl &file);
    virtual ~ErrorRecordFile()				{}

    QString format() const override;

private:
    QUrl mFile;
};


class ErrorRecordMessage : public ErrorRecord
{
public:
    ErrorRecordMessage(ErrorReporter::Severity severity, const QString &message, int line = -1);
    virtual ~ErrorRecordMessage()			{}

    QString format() const override;

private:
    QString mMessage;
    int mLine;
};


ErrorRecord::ErrorRecord(ErrorReporter::Severity severity)
{
    mSeverity = severity;
}


static QString errorColour(KColorScheme::ForegroundRole role)
{
    KColorScheme sch(QPalette::Normal);
    return (QString("#%1").arg(sch.foreground(role).color().rgb() & 0x00FFFFFF,
                               6, 16, QLatin1Char('0')));
}


QString ErrorRecord::severityString() const
{
    switch (mSeverity)
    {
case ErrorReporter::NoError:	return (i18n("No error"));
case ErrorReporter::Warning:	return (i18n("<font color=\"%1\">Warning</font>", errorColour(KColorScheme::NeutralText)));
case ErrorReporter::Error:	return (i18n("<font color=\"%1\">Error</font>", errorColour(KColorScheme::NegativeText)));
case ErrorReporter::Fatal:	return (i18n("<font color=\"%1\">Fatal</font>", errorColour(KColorScheme::NegativeText)));
default:			return (i18n("Unknown"));
    }

}


ErrorRecordFile::ErrorRecordFile(const QUrl &file)
    : ErrorRecord(ErrorReporter::NoError)
{
    mFile = file;
}


QString ErrorRecordFile::format() const
{
    return (i18n("<font color=\"%1\">In file</font> <filename>%2</filename><font color=\"%1\">:</font>",
                 errorColour(KColorScheme::InactiveText),
                 mFile.toDisplayString()));
}


ErrorRecordMessage::ErrorRecordMessage(ErrorReporter::Severity severity, const QString &message, int line)
    : ErrorRecord(severity)
{
    mMessage = message;
    mLine = line;
}


QString ErrorRecordMessage::format() const
{
    if (mLine>-1)					// including line number
    {
        return (i18n("%1 at line %2: %3", severityString(), QString::number(mLine), mMessage));
    }
    else						// no line number
    {
        return (i18n("%1: %2", severityString(), mMessage));
    }
}


ErrorReporter::ErrorReporter()
{
    qDebug();

    mSeverity = ErrorReporter::NoError;
}


ErrorReporter::~ErrorReporter()
{
    qDeleteAll(mList);
    qDebug() << "done";
}


void ErrorReporter::setFile(const QUrl &file)
{
    qDebug() << file;
    mList.append(new ErrorRecordFile(file));
}


void ErrorReporter::setError(ErrorReporter::Severity sev, const QString &message, int line)
{
    qDebug() << "severity" << sev << "line" << line << "=" << message;
    mList.append(new ErrorRecordMessage(sev, message, line));
    if (sev>mSeverity) mSeverity = sev;
}


int ErrorReporter::messageCount() const
{
    int count = 0;

    for (QList<ErrorRecord *>::const_iterator it = mList.constBegin(); it!=mList.constEnd(); ++it)
    {
        const ErrorRecord *record = (*it);
        if (dynamic_cast<const ErrorRecordMessage *>(record)!=nullptr) ++count;
    }

    return (count);
}


QStringList ErrorReporter::messageList() const
{
    QStringList result;

    for (QList<ErrorRecord *>::const_iterator it = mList.constBegin(); it!=mList.constEnd(); ++it)
    {
        const ErrorRecord *record = (*it);
        result.append(record->format());
    }

    return (result);
}
