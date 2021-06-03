
#include "trackpropertiespage.h"

#include <qformlayout.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qdebug.h>

#include <klocalizedstring.h>

#include <kfdialog/dialogbase.h>

#include "trackdata.h"


TrackPropertiesPage::TrackPropertiesPage(const QList<TrackDataItem *> *items, QWidget *pnt)
    : QWidget(pnt)
{
    Q_ASSERT(items!=nullptr);
    Q_ASSERT(!items->isEmpty());

    mFormLayout = new QFormLayout(this);

    mIsEmpty = (TrackData::sumTotalChildCount(items)==0);
    if (mIsEmpty && !items->isEmpty())
    {
        if (dynamic_cast<const TrackDataAbstractPoint *>(items->first())!=nullptr) mIsEmpty = false;
    }
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


void TrackPropertiesPage::disableIfEmpty(QWidget *field, bool always)
{
    if (!isEmpty() && !always) return;

    QWidget *l = mFormLayout->labelForField(field);
    if (l!=nullptr) l->setEnabled(false);
    field->setEnabled(false);
}
