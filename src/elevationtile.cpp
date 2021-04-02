//////////////////////////////////////////////////////////////////////////
//									//
//  Project:	NavTracks						//
//  Edit:	02-Apr-21						//
//									//
//////////////////////////////////////////////////////////////////////////
//									//
//  Copyright (c) 2012-2014 Jonathan Marten <jjm@keelhaul.me.uk>	//
//  Home and download page:  http://www.keelhaul.me.uk/TBD/		//
//									//
//  This program is free software; you can redistribute it and/or	//
//  modify it under the terms of the GNU General Public License as	//
//  published by the Free Software Foundation; either version 2 of	//
//  the License, or (at your option) any later version.			//
//									//
//  It is distributed in the hope that it will be useful, but		//
//  WITHOUT ANY WARRANTY; without even the implied warranty of		//
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the	//
//  GNU General Public License for more details.			//
//									//
//  You should have received a copy of the GNU General Public		//
//  License along with this program; see the file COPYING for further	//
//  details.  If not, write to the Free Software Foundation, Inc.,	//
//  59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.		//
//									//
//////////////////////////////////////////////////////////////////////////

#include "elevationtile.h"

#include <math.h>

#include <qpair.h>
#include <qdebug.h>
#include <qurl.h>
#include <qurlquery.h>
#include <qfile.h>
#include <qguiapplication.h>
#include <qcursor.h>


// generic tile handling here

ElevationTile::ElevationTile(const ElevationTile::TileId tid)
{
    mLatitudeBase = tid.first;
    mLongitudeBase = tid.second;
    mTileId = tid;
    mState = ElevationTile::Empty;
    mData = nullptr;

    qDebug() << "from" << tid << "-> lat" << mLatitudeBase << "lon" << mLongitudeBase;
}


ElevationTile::~ElevationTile()
{
    delete mData;
}


/* static */ ElevationTile::TileId ElevationTile::makeTileId(double lat, double lon)
{
    int latBase = qRound(floor(lat));
    int lonBase = qRound(floor(lon));
    return (qMakePair(latBase, lonBase));
}


void ElevationTile::setState(ElevationTile::State newState)
{
    qDebug() << "tile" << id() << "state changed" << mState << "->" << newState;
    mState = newState;
}


void ElevationTile::setData(int ncols, int nrows, ElevationTile::TileData *data)
{
    if (mData!=nullptr)
    {
        qWarning() << "data already loaded";
        delete mData;
    }

    qDebug() << "size" << data->size();
    mData = data;
    mNCols = ncols;
    mNRows = nrows;
    mState = ElevationTile::Loaded;
}


bool ElevationTile::isValidFor(double lat, double lon) const
{
    if (!isValid()) return (false);

    const double latOff = lat-double(mLatitudeBase);	// offset within this tile
    const double lonOff = lon-double(mLongitudeBase);
    return (latOff<1.0 && lonOff<1.0);			// check that both are within bounds
}


bool ElevationTile::load(const QString &file)
{
    bool ok = false;

    qDebug() << "from" << file;
    QFile f(file);
    if (f.open(QIODevice::ReadOnly))			// open geo data file
    {
        QGuiApplication::setOverrideCursor(Qt::BusyCursor);
        ok = loadInternal(f);				// check completed state
        f.close();					// finished with data file
        QGuiApplication::restoreOverrideCursor();
    }
    else qWarning() << "error opening file!";
    if (!ok) setState(ElevationTile::Error);		// set error state

    return (ok);
}




// format-specific from here on


static const int LINE_MAXLENGTH = 10240;		// length of line read buffer
static const char DEM_FORMAT[] = "AAIGrid";		// file format for download


int ElevationTile::elevation(double lat, double lon) const
{
    if (mState!=ElevationTile::Loaded) return (0);
    Q_ASSERT(mData!=nullptr);

    const double latOff = lat-double(mLatitudeBase);	// offset within this tile
    const double lonOff = lon-double(mLongitudeBase);

    if (latOff>=1.0 || lonOff>=1.0)			// check that both are within bounds
    {
        qWarning() << "lat/lon out of range, asked for" << lat << lon
                   << "base" << mLatitudeBase << mLongitudeBase;
        return (0);
    }

    // get tile elevation at that offset
    const int row = static_cast<int>((1.0-latOff)*mNRows);	// base is at bottom left
    const int col = static_cast<int>(lonOff*mNCols);
    return mData->at(row*mNCols+col);
}


