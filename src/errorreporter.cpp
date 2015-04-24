
#include "errorreporter.h"

#include <kdebug.h>
#include <klocale.h>
#include <kurl.h>
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
    ErrorRecordFile(const KUrl &file);
    virtual ~ErrorRecordFile()				{}

    QString format() const;

private:
    KUrl mFile;
};


class ErrorRecordMessage : public ErrorRecord
{
public:
    ErrorRecordMessage(ErrorReporter::Severity severity, const QString &message, int line = -1);
    virtual ~ErrorRecordMessage()			{}

    QString format() const;

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


ErrorRecordFile::ErrorRecordFile(const KUrl &file)
    : ErrorRecord(ErrorReporter::NoError)
{
    mFile = file;
}


QString ErrorRecordFile::format() const
{
    return (i18n("<font color=\"%1\">In file</font> <filename>%2</filename><font color=\"%1\">:</font>",
errorColour(KColorScheme::InactiveText),
mFile.pathOrUrl()));
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
        return (i18n("%1 at line %2: %3", severityString(), mLine, mMessage));
    }
    else						// no line number
    {
        return (i18n("%1: %2", severityString(), mMessage));
    }
}


ErrorReporter::ErrorReporter()
{
    kDebug();

    mSeverity = ErrorReporter::NoError;
}


ErrorReporter::~ErrorReporter()
{
    qDeleteAll(mList);
    kDebug() << "done";
}


void ErrorReporter::setFile(const KUrl &file)
{
    kDebug() << file;
    mList.append(new ErrorRecordFile(file));
}


void ErrorReporter::setError(ErrorReporter::Severity sev, const QString &message, int line)
{
    kDebug() << "severity" << sev << "line" << line << "=" << message;
    mList.append(new ErrorRecordMessage(sev, message, line));
    if (sev>mSeverity) mSeverity = sev;
}


int ErrorReporter::messageCount() const
{
    int count = 0;

    for (QList<ErrorRecord *>::const_iterator it = mList.constBegin(); it!=mList.constEnd(); ++it)
    {
        const ErrorRecord *record = (*it);
        if (dynamic_cast<const ErrorRecordMessage *>(record)!=NULL) ++count;
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