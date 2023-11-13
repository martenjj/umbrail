
#include "listeditwidget.h"

#include <qformlayout.h>
#include <qpushbutton.h>
#include <qlineedit.h>

#include <klocalizedstring.h>


// TODO: can use also in TrackPropertiesDetailPages?

ListEditWidget::ListEditWidget(QWidget *pnt)
    : QWidget(pnt)
{
    // TODO: use QHBoxLayout with first widget stretched
    QGridLayout *hlay = new QGridLayout(this);
    hlay->setMargin(0);

    mListLabel = new QLineEdit(this);
    mListLabel->setReadOnly(true);
    hlay->addWidget(mListLabel, 0, 0);
    hlay->setColumnStretch(0, 1);

    QPushButton *b = new QPushButton(pnt);
    //QPushButton *b = new QPushButton(i18nc("@action:button", "Edit..."), pnt);
    b->setIcon(QIcon::fromTheme("document-edit"));
    setFocusProxy(b);
    setFocusPolicy(Qt::StrongFocus);
    hlay->addWidget(b, 0, 1, Qt::AlignRight);

    connect(b, &QAbstractButton::clicked, this, &ListEditWidget::editRequested);
}


void ListEditWidget::setList(const QStringList &list)
{
    mListLabel->setText(list.join(", "));
}
