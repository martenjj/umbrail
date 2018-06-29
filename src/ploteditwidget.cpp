
#include "ploteditwidget.h"

#include <qgridlayout.h>
#include <qspinbox.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qicon.h>
#include <qdebug.h>

#include <klocalizedstring.h>


static QString titleFor(PlotEditWidget::EntryType type)
{
    switch (type)
    {
case PlotEditWidget::Bearing:	return (i18nc("@label:spinbox", "Bearing line:"));
case PlotEditWidget::Range:	return (i18nc("@label:spinbox", "Range ring:"));
default:			return (i18nc("@label:spinbox", "(Unknown):"));
    }
}


PlotEditWidget::PlotEditWidget(PlotEditWidget::EntryType type, QWidget *parent)
    : QFrame(parent)
{
    qDebug() << "type" << type;
    mType = type;

    mLayout = new QGridLayout(this);
    mLayout->setMargin(0);
    mLayout->setColumnStretch(1, 1);

    QLabel *l = new QLabel(titleFor(type), this);
    mLayout->addWidget(l, 0, 0, Qt::AlignRight);
}


static QSpinBox *createSpinBox(PlotEditWidget::EntryType type, QWidget *parent)
{
    QSpinBox *box = new QSpinBox(parent);
    switch (type)
    {
case PlotEditWidget::Bearing:
        box->setSuffix(i18nc("unit suffix for degrees", " degrees"));
        box->setRange(0, 359);
        box->setSingleStep(1);
        break;

case PlotEditWidget::Range:
        box->setSuffix(i18nc("unit suffix for metres", " metres"));
        box->setRange(0, 100000);
        box->setSingleStep(10);
        break;
    }

    return (box);
}


void PlotEditWidget::setPlotData(const QString &newData)
{
    qDebug() << newData;

    qDeleteAll(mFields);				// remove all existing fields
    mFields.clear();

    const QStringList items = newData.split(';', QString::SkipEmptyParts);
    int row = 0;
    foreach (const QString &item, items)
    {							// get existing spin box
        // Create a spin box for each data item
        QSpinBox *box = createSpinBox(mType, this);	// create new spin box
        box->setValue(item.toInt());			// set value from data
        mFields.append(box);				// append to field list
        ++row;						// on to next row
    }

    updateLayout();					// lay out and set buttons
}


void PlotEditWidget::updateLayout()
{
    int row;						// layout row
    QLayoutItem *li;					// layout item from grid
    QWidget *w;						// entry field or button

    qDebug() << "fields" << mFields.count() << "initial row count" << mLayout->rowCount();

    // Remove all existing items from the grid layout.  Delete any
    // existing "remove/add" buttons, but not the entry fields themselves.
    for (row = mLayout->rowCount()-1; row>=0; --row)
    {
        li = mLayout->itemAtPosition(row, 1);
        if (li!=nullptr)				// entry field in this row
        {
            w = li->widget();
            if (w!=nullptr) mLayout->removeWidget(w);
        }

        li = mLayout->itemAtPosition(row, 2);
        if (li!=nullptr)				// push button in this row
        {
            w = li->widget();
            if (w!=nullptr)
            {
                mLayout->removeWidget(w);
                // The push buttons will all be recreated later
                w->deleteLater();
            }
        }
    }

    // The title in column 0 of row 0 will not have been touched, so there
    // should still be at least one row in the layout.  However, once a row
    // has been added to the layout it never shrinks again, so the row count
    // will not reflect how many actual entry fields there are.
    qDebug() << "now row count" << mLayout->rowCount();

    // Populate the layout with the entry fields, along with a "remove"
    // button for each one.
    for (row = 0; row<mFields.count(); ++row)
    {
        mLayout->addWidget(mFields[row], row, 1);

        QPushButton *but = new QPushButton(QIcon::fromTheme("list-remove"), QString(), this);
        connect(but, &QPushButton::clicked, this, &PlotEditWidget::slotRemoveRow);
        mLayout->addWidget(but, row, 2);
    }

    // Finally add an "add" button below the last populated row
    if (mFields.isEmpty()) row = 1;
    else row = mFields.count();

    qDebug() << "create 'add' button in row" << row;
    QPushButton *but = new QPushButton(QIcon::fromTheme("list-add"), QString(), this);
    connect(but, &QPushButton::clicked, this, &PlotEditWidget::slotAddRow);
    mLayout->addWidget(but, row, 2);
}


QString PlotEditWidget::plotData() const
{
    qDebug() << "fields" << mFields.count();

    QStringList values;
    foreach (const QSpinBox *box, mFields) values.append(QString::number(box->value()));
    return (values.join(';'));
}


int PlotEditWidget::rowOfButton(QObject *send) const
{
    QPushButton *but = qobject_cast<QPushButton *>(send);
    Q_ASSERT(but!=nullptr);

    int idx = mLayout->indexOf(but);
    int row, col, rowSpan, colSpan;
    mLayout->getItemPosition(idx, &row, &col, &rowSpan, &colSpan);
    return (row);
}


void PlotEditWidget::slotRemoveRow()
{
    int row = rowOfButton(sender());			// row to remove
    qDebug() << "row" << row;

    QWidget *w = mFields.takeAt(row);			// remove from field list
    w->deleteLater();					// it's no longer needed

    updateLayout();					// lay out and set buttons
}


void PlotEditWidget::slotAddRow()
{
    int row = rowOfButton(sender());			// prospective row to add
    if (row==1)						// but should it be the first?
    {
        QLayoutItem *li = mLayout->itemAtPosition(0, 1);
        if (li==nullptr) row = 0;			// first row is not populated
    }

    qDebug() << "at row" << row;

    QSpinBox *box = createSpinBox(mType, this);		// create new spin box
    mFields.insert(row, box);				// insert into field list

    updateLayout();					// lay out and set buttons
}
