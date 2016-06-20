
#include "trackpropertiesmetadatapages.h"

#include <qtableview.h>
#include <qheaderview.h>
#include <qformlayout.h>

#include <kdebug.h>
#include <klocale.h>

#include "trackdata.h"
#include "metadatamodel.h"
#include "autotooltipdelegate.h"


TrackItemMetadataPage::TrackItemMetadataPage(const QList<TrackDataItem *> *items, QWidget *pnt)
    : TrackPropertiesPage(items, pnt)
{
    kDebug();
    setObjectName("TrackItemMetadataPage");

    mView = new QTableView(this);
    mView->setCornerButtonEnabled(false);
    mView->setWordWrap(false);
    mView->setDragEnabled(false);
    mView->setSelectionMode(QAbstractItemView::NoSelection);
    mView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    mView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    mView->setItemDelegate(new AutoToolTipDelegate(this));
    mView->horizontalHeader()->setStretchLastSection(true);

    // Don't want to use the existing form layout, because that presumably has
    // some sort of row height limit and it doesn't stretch the table view all
    // the way to the bottom.  There are no labels and nothing else to display
    // on this page, so the form layout is not needed.
    delete mFormLayout; mFormLayout = NULL;
    QVBoxLayout *vboxLayout = new QVBoxLayout(this);
    vboxLayout->addWidget(mView);

    if (items->count()!=1) return;			// only populate for single item

    const TrackDataItem *item = items->first();
    mModel = new MetadataModel(item, this);
    mView->setModel(mModel);
    mView->resizeColumnsToContents();
    mView->resizeRowsToContents();
}



TrackFileMetadataPage::TrackFileMetadataPage(const QList<TrackDataItem *> *items, QWidget *pnt)
    : TrackItemMetadataPage(items, pnt)
{
    kDebug();
    setObjectName("TrackFileMetadataPage");
}



TrackTrackMetadataPage::TrackTrackMetadataPage(const QList<TrackDataItem *> *items, QWidget *pnt)
    : TrackItemMetadataPage(items, pnt)
{
    kDebug();
    setObjectName("TrackTrackMetadataPage");
}



TrackRouteMetadataPage::TrackRouteMetadataPage(const QList<TrackDataItem *> *items, QWidget *pnt)
    : TrackItemMetadataPage(items, pnt)
{
    kDebug();
    setObjectName("TrackRouteMetadataPage");
}



TrackSegmentMetadataPage::TrackSegmentMetadataPage(const QList<TrackDataItem *> *items, QWidget *pnt)
    : TrackItemMetadataPage(items, pnt)
{
    kDebug();
    setObjectName("TrackSegmentMetadataPage");
}



TrackTrackpointMetadataPage::TrackTrackpointMetadataPage(const QList<TrackDataItem *> *items, QWidget *pnt)
    : TrackItemMetadataPage(items, pnt)
{
    kDebug();
    setObjectName("TrackPointMetadataPage");
}



TrackFolderMetadataPage::TrackFolderMetadataPage(const QList<TrackDataItem *> *items, QWidget *pnt)
    : TrackItemMetadataPage(items, pnt)
{
    kDebug();
    setObjectName("TrackFolderMetadataPage");
}



TrackWaypointMetadataPage::TrackWaypointMetadataPage(const QList<TrackDataItem *> *items, QWidget *pnt)
    : TrackItemMetadataPage(items, pnt)
{
    kDebug();
    setObjectName("TrackWaypointMetadataPage");
}



TrackRoutepointMetadataPage::TrackRoutepointMetadataPage(const QList<TrackDataItem *> *items, QWidget *pnt)
    : TrackItemMetadataPage(items, pnt)
{
    kDebug();
    setObjectName("TrackRoutepointMetadataPage");
}



CREATE_PROPERTIES_PAGE(File, Metadata);
CREATE_PROPERTIES_PAGE(Track, Metadata);
CREATE_PROPERTIES_PAGE(Route, Metadata);
CREATE_PROPERTIES_PAGE(Segment, Metadata);
CREATE_PROPERTIES_PAGE(Trackpoint, Metadata);
CREATE_PROPERTIES_PAGE(Folder, Metadata);
CREATE_PROPERTIES_PAGE(Waypoint, Metadata);
CREATE_PROPERTIES_PAGE(Routepoint, Metadata);
