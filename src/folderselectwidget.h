
#ifndef FOLDERSELECTWIDGET_H
#define FOLDERSELECTWIDGET_H


#include <qframe.h>

class QLineEdit;
class MainWindow;


class FolderSelectWidget : public QFrame
{
    Q_OBJECT

public:
    explicit FolderSelectWidget(MainWindow *mw, QWidget *pnt = NULL);
    virtual ~FolderSelectWidget()			{}

    QString folderPath() const;

signals:
    void folderChanged(const QString &path);

protected slots:
    void slotSelectFolder();

private:
    QLineEdit *mDestFolder;

    MainWindow *mMainWindow;
};

#endif							// FOLDERSELECTWIDGET_H
