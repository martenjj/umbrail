

#include "trackdata.h"

#include <math.h>

//#include <qregexp.h>
//#include <qstringlist.h>

#include <kglobal.h>
#include <klocale.h>
#include <kmimetype.h>



TrackDataItem::TrackDataItem(const QString &name)
{
    mName = name;
//    mType = cat;
    mParent = NULL;
}


TrackDataItem::~TrackDataItem()
{
    ///////// TODO: recursively delete children
}



void TrackDataItem::addChildItem(TrackDataItem *data)
{
    data->mParent = this;				// set item parent
    mChildItems.append(data);
}



TrackDataItem *TrackDataItem::removeLastChildItem()
{
    Q_ASSERT(!mChildItems.isEmpty());
    TrackDataItem *data = mChildItems.takeLast();
    data->mParent = NULL;				// now no longer has parent
    return (data);
}




TrackDataRoot::TrackDataRoot(const QString &name)
    : TrackDataItem(name)
{
}






TrackDataFile::TrackDataFile(const QString &name)
    : TrackDataItem(name)
{
}




QString TrackDataFile::iconName() const
{
    //return ("media-floppy");
    return (KMimeType::iconNameForUrl(mFileName));
}





TrackDataTrack::TrackDataTrack(const QString &name)
    : TrackDataItem(name)
{
}


TrackDataSegment::TrackDataSegment(const QString &name)
    : TrackDataItem(name)
{
}



TrackDataPoint::TrackDataPoint(const QString &name)
    : TrackDataItem(name)
{
    mLatitude = mLongitude = NAN;
    mElevation = NAN;
}


QString TrackDataPoint::formattedElevation() const
{
    if (isnan(mElevation)) return (i18nc("an unknown quantity", "unknown"));
    return (QString("%1").arg(mElevation, 0, 'f', 1));
}


QString TrackDataPoint::formattedTime() const
{
    if (!mDateTime.isValid()) return (i18nc("an unknown quantity", "unknown"));
    return (KGlobal::locale()->formatDateTime(mDateTime, KLocale::ShortDate, true));
}
