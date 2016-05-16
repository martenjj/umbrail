
#include "timezonedialogue.h"

#include <qgridlayout.h>
#include <qlabel.h>
#include <qheaderview.h>
#include <qpushbutton.h>
#include <qdebug.h>
#include <qtimezone.h>

#include <klocalizedstring.h>
#include <kconfiggroup.h>
#include <ktreewidgetsearchline.h>

#include "timezonewidget.h"


void TimeZoneStateSaver::saveConfig(QDialog *dialog, KConfigGroup &grp) const
{
    const TimeZoneDialogue *wid = qobject_cast<const TimeZoneDialogue *>(dialog);
    if (wid!=nullptr) grp.writeEntry("State", wid->timeZoneWidget()->header()->saveState().toHex());
    DialogStateSaver::saveConfig(dialog, grp);
}


void TimeZoneStateSaver::restoreConfig(QDialog *dialog, const KConfigGroup &grp)
{
    TimeZoneDialogue *wid = qobject_cast<TimeZoneDialogue *>(dialog);
    if (wid!=nullptr)
    {
        QString colStates = grp.readEntry("State");
        if (!colStates.isEmpty()) wid->timeZoneWidget()->header()->restoreState(QByteArray::fromHex(colStates.toAscii()));
    }
    DialogStateSaver::restoreConfig(dialog, grp);
}


TimeZoneDialogue::TimeZoneDialogue(QWidget *pnt)
    : DialogBase(pnt)
{
    setObjectName("TimeZoneDialogue");

    setModal(true);
    setButtons(QDialogButtonBox::Ok|QDialogButtonBox::Cancel|QDialogButtonBox::Reset|QDialogButtonBox::RestoreDefaults);
    buttonBox()->button(QDialogButtonBox::Ok)->setDefault(true);
    setButtonText(QDialogButtonBox::Ok, i18n("Select"));
    setButtonText(QDialogButtonBox::Reset, i18nc("@action:button", "Reset to UTC"));
    setButtonText(QDialogButtonBox::RestoreDefaults, i18nc("@action:button", "System Time Zone"));
    setButtonIcon(QDialogButtonBox::RestoreDefaults, buttonBox()->button(QDialogButtonBox::Reset)->icon());
    setWindowTitle(i18n("Select Time Zone"));

    connect(buttonBox()->button(QDialogButtonBox::Reset), &QPushButton::clicked, this, &TimeZoneDialogue::slotUseUTC);
    connect(buttonBox()->button(QDialogButtonBox::RestoreDefaults), &QPushButton::clicked, this, &TimeZoneDialogue::slotUseSystem);

    QWidget *w = new QWidget(this);
    setMainWidget(w);
    QGridLayout *gl = new QGridLayout(w);

    // Filter the available time zones so that the list does not need too
    // much scrolling.  Accept only those zones which start with "Europe/"
    // or "UTC", and whose time offset is within 3 hours from UTC.
    // TODO: make this configurable, offset from current system time zone
    QList<QByteArray> allZoneIds = QTimeZone::availableTimeZoneIds();
    QList<QByteArray> zoneIds;
    foreach (const QByteArray &zone, allZoneIds)
    {
        if (zone.startsWith("Europe/") || zone.startsWith("UTC"))
        {
            QTimeZone tz(zone);
            if (qAbs(tz.offsetFromUtc(QDateTime::currentDateTime()))<=(3*3600))
            {
                zoneIds.append(zone);
            }
        }
    }
    //qDebug() << zoneIds;

    mTimeZoneWidget = new TimeZoneWidget(this, zoneIds);
    mTimeZoneWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    connect(mTimeZoneWidget, SIGNAL(itemSelectionChanged()), SLOT(slotTimeZoneChanged()));
    gl->addWidget(mTimeZoneWidget, 1, 0, 1, -1);

    KTreeWidgetSearchLine *sl = new KTreeWidgetSearchLine(this, mTimeZoneWidget);
    sl->setCaseSensitivity(Qt::CaseInsensitive);
    gl->addWidget(sl, 0, 1);

    QLabel *l = new QLabel(i18nc("@label:textbox", "Search:"), this);
    l->setBuddy(sl);
    gl->addWidget(l, 0, 0);

    setMinimumSize(400,320);
    setStateSaver(new TimeZoneStateSaver(this));

    mReturnUTC = false;
    slotTimeZoneChanged();
    sl->setFocus(Qt::ActiveWindowFocusReason);
}


void TimeZoneDialogue::setTimeZone(const QByteArray &zone)
{
    qDebug() << zone;
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
    mTimeZoneWidget->setSelected(QTimeZone::systemTimeZone().id(), true);
    accept();
}


void TimeZoneDialogue::slotTimeZoneChanged()
{
    setButtonEnabled(QDialogButtonBox::Ok, !mTimeZoneWidget->selectedItems().isEmpty());
}
