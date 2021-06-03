// -*-mode:c++ -*-

#ifndef LATLONGDIALOGUE_H
#define LATLONGDIALOGUE_H


#include <kfdialog/dialogbase.h>


class LatLongWidget;


class LatLongDialogue : public DialogBase
{
    Q_OBJECT

public:
    explicit LatLongDialogue(QWidget *pnt = nullptr);
    virtual ~LatLongDialogue() = default;

    void setLatLong(double lat, double lon);
    double latitude() const;
    double longitude() const;

private slots:
    void slotUpdateButtonState();

private:
    LatLongWidget *mWidget;
};


#endif							// LATLONGDIALOGUE_H
