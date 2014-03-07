
#include "mapthemedialogue.h"

#include <qlayout.h>
#include <qlabel.h>
#include <qlistwidget.h>
#include <qstandarditemmodel.h>

#include <kdebug.h>
#include <klocale.h>


#define PIXMAP_SIZE		40			// size of preview pixmap


MapThemeDialogue::MapThemeDialogue(QStandardItemModel *model, QWidget *pnt)
    : KDialog(pnt)
{
    setObjectName("MapThemeDialogue");

    setModal(true);
    setButtons(KDialog::Ok|KDialog::Cancel);
    setButtonText(KDialog::Ok, i18n("Select"));
    setCaption(i18n("Select Map Theme"));
    showButtonSeparator(true);

    QWidget *vb = new QWidget(this);
    QVBoxLayout *vlay = new QVBoxLayout(vb);
    setMainWidget(vb);

    QLabel *l = new QLabel(i18n("Available Themes:"), vb);
    vlay->addWidget(l);

    mListBox = new QListWidget(vb);
    mListBox->setSelectionMode(QAbstractItemView::SingleSelection);
    mListBox->setUniformItemSizes(true);
    vlay->addWidget(mListBox, 1);
    l->setBuddy(mListBox);

    setMinimumSize(QSize(370, 200));

    mModel = model;
    createDisplay();
}


MapThemeDialogue::~MapThemeDialogue()
{
}


void MapThemeDialogue::setThemeId(const QString &id)
{
    kDebug() << id;

    for (int i = 0; i<mListBox->count(); ++i)
    {
        QListWidgetItem *item = mListBox->item(i);
        if (item==NULL) continue;

        QString itemId = item->data(Qt::UserRole).toString();
        if (itemId==id)
        {
            item->setSelected(true);
            mListBox->scrollToItem(item, QAbstractItemView::PositionAtCenter);
            break;
        }
    }

}


QString MapThemeDialogue::themeId() const
{
    QList<QListWidgetItem *> selected = mListBox->selectedItems();
    if (selected.count()==0) return (QString::null);	// should never happen

    QString id = selected.first()->data(Qt::UserRole).toString();
    kDebug() << id;
    return (id);
}


void MapThemeDialogue::createDisplay()
{
    kDebug() << "rows" << mModel->rowCount();
    for (int i = 0; i<mModel->rowCount(); ++i)
    {
        QStandardItem *themeData = mModel->item(i);
        if (themeData==NULL) continue;

        QString id = themeData->data(Qt::UserRole+1).toString();
        if (!id.startsWith("earth/")) continue;		// only this planet!
        kDebug() << "  " << i << id;

        QListWidgetItem *item = new QListWidgetItem();

        QWidget *hbox = new QWidget(this);
        QHBoxLayout *hlay = new QHBoxLayout(hbox);
        hlay->setMargin(0);
        hlay->setSpacing(KDialog::spacingHint());

        QLabel *label = new QLabel(hbox);
        QIcon ic = themeData->data(Qt::DecorationRole).value<QIcon>();
        label->setPixmap(ic.pixmap(PIXMAP_SIZE, PIXMAP_SIZE));
        hlay->addSpacing(KDialog::spacingHint());
        hlay->addWidget(label);

        label = new QLabel(QString("<qt><b>%1</b><br>%2")
                           .arg(themeData->data(Qt::DisplayRole).toString())
                           .arg(id));
        label->setTextInteractionFlags(Qt::NoTextInteraction);
        hlay->addSpacing(KDialog::spacingHint());
        hlay->addWidget(label);

        hlay->addStretch(1);

        item->setData(Qt::UserRole, id);		// for select/lookup
        item->setToolTip(themeData->data(Qt::ToolTipRole).toString());

        mListBox->addItem(item);
        mListBox->setItemWidget(item, hbox);
        item->setData(Qt::SizeHintRole, QSize(1, PIXMAP_SIZE+4));
    }							// X-size is ignored
}
