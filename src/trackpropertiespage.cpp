
#include "trackpropertiesdetailpages.h"

#include <qformlayout.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qdebug.h>
#include <qtimezone.h>

#include <klocalizedstring.h>
#include <kcolorscheme.h>

#include <dialogbase.h>

#include "trackdata.h"


TrackPropertiesPage::TrackPropertiesPage(const QList<TrackDataItem *> *items, QWidget *pnt)
    : QWidget(pnt)
{
    Q_ASSERT(items!=NULL);
    Q_ASSERT(!items->isEmpty());
    mFormLayout = new QFormLayout(this);

    mTimeZone = NULL;
    mPositionLabel = NULL;
    mIsEmpty = (TrackData::sumTotalChildCount(items)==0);
    if (mIsEmpty && !items->isEmpty())
    {
        if (dynamic_cast<const TrackDataAbstractPoint *>(items->first())!=NULL) mIsEmpty = false;
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
        mFormLayout->addItem(new QSpacerItem(1, DialogBase::verticalSpacing(), QSizePolicy::Minimum, QSizePolicy::Fixed));
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
    qDebug() << name;
    delete mTimeZone;
    if (name.isEmpty())
    {
        mTimeZone = NULL;
        qDebug() << "set to no zone";
    }
    else
    {
        mTimeZone = new QTimeZone(name.toLatin1());
        if (!mTimeZone->isValid())
        {
            qWarning() << "unknown time zone" << name;
            delete mTimeZone;
            mTimeZone = NULL;
        }
        else qDebug() << "set to" << mTimeZone->id() << "offset" << mTimeZone->offsetFromUtc(QDateTime::currentDateTime());
    }

    emit updateTimeZones(timeZone());
}


void TrackPropertiesPage::slotPointPositionChanged(double newLat, double newLon)
{
    if (mPositionLabel==NULL) return;
    qDebug() << newLat << newLon;

    mPositionLabel->setText(TrackData::formattedLatLong(newLat, newLon));

    KColorScheme sch(QPalette::Normal);
    QPalette pal(mPositionLabel->palette());
    pal.setColor(QPalette::WindowText, sch.foreground(KColorScheme::NeutralText).color());
    mPositionLabel->setPalette(pal);
}


void TrackPropertiesPage::disableIfEmpty(QWidget *field, bool always)
{
    if (!isEmpty() && !always) return;

    QWidget *l = mFormLayout->labelForField(field);
    if (l!=NULL) l->setEnabled(false);
    field->setEnabled(false);
}
