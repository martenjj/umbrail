
#include "itemtypecombo.h"

#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <kconfiggroup.h>




ItemTypeCombo::ItemTypeCombo(QWidget *pnt)
    : KComboBox(pnt)
{
    setObjectName("ItemTypeCombo");

    setEditable(true);
    setDuplicatesEnabled(false);
    setInsertPolicy(QComboBox::InsertAtBottom);
    setSizePolicy(QSizePolicy::Expanding, sizePolicy().verticalPolicy());

    QStringList defaultTypes(i18n("(none)"));
    defaultTypes << "Walk" << "Car";
    const KConfigGroup grp = KGlobal::config()->group(objectName());
    QStringList types = grp.readEntry("types", defaultTypes);
    addItems(types);

    mOriginalCount = count();				// how many when we started
}



ItemTypeCombo::~ItemTypeCombo()
{
    kDebug() << "count" << count() << "original" << mOriginalCount;
    if (count()>mOriginalCount)				// was anything added?
    {
        QStringList types;
        for (int i = 0; i<count(); ++i) types << itemText(i);
        kDebug() << "saving" << types;
        KConfigGroup grp = KGlobal::config()->group(objectName());
        grp.writeEntry("types", types);
    }
}





void ItemTypeCombo::setType(const QString &type)
{
    if (type.isEmpty()) setCurrentIndex(0);		// "none" is always first
    else
    {
        int idx = findText(type, Qt::MatchFixedString);
        kDebug() << type << "found at" << idx;
        if (idx==-1)					// not already in combo
        {
            addItem(type);				// add as new item
            idx = count()-1;				// item was added at end
        }

        setCurrentIndex(idx);				// set item as current
    }
}
