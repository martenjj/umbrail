
#include "trackpropertiesdetailpages.h"

#include <qformlayout.h>

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




void TrackPropertiesPage::addSpacerField()
{
    mFormLayout->addItem(new QSpacerItem(1, KDialog::spacingHint(), QSizePolicy::Minimum, QSizePolicy::Fixed));
}


