
#include "timezonedialogue.h"

#include <qgridlayout.h>
#include <qlabel.h>
#include <qheaderview.h>

#include <kdebug.h>
#include <klocale.h>
#include <kglobal.h>
#include <kconfiggroup.h>
#include <ktimezonewidget.h>
#include <ktreewidgetsearchline.h>
#include <ksystemtimezone.h>




TimeZoneDialogue::TimeZoneDialogue(QWidget *pnt)
    : KDialog(pnt)
{
    setObjectName("TimeZoneDialogue");

    setModal(true);
    setButtons(KDialog::Ok|KDialog::Cancel|KDialog::Reset|KDialog::Default);
    setButtonText(KDialog::Ok, i18n("Select"));
    setButtonText(KDialog::Reset, i18nc("@action:button", "Reset to UTC"));
    setButtonText(KDialog::Default, i18nc("@action:button", "System Time Zone"));
    setCaption(i18n("Select Time Zone"));
    showButtonSeparator(true);

    connect(this, SIGNAL(resetClicked()), SLOT(slotUseUTC()));
    connect(this, SIGNAL(defaultClicked()), SLOT(slotUseSystem()));

    QWidget *w = new QWidget(this);
    setMainWidget(w);
    QGridLayout *gl = new QGridLayout(w);

    mTimeZoneWidget = new KTimeZoneWidget(this);
    mTimeZoneWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    gl->addWidget(mTimeZoneWidget, 1, 0, 1, -1);

    KTreeWidgetSearchLine *sl = new KTreeWidgetSearchLine(this, mTimeZoneWidget);
    sl->setCaseSensitivity(Qt::CaseInsensitive);
    gl->addWidget(sl, 0, 1);

    QLabel *l = new QLabel(i18nc("@label:textbox", "Search:"), this);
    l->setBuddy(sl);
    gl->addWidget(l, 0, 0);

    setMinimumSize(400,320);
    KConfigGroup grp = KGlobal::config()->group(objectName());
    restoreDialogSize(grp);
    QString colStates = grp.readEntry("State");
    if (!colStates.isEmpty()) mTimeZoneWidget->header()->restoreState(QByteArray::fromHex(colStates.toAscii()));

    mReturnUTC = false;
}



TimeZoneDialogue::~TimeZoneDialogue()
{
    KConfigGroup grp = KGlobal::config()->group(objectName());
    saveDialogSize(grp);
    grp.writeEntry("State", mTimeZoneWidget->header()->saveState().toHex());
}




void TimeZoneDialogue::setTimeZone(const QString &zone)
{
    kDebug() << zone;
    mTimeZoneWidget->setSelected(zone, true);
}



QString TimeZoneDialogue::timeZone() const
{
    if (mReturnUTC) return (QString::null);
    QStringList sel = mTimeZoneWidget->selection();
    return (sel.isEmpty() ? QString::null : sel.first());
}



void TimeZoneDialogue::slotUseUTC()
{
    mReturnUTC = true;
    accept();
}



void TimeZoneDialogue::slotUseSystem()
{
    mTimeZoneWidget->setSelected(KSystemTimeZones::local().name(), true);
    accept();
}
