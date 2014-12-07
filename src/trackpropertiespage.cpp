
#include "trackpropertiesdetailpages.h"

#include "time.h"

#include <qformlayout.h>
#include <qgroupbox.h>
#include <qlabel.h>

#include <kdebug.h>
#include <kdialog.h>
#include <klocale.h>
#include <ksystemtimezone.h>
#include <kcolorscheme.h>

#include "trackdata.h"





TrackPropertiesPage::TrackPropertiesPage(const QList<TrackDataItem *> items, QWidget *pnt)
    : QWidget(pnt)
{
    Q_ASSERT(!items.isEmpty());
    mFormLayout = new QFormLayout(this);

    mTimeZone = NULL;
    mPositionLabel = NULL;
    mIsEmpty = (TrackData::sumTotalChildCount(items)==0);
    if (mIsEmpty && !items.isEmpty())
    {
        if (dynamic_cast<const TrackDataTrackpoint *>(items.first())!=NULL) mIsEmpty = false;
    }
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


void TrackPropertiesPage::slotPointPositionChanged(double newLat, double newLon)
{
    if (mPositionLabel==NULL) return;
    kDebug() << newLat << newLon;

    mPositionLabel->setText(TrackData::formattedLatLong(newLat, newLon));

    KColorScheme sch(QPalette::Normal);
    QPalette pal(mPositionLabel->palette());
    pal.setColor(QPalette::WindowText, sch.foreground(KColorScheme::NeutralText).color());
    mPositionLabel->setPalette(pal);
}


void TrackPropertiesPage::disableIfEmpty(QWidget *field)
{
    if (!isEmpty()) return;

    QWidget *l = mFormLayout->labelForField(field);
    if (l!=NULL) l->setEnabled(false);
    field->setEnabled(false);
}
