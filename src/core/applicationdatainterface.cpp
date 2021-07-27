
#include "applicationdatainterface.h"

#include <qobject.h>
#include <qwidget.h>

#include "applicationdata.h"


ApplicationDataInterface::ApplicationDataInterface(QObject *pnt)
{
    QObject *obj = pnt;
    mApplicationData = nullptr;
    while (obj!=nullptr)
    {
        mApplicationData = dynamic_cast<ApplicationData *>(obj);
        if (mApplicationData!=nullptr) break;
        obj = obj->parent();
    }

    if (mApplicationData==nullptr)
    {
        qFatal("ApplicationDataInterface: Parent '%s' must be a descendent of ApplicationData", qPrintable(pnt->objectName()));
    }
}


FilesController *ApplicationDataInterface::filesController() const
{
    return (mApplicationData->filesController());
}


FilesView *ApplicationDataInterface::filesView() const
{
    return (mApplicationData->filesView());
}


MapController *ApplicationDataInterface::mapController() const
{
    return (mApplicationData->mapController());
}


QWidget *ApplicationDataInterface::mainWidget() const
{
    return (mApplicationData->mainWidget());
}


bool ApplicationDataInterface::isReadOnly() const
{
    return (mApplicationData->isReadOnly());
}


void ApplicationDataInterface::executeCommand(QUndoCommand *cmd)
{
    QMetaObject::invokeMethod(mApplicationData->mainWidget(),
                              "slotExecuteCommand",
                              Q_ARG(QUndoCommand *, cmd));
}
