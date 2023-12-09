

#include "filesdatamodel.h"

#include <qfontdatabase.h>
#include <qdebug.h>
#include <qimage.h>
#include <qicon.h>

#include <klocalizedstring.h>
#include <kcolorscheme.h>

#include "trackdata.h"
#include "filesmodel.h"


FilesDataModel::FilesDataModel(QObject *pnt)
    : QIdentityProxyModel(pnt),
      ItemIndexInterface(this)
{
    qDebug();
}


int FilesDataModel::columnCount(const QModelIndex &pnt) const
{
    return (1);
}


QVariant FilesDataModel::data(const QModelIndex &idx, int role) const
{
    const TrackDataItem *item = itemForIndex(idx);
    const int col = idx.column();

    if (role==Qt::ToolTipRole && col==ColumnName)
    {
        QString tip;

        if (dynamic_cast<const TrackDataFolder *>(item)!=nullptr) tip = i18np("Folder with %1 item", "Folder with %1 items", item->childCount());
        else if (dynamic_cast<const TrackDataTrack *>(item)!=nullptr) tip = i18np("Track with %1 segment", "Track with %1 segments", item->childCount());
        else if (dynamic_cast<const TrackDataSegment *>(item)!=nullptr) tip = i18np("Segment with %1 point", "Segment with %1 points", item->childCount());
        else if (dynamic_cast<const TrackDataRoute *>(item)!=nullptr) tip = i18np("Route with %1 point", "Route with %1 points", item->childCount());
        else
        {
            const TrackDataFile *tdf = dynamic_cast<const TrackDataFile *>(item);
            if (tdf!=nullptr) tip = i18np("File %2 with %1 item", "File %2 with %1 items", tdf->childCount(), tdf->fileName().toDisplayString());
            else
            {
                const TrackDataAbstractPoint *tdp = dynamic_cast<const TrackDataAbstractPoint *>(item);
                if (dynamic_cast<const TrackDataTrackpoint *>(tdp)!=nullptr) tip = i18n("Point at %1, elevation %2", tdp->formattedTime(true), tdp->formattedElevation());
                else if (dynamic_cast<const TrackDataWaypoint *>(tdp)!=nullptr)
                {
                    tip = i18n("Waypoint at %1, elevation %2", tdp->formattedTime(true), tdp->formattedElevation());
                    const TrackData::WaypointStatus s = static_cast<TrackData::WaypointStatus>(item->metadata("status").toInt());
                    const QString wptStatus = TrackData::formattedWaypointStatus(s, true);
                    if (!wptStatus.isEmpty()) tip = i18n("%1 (%2)", tip, wptStatus);
                }
                else if (dynamic_cast<const TrackDataRoutepoint *>(tdp)!=nullptr) tip = i18n("Routepoint, elevation %1", tdp->formattedElevation());
            }
        }

        if (!tip.isEmpty())
        {
            QString desc = item->metadata("desc").toString();
            if (!desc.isEmpty())
            {
                desc.replace('\n', ";&nbsp;");
                tip = i18n("<div style=\"white-space:nowrap\">%1</div><div style=\"font-style:italic\">\"%2\"</div>", tip, desc);
            }

            return (tip);
        }
    }

    if (role==Qt::ForegroundRole && col==ColumnName)
    {
        QVariant status = item->metadata("status");
        if (!status.isNull())
        {
            const TrackData::WaypointStatus s = static_cast<TrackData::WaypointStatus>(status.toInt());
            KColorScheme sch(QPalette::Normal);
            if (s==TrackData::StatusTodo) return (sch.foreground(KColorScheme::NegativeText));
            if (s==TrackData::StatusDone) return (sch.foreground(KColorScheme::PositiveText));
            if (s==TrackData::StatusQuestion) return (sch.foreground(KColorScheme::NeutralText));
            if (s==TrackData::StatusUnwanted) return (sch.foreground(KColorScheme::NeutralText));
        }
    }

    return (QIdentityProxyModel::data(idx, role));
}
