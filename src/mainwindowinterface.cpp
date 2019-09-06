
#include "mainwindowinterface.h"
#include "mainwindow.h"


MainWindowInterface::MainWindowInterface(QObject *pnt)
{
    QObject *obj = pnt;
    mMainWindow = nullptr;
    while (obj!=nullptr)
    {
        mMainWindow = qobject_cast<MainWindow *>(obj);
        if (mMainWindow!=nullptr) break;
        obj = obj->parent();
    }

    if (mMainWindow==nullptr)
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
