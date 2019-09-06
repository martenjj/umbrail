
#ifndef FOLDERSELECTWIDGET_H
#define FOLDERSELECTWIDGET_H


#include <qframe.h>
#include "mainwindowinterface.h"

class QLineEdit;


class FolderSelectWidget : public QFrame, public MainWindowInterface
{
    Q_OBJECT

public:
    explicit FolderSelectWidget(QWidget *pnt = nullptr);
    virtual ~FolderSelectWidget()			{}

    QString folderPath() const;

signals:
    void folderChanged(const QString &path);

protected slots:
    void slotSelectFolder();

private:
    QLineEdit *mDestFolder;
};

#endif							// FOLDERSELECTWIDGET_H
