// -*-mode:c++ -*-

#ifndef WAYPOINTIMAGEPROVIDER_H
#define WAYPOINTIMAGEPROVIDER_H


class QColor;
class QIcon;

class WaypointImageProviderPrivate;


class WaypointImageProvider
{
public:
    ~WaypointImageProvider();

    QIcon icon(const QColor &col);

    static WaypointImageProvider *self();

protected:
    explicit WaypointImageProvider();

private:
    WaypointImageProviderPrivate *d;
};

#endif							// WAYPOINTIMAGEPROVIDER_H
