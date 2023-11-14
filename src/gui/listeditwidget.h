// -*-mode:c++ -*-

#ifndef LISTEDITWIDGET_H
#define LISTEDITWIDGET_H
 

#include <qwidget.h>

class QLineEdit;


class ListEditWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ListEditWidget(QWidget *pnt = nullptr);
    virtual ~ListEditWidget() = default;

    void setList(const QStringList &list);
    void setDisplayToolTip(const QString &tip);
    void setEditToolTip(const QString &tip);

signals:
    void editRequested();

private:
    QLineEdit *mListLabel;
};

#endif							// LISTEDITWIDGET_H
