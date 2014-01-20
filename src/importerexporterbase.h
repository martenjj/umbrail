// -*-mode:c++ -*-

#ifndef IMPORTEREXPORTERBASE_H
#define IMPORTEREXPORTERBASE_H
 

#include <qstring.h>


//class MainWindow;


class ImporterExporterBase
{
protected:
    ImporterExporterBase();
    virtual ~ImporterExporterBase();

    void setError(const QString &err);
//    MainWindow *mainWindow() const		{ return (mMainWindow); }

public:
    const QString &lastError();

//signals:
//    void statusMessage(const QString &text);

protected:
    QString mErrorString;
//    MainWindow *mMainWindow;
};

 
#endif							// IMPORTEREXPORTERBASE_H
