

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
        QString tip = item->toolTip();
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
