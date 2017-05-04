
#ifndef MAPTHEMEDIALOGUE_H
#define MAPTHEMEDIALOGUE_H


#include <dialogbase.h>


class QListWidget;
class QStandardItemModel;


class MapThemeDialogue : public DialogBase
{
    Q_OBJECT

public:
    MapThemeDialogue(QStandardItemModel *model, QWidget *pnt = nullptr);
    virtual ~MapThemeDialogue() = default;

    void setThemeId(const QString &id);
    QString themeId() const;

signals:
    void themeSelected(const QString &themeId);

private:
    void createDisplay();

private:
    QStandardItemModel *mModel;
    QListWidget *mListBox;

};

#endif							// MAPTHEMEDIALOGUE_H
