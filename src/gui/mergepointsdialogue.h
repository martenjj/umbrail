// -*-mode:c++ -*-

#ifndef MERGEPOINTSDIALOG_H
#define MERGEPOINTSDIALOG_H

#include <kfdialog/dialogbase.h>

#include "trackdata.h"

class QComboBox;
class ListEditWidget;


class MergePointsDialogue : public DialogBase
{
    Q_OBJECT

public:
    explicit MergePointsDialogue(QWidget *pnt = nullptr);
    virtual ~MergePointsDialogue() = default;

    void setPoints(const QList<const TrackDataWaypoint *> *points);
    TrackDataWaypoint *resultPoint();

protected slots:
    void slotEditCategories();

private slots:
    void slotUpdateButtons();

private:
    const QList<const TrackDataWaypoint *> *mPoints;
    QStringList mCombinedCats;

    QComboBox *mNameEdit;
    QComboBox *mSymbolEdit;
    QComboBox *mLatLongEdit;
    QComboBox *mElevationEdit;
    QComboBox *mTimeEdit;
    QComboBox *mStatusEdit;
    QComboBox *mDescriptionEdit;
    ListEditWidget *mCategoriesLabel;
    QComboBox *mAddressEdit;
};

#endif							// MERGEPOINTSDIALOG_H
