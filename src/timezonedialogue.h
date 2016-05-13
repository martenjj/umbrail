
#ifndef TIMEZONEDIALOGUE_H
#define TIMEZONEDIALOGUE_H


#include <dialogbase.h>
#include <dialogstatesaver.h>


class K4TimeZoneWidget;


class TimeZoneStateSaver : public DialogStateSaver
{
    Q_OBJECT

public:
    TimeZoneStateSaver(QDialog *pnt) : DialogStateSaver(pnt)	{}
    virtual ~TimeZoneStateSaver() = default;

protected:
    void saveConfig(QDialog *dialog, KConfigGroup &grp) const;
    void restoreConfig(QDialog *dialog, const KConfigGroup &grp);
};


class TimeZoneDialogue : public DialogBase
{
    Q_OBJECT

public:
    explicit TimeZoneDialogue(QWidget *pnt = nullptr);
    virtual ~TimeZoneDialogue() = default;

    void setTimeZone(const QString &zone);
    QString timeZone() const;

    K4TimeZoneWidget *timeZoneWidget() const		{ return (mTimeZoneWidget); }

protected slots:
    void slotUseUTC();
    void slotUseSystem();

private slots:
    void slotTimeZoneChanged();

private:
    K4TimeZoneWidget *mTimeZoneWidget;
    bool mReturnUTC;
};

#endif							// TIMEZONEDIALOGUE_H
