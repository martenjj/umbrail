
#ifndef ITEMTYPECOMBO_H
#define ITEMTYPECOMBO_H

#include <kcombobox.h>



class ItemTypeCombo : public KComboBox
{
    Q_OBJECT

public:
    explicit ItemTypeCombo(QWidget *pnt = nullptr);
    virtual ~ItemTypeCombo();

    void setType(const QString &type);
    QString typeText() const;

private:
    int mOriginalCount;
};

#endif							// ITEMTYPECOMBO_H
