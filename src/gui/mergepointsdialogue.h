// -*-mode:c++ -*-

#ifndef MERGEPOINTSDIALOG_H
#define MERGEPOINTSDIALOG_H
 

#include <kfdialog/dialogbase.h>

#include "trackdata.h"

class QComboBox;
class QLabel;





class ListEdit : public QWidget
{
    Q_OBJECT

public:
    // TODO: for use with TrackPropertiesDetailPage
    //explicit ListEdit(bool withEditButtonText, QWidget *pnt = nullptr);
    explicit ListEdit(QWidget *pnt = nullptr);
    virtual ~ListEdit() = default;

    void setList(const QStringList &list);

signals:
    void editRequested();

private:
    // TODO: a read only QLineEdit for more consistent appearance
    QLabel *mListLabel;
};






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
    ListEdit *mCategoriesLabel;
    QComboBox *mAddressEdit;
};

#endif							// MERGEPOINTSDIALOG_H
