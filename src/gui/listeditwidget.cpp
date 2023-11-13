
#include "listeditwidget.h"

#include <qformlayout.h>
#include <qpushbutton.h>
#include <qlabel.h>

#include <klocalizedstring.h>


// TODO: use also in TrackPropertiesDetailPages

ListEditWidget::ListEditWidget(QWidget *pnt)
    : QWidget(pnt)
{
    // TODO: use QHBoxLayout with first widget stretched
    QGridLayout *hlay = new QGridLayout(this);
    hlay->setMargin(0);

    mListLabel = new QLabel(this);
    mListLabel->setTextInteractionFlags(Qt::TextSelectableByMouse|Qt::TextSelectableByKeyboard);
    hlay->addWidget(mListLabel, 0, 0, Qt::AlignTop);
    hlay->setColumnStretch(0, 1);

    QPushButton *b = new QPushButton(pnt);
    //QPushButton *b = new QPushButton(i18nc("@action:button", "Edit..."), pnt);
    b->setIcon(QIcon::fromTheme("document-edit"));
    setFocusProxy(b);
    setFocusPolicy(Qt::StrongFocus);
    hlay->addWidget(b, 0, 1, Qt::AlignRight|Qt::AlignTop);

    connect(b, &QAbstractButton::clicked, this, &ListEditWidget::editRequested);
}


void ListEditWidget::setList(const QStringList &list)
{
    mListLabel->setText(list.join(", "));
}
