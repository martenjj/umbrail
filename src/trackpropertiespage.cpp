
#include "trackpropertiesdetailpages.h"

#include <qformlayout.h>
#include <qgroupbox.h>

#include <kdebug.h>
#include <kdialog.h>
#include <klocale.h>






TrackPropertiesPage::TrackPropertiesPage(const QList<TrackDataItem *> items, QWidget *pnt)
    : QWidget(pnt)
{
    Q_ASSERT(!items.isEmpty());
    mFormLayout = new QFormLayout(this);


}






void TrackPropertiesPage::slotDataChanged()
{
    emit dataChanged();
}




void TrackPropertiesPage::addSeparatorField(const QString &title)
{
    if (title.isEmpty())				// no title, just some space
    {
        mFormLayout->addItem(new QSpacerItem(1, KDialog::spacingHint(), QSizePolicy::Minimum, QSizePolicy::Fixed));
    }
    else						// title, a separator line
    {
        QGroupBox *sep = new QGroupBox(title, this);
        sep->setFlat(true);
        mFormLayout->addRow(sep);
    }
}

