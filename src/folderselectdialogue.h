
#ifndef FOLDERSELECTDIALOGUE_H
#define FOLDERSELECTDIALOGUE_H


#include "moveitemdialogue.h"


class MainWindow;


class FolderSelectDialogue : public MoveItemDialogue
{
    Q_OBJECT

public:
    explicit FolderSelectDialogue(MainWindow *mw, QWidget *pnt = NULL);
    virtual ~FolderSelectDialogue()			{}

    void setDestinationPath(const QString &path);

protected slots:
    void slotNewFolder();

private slots:
    void slotUpdateButtonStates();

private:
    MainWindow *mMainWindow;
};

#endif							// FOLDERSELECTDIALOGUE_H
