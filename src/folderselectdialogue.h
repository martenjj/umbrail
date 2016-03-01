
#ifndef FOLDERSELECTDIALOGUE_H
#define FOLDERSELECTDIALOGUE_H


#include "moveitemdialogue.h"
#include "mainwindowinterface.h"


class FolderSelectDialogue : public MoveItemDialogue
{
    Q_OBJECT

public:
    explicit FolderSelectDialogue(QWidget *pnt = NULL);
    virtual ~FolderSelectDialogue()			{}

    void setDestinationPath(const QString &path);

protected slots:
    void slotNewFolder();

private slots:
    void slotUpdateButtonStates();
};

#endif							// FOLDERSELECTDIALOGUE_H
