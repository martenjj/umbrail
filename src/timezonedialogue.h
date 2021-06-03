
#ifndef TIMEZONEDIALOGUE_H
#define TIMEZONEDIALOGUE_H


#include <kfdialog/dialogbase.h>
#include <kfdialog/dialogstatesaver.h>


class TimeZoneWidget;


class TimeZoneDialogue : public DialogBase, public DialogStateSaver
{
    Q_OBJECT

public:
    explicit TimeZoneDialogue(QWidget *pnt = nullptr);
    virtual ~TimeZoneDialogue() = default;

    void setTimeZone(const QByteArray &zone);
    QString timeZone() const;

    void saveConfig(QDialog *dialog, KConfigGroup &grp) const override;
    void restoreConfig(QDialog *dialog, const KConfigGroup &grp) override;

protected slots:
    void slotUseUTC();
    void slotUseSystem();

private slots:
    void slotTimeZoneChanged();

private:
    TimeZoneWidget *mTimeZoneWidget;
    bool mReturnUTC;
};

#endif							// TIMEZONEDIALOGUE_H
