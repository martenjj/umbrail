// -*-mode:c++ -*-

#ifndef LATLONGDIALOGUE_H
#define LATLONGDIALOGUE_H


#include <kdialog.h>


class LatLongWidget;


class LatLongDialogue : public KDialog
{
    Q_OBJECT

public:
    explicit LatLongDialogue(QWidget *pnt = NULL);
    virtual ~LatLongDialogue();

    void setLatLong(double lat, double lon);
    double latitude() const;
    double longitude() const;

private slots:
    void slotUpdateButtonState();

private:
    LatLongWidget *mWidget;
};


#endif							// LATLONGDIALOGUE_H
