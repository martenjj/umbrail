
#include "mainwindowinterface.h"
#include "mainwindow.h"


MainWindowInterface::MainWindowInterface(QObject *pnt)
{
    QObject *obj = pnt;
    mMainWindow = NULL;
    while (obj!=NULL)
    {
        mMainWindow = qobject_cast<MainWindow *>(obj);
        if (mMainWindow!=NULL) break;
        obj = obj->parent();
    }

    if (mMainWindow==NULL)
    {
        qFatal("MainWindowInterface: Parent '%s' must be a descendent of MainWindow", qPrintable(pnt->objectName()));
    }
}


FilesController *MainWindowInterface::filesController() const
{
    return (mMainWindow->filesController());
}


MapController *MainWindowInterface::mapController() const
{
    return (mMainWindow->mapController());
}
