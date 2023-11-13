// -*-mode:c++ -*-

#ifndef LISTEDITWIDGET_H
#define LISTEDITWIDGET_H
 

#include <qwidget.h>

class QLineEdit;


class ListEditWidget : public QWidget
{
    Q_OBJECT

public:
    // TODO: for use with TrackPropertiesDetailPage
    //explicit ListEditWidget(bool withEditButtonText, QWidget *pnt = nullptr);
    explicit ListEditWidget(QWidget *pnt = nullptr);
    virtual ~ListEditWidget() = default;

    void setList(const QStringList &list);

signals:
    void editRequested();

private:
    QLineEdit *mListLabel;
};

#endif							// LISTEDITWIDGET_H
