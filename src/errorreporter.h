// -*-mode:c++ -*-

#ifndef ERRORREPORTER_H
#define ERRORREPORTER_H


#include <qlist.h>
#include <qstringlist.h>


class KUrl;

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

    void setFile(const KUrl &file);
    void setError(ErrorReporter::Severity severity, const QString &message, int line = -1);

    ErrorReporter::Severity severity() const		{ return (mSeverity); }

    QStringList messageList() const;
    int messageCount() const;

private:
    ErrorReporter::Severity mSeverity;
    QList<ErrorRecord *> mList;
};

 
#endif							// ERRORREPORTER_H
