
#include "trackpropertiesdetailpages.h"

#include "time.h"

#include <qformlayout.h>
#include <qgroupbox.h>

#include <kdebug.h>
#include <kdialog.h>
#include <klocale.h>
#include <ksystemtimezone.h>

#include "trackdata.h"





TrackPropertiesPage::TrackPropertiesPage(const QList<TrackDataItem *> items, QWidget *pnt)
    : QWidget(pnt)
{
    Q_ASSERT(!items.isEmpty());
    mFormLayout = new QFormLayout(this);

    mTimeZone = NULL;
    mIsEmpty = (TrackData::sumTotalChildCount(items)==0);
}




TrackPropertiesPage::~TrackPropertiesPage()
{
    delete mTimeZone;
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



void TrackPropertiesPage::setTimeZone(const QString &name)
{
    kDebug() << name;
    mTimeZone = new KTimeZone(KSystemTimeZones::zone(name));
    kDebug() << "set to" << mTimeZone->name() << "offset" << mTimeZone->offset(time(NULL));
    emit updateTimeZones(timeZone());
}




void TrackPropertiesPage::disableIfEmpty(QWidget *field)
{
    if (!isEmpty()) return;
    mFormLayout->labelForField(field)->setEnabled(false);
    field->setEnabled(false);
}
