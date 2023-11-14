
#include "listeditwidget.h"

#include <qtoolbutton.h>
#include <qlineedit.h>
#include <qboxlayout.h>

#include <klocalizedstring.h>


// TODO: can use also in TrackPropertiesDetailPages?

ListEditWidget::ListEditWidget(QWidget *pnt)
    : QWidget(pnt)
{
    QHBoxLayout *hlay = new QHBoxLayout(this);
    hlay->setContentsMargins(0, 0, 0, 0);

    mListLabel = new QLineEdit(this);
    mListLabel->setReadOnly(true);
    hlay->addWidget(mListLabel);
    hlay->setStretch(0, 1);

    QToolButton *b = new QToolButton(this);
    b->setAutoRaise(true);
    b->setIcon(QIcon::fromTheme("document-edit"));
    connect(b, &QAbstractButton::clicked, this, &ListEditWidget::editRequested);
    hlay->addWidget(b);

    setFocusProxy(b);
    setFocusPolicy(Qt::StrongFocus);
}


void ListEditWidget::setList(const QStringList &list)
{
    mListLabel->setText(list.join(", "));
    mListLabel->setCursorPosition(0);			// show start of new text
}


void ListEditWidget::setDisplayToolTip(const QString &tip)
{
    mListLabel->setToolTip(tip);			// the line edit
}


void ListEditWidget::setEditToolTip(const QString &tip)
{
    layout()->itemAt(1)->widget()->setToolTip(tip);	// the tool button
}
