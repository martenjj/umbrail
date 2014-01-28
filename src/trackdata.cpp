

#include "trackdata.h"

#include <math.h>

//#include <qregexp.h>
//#include <qstringlist.h>

#include <kglobal.h>
#include <klocale.h>
#include <kmimetype.h>



TrackDataItem::TrackDataItem(const QString &desc)
{
    mName = desc;
    init();
}



TrackDataItem::TrackDataItem(const QString &desc, const char *format, int *counter)
{
    if (!desc.isEmpty()) mName = desc;
    else mName.sprintf(format, ++(*counter));
    init();
}



void TrackDataItem::init()
{
    mParent = NULL;
}



TrackDataItem::~TrackDataItem()
{
    qDeleteAll(mChildItems);
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




TrackDataRoot::TrackDataRoot(const QString &desc)
    : TrackDataItem(desc)
{
}




int TrackDataFile::sCounter = 0;

TrackDataFile::TrackDataFile(const QString &desc)
    : TrackDataItem(desc, "file_%02d", &TrackDataFile::sCounter)
{
}




QString TrackDataFile::iconName() const
{
    //return ("media-floppy");
    return (KMimeType::iconNameForUrl(mFileName));
}



int TrackDataTrack::sCounter = 0;

TrackDataTrack::TrackDataTrack(const QString &desc)
    : TrackDataItem(desc, "track_%02d", &TrackDataTrack::sCounter)
{
}



int TrackDataSegment::sCounter = 0;

TrackDataSegment::TrackDataSegment(const QString &desc)
    : TrackDataItem(desc, "segment_%02d", &TrackDataSegment::sCounter)
{
}



int TrackDataPoint::sCounter = 0;

TrackDataPoint::TrackDataPoint(const QString &desc)
    : TrackDataItem(desc, "point_%04d", &TrackDataPoint::sCounter)
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
