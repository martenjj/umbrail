//////////////////////////////////////////////////////////////////////////
//									//
//  Project:	Umbrail - GPX track viewer and editor			//
//									//
//////////////////////////////////////////////////////////////////////////
//									//
//  Copyright (c) 2014-2021 Jonathan Marten <jjm@keelhaul.me.uk>	//
//  Home and download page: <http://github.com/martenjj/umbrail>	//
//									//
//  This program is free software; you can redistribute it and/or	//
//  modify it under the terms of the GNU General Public License as	//
//  published by the Free Software Foundation, either version 3 of	//
//  the License or (at your option) any later version.			//
//									//
//  It is distributed in the hope that it will be useful, but		//
//  WITHOUT ANY WARRANTY;  without even the implied warranty of		//
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the	//
//  GNU General Public License for more details.			//
//									//
//  You should have received a copy of the GNU General Public License	//
//  along with this program;  see the file COPYING for further		//
//  details.  If not, see <http://gnu.org/licenses/gpl>.      		//
//									//
//////////////////////////////////////////////////////////////////////////

#ifndef ELEVATIONTILE_H
#define ELEVATIONTILE_H

#include <qflags.h>
#include <qpair.h>
#include <qvector.h>

class QFile;
class QUrl;


class ElevationTile
{

public:
    enum StateFlag
    {
        Empty = 0,
        Loaded = 1,
        Pending = 2,
        Error = 4
    };
    Q_DECLARE_FLAGS(State, StateFlag)

    typedef QPair<int,int> TileId;			// floor of <latitude,longitude>

    explicit ElevationTile(const ElevationTile::TileId id);
    ~ElevationTile();

    ElevationTile::State state() const			{ return (mState); }
    ElevationTile::TileId id() const			{ return (mTileId); }
    bool isValid() const				{ return (mData!=nullptr); }
    bool isValidFor(double lat, double lon) const;

    QString cacheFile() const;
    QUrl sourceUrl() const;
    bool load(const QString &file);

    void setState(ElevationTile::State newState);

    int elevation(double lat, double lon) const;

    static ElevationTile::TileId makeTileId(double lat, double lon);

protected:
    typedef QVector<short> TileData;			// data array

protected:
    void setData(int ncols, int nrows, ElevationTile::TileData *data);
    bool loadInternal(QFile &f);

private:
    int mLatitudeBase;
    int mLongitudeBase;

    ElevationTile::TileId mTileId;
    ElevationTile::State mState;
    ElevationTile::TileData *mData;
    int mNCols;
    int mNRows;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(ElevationTile::State)

#endif							// ELEVATIONTILE_H
