
#ifndef MAPTHEMEDIALOGUE_H
#define MAPTHEMEDIALOGUE_H


#include <kdialog.h>


class QListWidget;
class QStandardItemModel;


class MapThemeDialogue : public KDialog
{
    Q_OBJECT

public:
    MapThemeDialogue(QStandardItemModel *model, QWidget *pnt = NULL);
    virtual ~MapThemeDialogue();

    void setThemeId(const QString &id);
    QString themeId() const;

private:
    void createDisplay();

private:
    QStandardItemModel *mModel;
    QListWidget *mListBox;

};

#endif							// MAPTHEMEDIALOGUE_H