QString ElevationTile::cacheFile() const
{
    return (QString("%1%2%3%4.%5")
            .arg(mLatitudeBase==0 ? "Z" : (mLatitudeBase>0 ? "N" : "S"))	// latitude sign
            .arg(abs(mLatitudeBase), 2, 10, QLatin1Char('0'))			// latitude
            .arg(mLongitudeBase==0 ? "Z" : (mLongitudeBase>0 ? "E" : "W"))	// longitide sign
            .arg(abs(mLongitudeBase), 3, 10, QLatin1Char('0'))			// longitude
            .arg(DEM_FORMAT));							// format
}


QUrl ElevationTile::sourceUrl() const
{
    // see https://portal.opentopography.org/apidocs/#/Public/getGlobalDem

    QUrlQuery query("demtype=SRTMGL3");			// SRTM GL3 (90m)

    query.addQueryItem("south", QString::number(mLatitudeBase));
    query.addQueryItem("north", QString::number(mLatitudeBase+1));
    query.addQueryItem("west", QString::number(mLongitudeBase));
    query.addQueryItem("east", QString::number(mLongitudeBase+1));

    query.addQueryItem("outputFormat", DEM_FORMAT);

    QUrl u("http://portal.opentopography.org/API/globaldem", QUrl::StrictMode);
    u.setQuery(query);
    return (u);
}


bool ElevationTile::loadInternal(QFile &f)
{
    // AAgrid file header format:
    //
    //   ncols        1200
    //   nrows        1200
    //   xllcorner    -1.000416666707
    //   yllcorner    50.000416666669
    //   cellsize     0.000833333333
    //   NODATA_value -32768

    int lineno = 0;					// line number from file
    int row = -1;					// current row number
    int nrows = -1;					// row count from header
    int ncols = -1;					// column count from header
    ElevationTile::TileData *v = nullptr;		// allocated file data

    while (true)
    {
        QByteArray line = f.readLine(LINE_MAXLENGTH);
        ++lineno;

        const int len = line.size();
        if (len==0)					// check not empty or EOF
        {
            if (row>=nrows) break;			// all expected rows read
							// unexpected EOF
            qWarning() << "EOF while parsing, at line" << lineno;
            break;
        }

        if (line[len]!='\0' || line[len-1]!='\n')	// check properly terminated
        {
            qWarning() << "Short line while parsing, at line" << lineno;
            continue;
        }

        line.resize(len-1);				// remove terminators

        QStringList fields = QString::fromLatin1(line).split(QRegExp("\\s+"), Qt::SkipEmptyParts);
							// split into fields
        if (fields.size()==2)				// fields for a header
        {
            const QString &name = fields[0];
            const QString &value = fields[1];
            qDebug() << "  header field" << name << "value" << value;
            if (name=="ncols") ncols = value.toInt();
            else if (name=="nrows") nrows = value.toInt();
            continue;
        }

        if (v==nullptr)
        {
            // Assume that now we are after the header, at the first data line.
            if (ncols<=0 || nrows<=0)
            {
                qWarning() << "Bad value for columns/rows," << ncols << nrows << "at line" << lineno;
                break;
            }

            qDebug() << "data size cols" << ncols << "rows" << nrows;
            v = new QVector<short>(ncols*nrows);	// allocate data storage
            row = 0;					// set first row number
        }

        Q_ASSERT(v!=nullptr);				// must have array by now
        if (row>=nrows)					// check number of rows
        {
            qWarning() << "Too many rows at line" << lineno;
            break;
        }

        int cols = fields.size();
        if (cols!=ncols)
        {
            qWarning() << "Wrong column count, got" << cols << "expected" << ncols << "at line" << lineno;
            cols = qMin(cols, ncols);			// don't store too many
        }

        short *p = &(v->data()[row*ncols]);		// store row data
        for (int col = 0; col<cols; ++col) *p++ = fields[col].toShort();
        ++row;						// count up this row
    }

    const bool ok = (v!=nullptr);			// check completed state
    qDebug() << "read" << lineno << "lines, status" << ok;

    if (ok) setData(ncols, nrows, v);			// save tile data
    return (ok);
}
