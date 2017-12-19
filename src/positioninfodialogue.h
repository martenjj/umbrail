
#ifndef POSITIONINFODIALOGUE_H
#define POSITIONINFODIALOGUE_H


#include <dialogbase.h>
#include <marble/GeoDataPlacemark.h>

using Marble::GeoDataCoordinates;
using Marble::GeoDataPlacemark;


class QLabel;
class ElevationTile;
class VariableUnitDisplay;


class PositionInfoDialogue : public DialogBase
{
    Q_OBJECT

public:
    PositionInfoDialogue(int posX, int posY, QWidget *pnt = nullptr);
    virtual ~PositionInfoDialogue() = default;

private slots:
    void slotShowAddressInformation(const GeoDataCoordinates &coords, const GeoDataPlacemark &placemark);
    void slotShowElevation(const ElevationTile *tile);

private:
    QLabel *mAddressLabel;
    VariableUnitDisplay *mElevationDisplay;

    qreal mLatitude;
    qreal mLongitude;
};

#endif							// POSITIONINFODIALOGUE_H
