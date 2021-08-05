
#ifndef FOLDERSELECTWIDGET_H
#define FOLDERSELECTWIDGET_H


#include <qframe.h>
#include "applicationdatainterface.h"

class QLineEdit;


class FolderSelectWidget : public QFrame, public ApplicationDataInterface
{
    Q_OBJECT

public:
    explicit FolderSelectWidget(QWidget *pnt = nullptr);
    virtual ~FolderSelectWidget() = default;

    QString folderPath() const;
    void setFolderPath(const QString &path, bool asDefault);

signals:
    void folderChanged(const QString &path);

protected slots:
    void slotSelectFolder();

private:
    QLineEdit *mDestFolder;
    QString mDefaultFolder;
};

#endif							// FOLDERSELECTWIDGET_H
