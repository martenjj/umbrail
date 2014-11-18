
#ifndef TIMEZONEDIALOGUE_H
#define TIMEZONEDIALOGUE_H


#include <kdialog.h>


class KTimeZoneWidget;



class TimeZoneDialogue : public KDialog
{
    Q_OBJECT

public:
    explicit TimeZoneDialogue(QWidget *pnt = NULL);
    virtual ~TimeZoneDialogue();

    void setTimeZone(const QString &zone);
    QString timeZone() const;

protected slots:
    void slotUseUTC();
    void slotUseSystem();

private slots:
    void slotTimeZoneChanged();

private:
    KTimeZoneWidget *mTimeZoneWidget;
    bool mReturnUTC;
};

#endif							// TIMEZONEDIALOGUE_H
