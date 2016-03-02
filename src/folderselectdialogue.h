
#ifndef FOLDERSELECTDIALOGUE_H
#define FOLDERSELECTDIALOGUE_H


#include "itemselectdialogue.h"


class FolderSelectDialogue : public ItemSelectDialogue
{
    Q_OBJECT

public:
    explicit FolderSelectDialogue(QWidget *pnt = NULL);
    virtual ~FolderSelectDialogue()			{}

    void setPath(const QString &path);

protected slots:
    void slotNewFolder();

private slots:
    void slotUpdateButtonStates();
};

#endif							// FOLDERSELECTDIALOGUE_H
